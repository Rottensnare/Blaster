// Fill out your copyright notice in the Description page of Project Settings.


#include "LagCompensationComponent.h"

#include "Blaster/Blaster.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/Weapon/Weapon.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values for this component's properties
ULagCompensationComponent::ULagCompensationComponent()
{

	PrimaryComponentTick.bCanEverTick = true;
	
}

void ULagCompensationComponent::BeginPlay()
{
	Super::BeginPlay();


	FFramePackage Package;
	SaveFramePackage(Package);
	//ShowFramePackage(Package, FColor::Red);
}

//Increase or decrease TickRate. Goes without saying, but decreasing tickrate will decrease the accuracy of server side rewind, but will cause a performance improvement for the server
void ULagCompensationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	SaveFramePackage();
}

void ULagCompensationComponent::SaveFramePackage()
{
	if(BlasterCharacter == nullptr || !BlasterCharacter->HasAuthority()) return; //We only want the server to keep track of the past. That's the whole point of 'SERVER-side rewind'. 
	//If player has just spawned, we won't try to remove Frame Packages from the FrameHistory. Just add new packages to history
	if(FrameHistory.Num() <= 1)
	{
		FFramePackage ThisFrame;
		SaveFramePackage(ThisFrame);
		FrameHistory.AddHead(ThisFrame);
	}
	else //Checking if we should remove Packages from FrameHistory. This handles the limiting of FrameHistory based on time. We don't want to keep track of frame history for too long.
	{
		float HistoryLenght = FrameHistory.GetHead()->GetValue().Time - FrameHistory.GetTail()->GetValue().Time; //This is the length of time we are currently keeping our FrameHistory

		//Keep removing old packages from the list until FrameHistory is within the allowed HistoryLenght we set 
		//Using a while loop because time inconsistencies. Might need to remove 2 frames or more.
		while(HistoryLenght > MaxRecordTime) 
		{
			FrameHistory.RemoveNode(FrameHistory.GetTail()); //Removing the oldest FramePackage
			HistoryLenght = FrameHistory.GetHead()->GetValue().Time - FrameHistory.GetTail()->GetValue().Time; //Updating the our Frame history length, THIS IS A MUST, OTHERWISE WE HAVE AN INFINITE LOOP
		}
		//After we know that the history length is shorter than the allowed time, we need to create a new package for the current frame and add it as the head.
		FFramePackage ThisFrame;
		SaveFramePackage(ThisFrame);
		FrameHistory.AddHead(ThisFrame);
		//ShowFramePackage(ThisFrame, FColor::Emerald);
	}
}

void ULagCompensationComponent::SaveFramePackage(FFramePackage& OutPackage)
{
	BlasterCharacter = BlasterCharacter == nullptr ? Cast<ABlasterCharacter>(GetOwner()) : BlasterCharacter;
	if(BlasterCharacter)
	{
		//Current server time
		OutPackage.Time = GetWorld()->GetTimeSeconds();
		OutPackage.BlasterCharacter = BlasterCharacter;
		//Iterate over every HitBox the character has
		for (auto& BoxPair : BlasterCharacter->HitCollisionBoxes)
		{
			FBoxInformation BoxInformation;
			BoxInformation.BoxLocation = BoxPair.Value->GetComponentLocation();
			BoxInformation.BoxRotation = BoxPair.Value->GetComponentRotation();
			BoxInformation.BoxExtent = BoxPair.Value->GetScaledBoxExtent();

			OutPackage.HitBoxInfo.Add(BoxPair.Key, BoxInformation);
		}
	}
}

FFramePackage ULagCompensationComponent::InterpBetweenFrames(const FFramePackage& OlderFrame, //TODO: Need to make sure I fully understand what's happening here
	const FFramePackage& YoungerFrame, const float HitTime)
{
	//Get the time between the 2 FramePackages
	const float Distance = YoungerFrame.Time - OlderFrame.Time;
	//If the time between the FramePackages is 1, InterpFraction will be somewhere between 0 and 1, where that hit occured within that time frame. A Fraction so to speak.
	const float InterpFraction = FMath::Clamp((HitTime - OlderFrame.Time) / Distance, 0.f, 1.f);
	FFramePackage InterpFramePackage;
	InterpFramePackage.Time = HitTime;
	//Iterating over all the hitboxes and their 'transforms'
	for(auto& YoungerPair : YoungerFrame.HitBoxInfo)
	{
		const FName& BoxInfoName = YoungerPair.Key;
		const FBoxInformation& OlderBox = OlderFrame.HitBoxInfo[BoxInfoName];
		const FBoxInformation& YoungerBox = YoungerFrame.HitBoxInfo[BoxInfoName];
		FBoxInformation InterpBoxInfo;
		//Interpolating between the locations and rotations of the 2 FramePackages and using the InterpFraction so that we are interpolating exactly to that place in time where the client claimed to have hit the player.
		//Without setting DeltaTime to 1.f we couldn't use InterpFraction in this way. It essentially only interpolates once based on the InterpFraction.
		InterpBoxInfo.BoxLocation = FMath::VInterpTo(OlderBox.BoxLocation, YoungerBox.BoxLocation, 1.f, InterpFraction);
		InterpBoxInfo.BoxRotation = FMath::RInterpTo(OlderBox.BoxRotation, YoungerBox.BoxRotation, 1.f, InterpFraction);
		InterpBoxInfo.BoxExtent = OlderBox.BoxExtent;

		InterpFramePackage.HitBoxInfo.Add(BoxInfoName, InterpBoxInfo);
	}
	//Returning the interpolated FramePackage we created and filled.
	return InterpFramePackage;
}

FServerSideRewindResult ULagCompensationComponent::ConfirmHit(const FFramePackage& Package,
	ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation)
{
	if(HitCharacter == nullptr) return FServerSideRewindResult();
	
	//FramePackage we use to store the current hitbox location. Needed because we are moving the hitboxes to the place in time based on the interpolated FramePackage.
	//After that we need to set the boxes back to their correct positions.
	FFramePackage CurrentFrame;

	//Saving the HitBox information for later use.
	CacheBoxTransform(HitCharacter,CurrentFrame);
	MoveHitBoxes(HitCharacter, Package);
	//Disabling character mesh collision so that it won't interfere with the line trace we are about to do.
	EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::NoCollision);

	//First check hits for headshots
	UBoxComponent* HeadBox = HitCharacter->HitCollisionBoxes[FName("head")];
	HeadBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	HeadBox->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);

	FHitResult HitResult;
	const FVector TraceEnd = TraceStart + (HitLocation - TraceStart) * 1.2f;
	UWorld* World = GetWorld();
	if(World)
	{
		World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_HitBox);
		if(HitResult.bBlockingHit) //TODO: Probably need to have more checks so that we can accurately tell which character actually got hit
		{
			if(HitResult.Component.IsValid())
			{
				UBoxComponent* Box = Cast<UBoxComponent>(HitResult.Component);
				if(Box)
				{
					DrawDebugBox(GetWorld(), Box->GetComponentLocation(), Box->GetScaledBoxExtent(), FQuat(Box->GetComponentRotation()), FColor::Red, false, 7.f);
				}
			}
			ResetHitBoxes(HitCharacter, CurrentFrame);
			EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
			return FServerSideRewindResult{true, true};
		}
		else
		{
			//If no headshot, then iterate over all the hitboxes and enable their collision
			for(auto& HitBoxPair : HitCharacter->HitCollisionBoxes)
			{
				if(HitBoxPair.Value != nullptr)
				{
					HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
					HitBoxPair.Value->SetCollisionResponseToChannel(ECC_HitBox, ECR_Block);
				}
			}
			//Trace against all the hitboxes
			World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_HitBox);
			if(HitResult.bBlockingHit) //TODO: Probably need to have more checks so that we can accurately tell which character actually got hit
			{
				if(HitResult.Component.IsValid())
				{
					UBoxComponent* Box = Cast<UBoxComponent>(HitResult.Component);
					if(Box)
					{
						DrawDebugBox(GetWorld(), Box->GetComponentLocation(), Box->GetScaledBoxExtent(), FQuat(Box->GetComponentRotation()), FColor::Blue, false, 7.f);
					}
				}
				ResetHitBoxes(HitCharacter, CurrentFrame);
				EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
				return FServerSideRewindResult{true, false};
			}
		}
	}
	ResetHitBoxes(HitCharacter, CurrentFrame);
	EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
	return FServerSideRewindResult{false, false};
}

FServerSideRewindResult ULagCompensationComponent::ProjectileConfirmHit(const FFramePackage& Package, ABlasterCharacter* HitCharacter,
	const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& InitialVelocity, const float HitTime)
{
	FFramePackage CurrentFrame;
	//Saving the HitBox information for later use.
	CacheBoxTransform(HitCharacter,CurrentFrame);
	MoveHitBoxes(HitCharacter, Package);
	//Disabling character mesh collision so that it won't interfere with the line trace we are about to do.
	EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::NoCollision);

	//First check hits for headshots
	UBoxComponent* HeadBox = HitCharacter->HitCollisionBoxes[FName("head")];
	HeadBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	HeadBox->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);

	FPredictProjectilePathResult PathResult;
	FPredictProjectilePathParams PathParams;
	PathParams.bTraceWithChannel = true;
	PathParams.bTraceWithCollision = true;
	//PathParams.DrawDebugTime = 5.f;
	//PathParams.DrawDebugType = EDrawDebugTrace::ForDuration;
	PathParams.LaunchVelocity = InitialVelocity;
	PathParams.MaxSimTime = MaxRecordTime;
	PathParams.ProjectileRadius = 5.f;
	PathParams.SimFrequency = 15.f;
	PathParams.StartLocation = TraceStart;
	PathParams.TraceChannel = ECC_HitBox;
	PathParams.ActorsToIgnore.Add(GetOwner());
	UGameplayStatics::PredictProjectilePath(this, PathParams, PathResult);

	if (PathResult.HitResult.bBlockingHit)
	{
		if(PathResult.HitResult.Component.IsValid())
		{
			UBoxComponent* Box = Cast<UBoxComponent>(PathResult.HitResult.Component);
			if(Box)
			{
				DrawDebugBox(GetWorld(), Box->GetComponentLocation(), Box->GetScaledBoxExtent(), FQuat(Box->GetComponentRotation()), FColor::Red, false, 7.f);
			}
		}
		
		ResetHitBoxes(HitCharacter, CurrentFrame);
		EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
		return FServerSideRewindResult{true, true};
	}
	else
	{
		for(auto& HitBoxPair : HitCharacter->HitCollisionBoxes)
		{
			if(HitBoxPair.Value != nullptr)
			{
				HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
				HitBoxPair.Value->SetCollisionResponseToChannel(ECC_HitBox, ECR_Block);
			}
		}

		UGameplayStatics::PredictProjectilePath(this, PathParams, PathResult);
		if(PathResult.HitResult.bBlockingHit)
		{
			if(PathResult.HitResult.Component.IsValid())
			{
				UBoxComponent* Box = Cast<UBoxComponent>(PathResult.HitResult.Component);
				if(Box)
				{
					DrawDebugBox(GetWorld(), Box->GetComponentLocation(), Box->GetScaledBoxExtent(), FQuat(Box->GetComponentRotation()), FColor::Blue, false, 7.f);
				}
			}
			
			ResetHitBoxes(HitCharacter, CurrentFrame);
			EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
			return FServerSideRewindResult{true, false};
		}
	}
	ResetHitBoxes(HitCharacter, CurrentFrame);
	EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
	return FServerSideRewindResult{false, false};
	
}

FShotgunServerSideRewindResult ULagCompensationComponent::ShotgunConfirmHit(const TArray<FFramePackage>& Packages,
	const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations)
{
	for(auto& Frame : Packages)
	{
		
		if(Frame.BlasterCharacter == nullptr)
		{
			UE_LOG(LogTemp, Warning, TEXT("Frame.BlasterCharacter is nullptr"))
			return FShotgunServerSideRewindResult();
		}
			
	}
	
	FShotgunServerSideRewindResult ShotgunResult;
	TArray<FFramePackage> CurrentFrames;
	for(auto& Frame : Packages)
	{
		
		FFramePackage CurrentFrame;
		CurrentFrame.BlasterCharacter = Frame.BlasterCharacter;
		CacheBoxTransform(Frame.BlasterCharacter,CurrentFrame);
		MoveHitBoxes(Frame.BlasterCharacter, Frame);
		EnableCharacterMeshCollision(Frame.BlasterCharacter, ECollisionEnabled::NoCollision);
		CurrentFrames.Add(CurrentFrame);
	}
	for(auto& Frame : Packages)
	{
		UBoxComponent* HeadBox = Frame.BlasterCharacter->HitCollisionBoxes[FName("head")];
		HeadBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		HeadBox->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);
	}
	UWorld* World = GetWorld();
	if(World)
	{
		//Headshots
		for(auto& HitLocation : HitLocations)
		{
			//UE_LOG(LogTemp, Warning, TEXT("Shubidubaa"))
			FHitResult HitResult;
			const FVector TraceEnd = TraceStart + (HitLocation - TraceStart) * 1.2f;
			World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_HitBox);
			if(HitResult.bBlockingHit)
			{
				if(ABlasterCharacter* BlasterChara = Cast<ABlasterCharacter>(HitResult.GetActor()))
				{
					if(ShotgunResult.HeadShots.Contains(BlasterChara))
					{
						
						ShotgunResult.HeadShots[BlasterChara]++;
						UE_LOG(LogTemp, Warning, TEXT("HeadShots: %d"), ShotgunResult.HeadShots[BlasterChara])
					}else
					{
						UE_LOG(LogTemp, Warning, TEXT("No Contains"))
						ShotgunResult.HeadShots.Emplace(BlasterChara, 1);
					}
				}
			}
		}

		//Enable collision for all boxes except head
		for(auto& Frame : Packages)
		{
			for(auto& HitBoxPair : Frame.BlasterCharacter->HitCollisionBoxes)
			{
				if(HitBoxPair.Value != nullptr)
				{
					HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
					HitBoxPair.Value->SetCollisionResponseToChannel(ECC_HitBox, ECR_Block);
				}
			}
			UBoxComponent* HeadBox = Frame.BlasterCharacter->HitCollisionBoxes[FName("head")];
			HeadBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
		
		//Body shots
		for(auto& HitLocation : HitLocations)
		{
			FHitResult HitResult;
			const FVector TraceEnd = TraceStart + (HitLocation - TraceStart) * 1.2f;
			World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_HitBox);
			if(HitResult.bBlockingHit)
			{
				UE_LOG(LogTemp, Warning, TEXT("Blocking Hit: %s"), *HitResult.GetActor()->GetName())
				if(ABlasterCharacter* BlasterChara = Cast<ABlasterCharacter>(HitResult.GetActor()))
				{
					if(ShotgunResult.BodyShots.Contains(BlasterChara))
					{
						ShotgunResult.BodyShots[BlasterChara]++;
					}else
					{
						ShotgunResult.BodyShots.Emplace(BlasterChara, 1);
					}
				}
			}
		}
	}

	for(auto& Frame : Packages)
	{
		ResetHitBoxes(Frame.BlasterCharacter, Frame);
		EnableCharacterMeshCollision(Frame.BlasterCharacter, ECollisionEnabled::QueryAndPhysics);
	}
	
	return ShotgunResult;
}



void ULagCompensationComponent::MoveHitBoxes(ABlasterCharacter* HitCharacter, const FFramePackage& Package)
{
	if(HitCharacter == nullptr) return;
	for(TTuple<FName, UBoxComponent*>& HitBoxPair : HitCharacter->HitCollisionBoxes)
	{
		if(!HitBoxPair.Key.IsValid()) continue;;
		if(HitBoxPair.Value != nullptr)
		{
			HitBoxPair.Value->SetWorldLocation(Package.HitBoxInfo[HitBoxPair.Key].BoxLocation);
			HitBoxPair.Value->SetWorldRotation(Package.HitBoxInfo[HitBoxPair.Key].BoxRotation);
			HitBoxPair.Value->SetBoxExtent(Package.HitBoxInfo[HitBoxPair.Key].BoxExtent);
		}
	}
}

void ULagCompensationComponent::ResetHitBoxes(ABlasterCharacter* HitCharacter, const FFramePackage& Package)
{
	if(HitCharacter == nullptr) return;
	for(auto& HitBoxPair : HitCharacter->HitCollisionBoxes)
	{
		if(HitBoxPair.Value != nullptr)
		{
			HitBoxPair.Value->SetWorldLocation(Package.HitBoxInfo[HitBoxPair.Key].BoxLocation);
			HitBoxPair.Value->SetWorldRotation(Package.HitBoxInfo[HitBoxPair.Key].BoxRotation);
			HitBoxPair.Value->SetBoxExtent(Package.HitBoxInfo[HitBoxPair.Key].BoxExtent);
			HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
}

void ULagCompensationComponent::EnableCharacterMeshCollision(ABlasterCharacter* HitCharacter,
	ECollisionEnabled::Type CollisionEnabled)
{
	if(HitCharacter && HitCharacter->GetMesh())
	{
		HitCharacter->GetMesh()->SetCollisionEnabled(CollisionEnabled);
	}
}

FFramePackage ULagCompensationComponent::GetFrameToCheck(ABlasterCharacter* HitCharacter, const float HitTime)
{
	bool bReturn =	HitCharacter == nullptr ||
					HitCharacter->GetLagCompensationComponent() == nullptr ||
					HitCharacter->GetLagCompensationComponent()->FrameHistory.GetHead() == nullptr ||
					HitCharacter->GetLagCompensationComponent()->FrameHistory.GetTail() == nullptr;

	if(bReturn) return FFramePackage();
	//FrameToCheck will ultimately be passed to ConfirmHit function
	FFramePackage FrameToCheck;
	FrameToCheck.Time = 0.f;
	bool bShouldInterpolate = true;
	//Making a reference to the original, just so that we don't have to type out the long line of code.
	const TDoubleLinkedList<FFramePackage>& History = HitCharacter->GetLagCompensationComponent()->FrameHistory;
	const float OldestHistoryTime = History.GetTail()->GetValue().Time;
	const float NewestHistoryTime = History.GetHead()->GetValue().Time;
	
	//Might look confusing, but if the time the client sends was earlier than oldest kept time, then we probably have already removed the FramePackage for that time and the client shouldn't hit anything, they are too laggy.
	if(OldestHistoryTime > HitTime) return FFramePackage();
	//If the oldest kept time is the same as the time the client claims to have hit the target, then we don't need to interpolate, since we already have the correct FramePackage for that time. EXTREMELY UNLIKELY, but doing it anyway.
	if(OldestHistoryTime == HitTime)
	{
		FrameToCheck = History.GetTail()->GetValue();
		bShouldInterpolate = false;
	}
	//If the newest kept time is the same as the client claims to have hit the target, or even newer, then we have already have the correct FramePackage so no need to Interpolate
	if(NewestHistoryTime <= HitTime)
	{
		FrameToCheck = History.GetHead()->GetValue();
		bShouldInterpolate = false;
	}
	//Need 2 different frame packages, so that we can determine between which 2 FramePackages the server saved is the hit time the client claims to have hit located
	TDoubleLinkedList<FFramePackage>::TDoubleLinkedListNode* Younger = History.GetHead();
	TDoubleLinkedList<FFramePackage>::TDoubleLinkedListNode* Older = Younger;

	//Iterating over the History until Older and Younger and between the time the client claimed to have hit the player.
	while (Older->GetValue().Time > HitTime)
	{
		if(Older->GetNextNode() == nullptr) break;
		Older = Older->GetNextNode();
		if(Older->GetValue().Time > HitTime) Younger = Older;
		
	}
	//If By some miracle the FramePackage has the exact same time as the HitTime, we don't need to interpolate. THIS IS EXTREMELY UNLIKELY SINCE IT'S FLOATS WERE TALKING ABOUT HERE
	if(Older->GetValue().Time == HitTime)
	{
		FrameToCheck = Older->GetValue();
		bShouldInterpolate = false;
	}
	
	if(bShouldInterpolate)
	{
		//Interpolating between the FramePackages that are on both sides of the HitTime. This will provide a more accurate result and fill the FrameToCheck package with the most accurate data.
		FrameToCheck = InterpBetweenFrames(Older->GetValue(), Younger->GetValue(), HitTime);
	}
	FrameToCheck.BlasterCharacter = HitCharacter;
	
	return FrameToCheck;
}

void ULagCompensationComponent::CacheBoxTransform(ABlasterCharacter* HitCharacter, FFramePackage& OutFramePackage)
{
	if(HitCharacter == nullptr) return;

	for(auto& HitBoxPair : HitCharacter->HitCollisionBoxes)
	{
		if(HitBoxPair.Value != nullptr)
		{
			FBoxInformation BoxInformation;
			BoxInformation.BoxLocation = HitBoxPair.Value->GetComponentLocation();
			BoxInformation.BoxRotation = HitBoxPair.Value->GetComponentRotation();
			BoxInformation.BoxExtent = HitBoxPair.Value->GetScaledBoxExtent();
			OutFramePackage.HitBoxInfo.Add(HitBoxPair.Key, BoxInformation);
		}
	}
}

void ULagCompensationComponent::ShowFramePackage(const FFramePackage& Package,const FColor& Color)
{
	for(auto& BoxInfo : Package.HitBoxInfo)
	{
		DrawDebugBox(GetWorld(), BoxInfo.Value.BoxLocation, BoxInfo.Value.BoxExtent, FQuat(BoxInfo.Value.BoxRotation), Color, false, 0.5f);
	}
}

FServerSideRewindResult ULagCompensationComponent::ServerSideRewind(ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart,
	const FVector_NetQuantize& HitLocation, const float HitTime)
{
	const FFramePackage FrameToCheck = GetFrameToCheck(HitCharacter, HitTime);

	//Lastly we can just return the result of ConfirmHit, which just checks with a line trace if we hit the character and if we hit the head.
	return ConfirmHit(FrameToCheck, HitCharacter, TraceStart, HitLocation);
}

FShotgunServerSideRewindResult ULagCompensationComponent::ShotgunServerSideRewind(
	const TArray<ABlasterCharacter*>& HitCharacters, const FVector_NetQuantize& TraceStart,
	const TArray<FVector_NetQuantize>& HitLocations, const float HitTime)
{
	
	TArray<FFramePackage> FramesToCheck;
	for(ABlasterCharacter* HitCharacter : HitCharacters)
	{
		UE_LOG(LogTemp, Warning, TEXT("HitCharacter: %s"), *HitCharacter->GetName())
		FramesToCheck.Add(GetFrameToCheck(HitCharacter, HitTime));
	}
	
	return ShotgunConfirmHit(FramesToCheck, TraceStart, HitLocations);
}

FServerSideRewindResult ULagCompensationComponent::ProjectileServerSideRewind(ABlasterCharacter* HitCharacter,
	const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& InitialVelocity, const float HitTime)
{
	const FFramePackage FrameToCheck = GetFrameToCheck(HitCharacter, HitTime);

	return ProjectileConfirmHit(FrameToCheck, HitCharacter, TraceStart, InitialVelocity, HitTime);
}


void ULagCompensationComponent::ServerScoreRequest_Implementation(ABlasterCharacter* HitCharacter,
                                                                  const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, const float HitTime,
                                                                  AWeapon* DamageCauser)
{
	const FServerSideRewindResult Confirm = ServerSideRewind(HitCharacter, TraceStart, HitLocation, HitTime);
	
	//Essentially, if client scored a hit, apply damage.
	if(HitCharacter && Confirm.bHitConfirmed && DamageCauser && BlasterCharacter)
	{
		UGameplayStatics::ApplyDamage(HitCharacter, DamageCauser->GetDamage(), BlasterCharacter->Controller, DamageCauser, UDamageType::StaticClass());
	}
}

void ULagCompensationComponent::ProjectileServerScoreRequest_Implementation(ABlasterCharacter* HitCharacter,
	const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& InitialVelocity, const float HitTime)
{
	const FServerSideRewindResult Confirm = ProjectileServerSideRewind(HitCharacter, TraceStart, InitialVelocity, HitTime);
	
	//Essentially, if client scored a hit, apply damage.
	if(HitCharacter && Confirm.bHitConfirmed && BlasterCharacter)
	{
		UGameplayStatics::ApplyDamage(HitCharacter, BlasterCharacter->GetEquippedWeapon()->GetDamage(), BlasterCharacter->Controller, BlasterCharacter->GetEquippedWeapon(), UDamageType::StaticClass());
	}
}

void ULagCompensationComponent::ServerShotgunScoreRequest_Implementation(
	const TArray<ABlasterCharacter*>& HitCharacters, const FVector_NetQuantize& TraceStart,
	const TArray<FVector_NetQuantize>& HitLocations, const float HitTime, AWeapon* DamageCauser)
{
	const FShotgunServerSideRewindResult Confirm = ShotgunServerSideRewind(HitCharacters, TraceStart, HitLocations, HitTime);
	
	if(BlasterCharacter && BlasterCharacter->GetEquippedWeapon() && BlasterCharacter->Controller)
	{
		UE_LOG(LogTemp, Warning, TEXT("ServerShotgunScoreRequest_Implementation"))
		for(auto& HitCharacter : HitCharacters)
		{
			if(HitCharacter == nullptr) continue;
			float TotalDamage{0.f};
			if(Confirm.BodyShots.Contains(HitCharacter))
			{
				
				TotalDamage += Confirm.BodyShots[HitCharacter] * BlasterCharacter->GetEquippedWeapon()->GetDamage();
			}
			if(Confirm.HeadShots.Contains(HitCharacter))
			{
				TotalDamage += Confirm.HeadShots[HitCharacter] * BlasterCharacter->GetEquippedWeapon()->GetDamage();
			}
			UE_LOG(LogTemp, Warning, TEXT("TotalDamage: %f"), TotalDamage)
			UE_LOG(LogTemp, Warning, TEXT("HitCharacter: %s"), *HitCharacter->GetName())
			UGameplayStatics::ApplyDamage(HitCharacter, TotalDamage, BlasterCharacter->Controller, BlasterCharacter->GetEquippedWeapon(), UDamageType::StaticClass());
		}
		
	}
}

