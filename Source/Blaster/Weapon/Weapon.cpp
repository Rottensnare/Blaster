// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"

#include "Casing.h"
#include "Blaster/BlasterPlayerController.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"


// Sets default values
AWeapon::AWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	SetRootComponent(WeaponMesh);

	WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECR_Ignore);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh->SetCustomDepthStencilValue(CustomDepthValue);
	WeaponMesh->MarkRenderStateDirty();
	EnableCustomDepth(true);

	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));
	AreaSphere->SetupAttachment(GetRootComponent());
	AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	PickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidget"));
	PickupWidget->SetupAttachment(RootComponent);

	
}

void AWeapon::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	for(int i = 0; i < 5; i++) //If someone forgets to put the crosshairs, this prevents a crash.
		{
			if(Crosshairs.Num() < 5)
			{
				Crosshairs.Add(nullptr);
			}
		}
}

// Called when the game starts or when spawned
void AWeapon::BeginPlay()
{
	Super::BeginPlay();
	if(PickupWidget)
	{
		PickupWidget->SetVisibility(false);
	}
	
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	AreaSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::AWeapon::OnSphereOverlap);
	AreaSphere->OnComponentEndOverlap.AddDynamic(this, &ThisClass::AWeapon::OnSphereEndOverlap);
}

// Called every frame
void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if(WeaponState == EWeaponState::EWS_Initial)
	{
		if(WeaponMesh)
		{
			WeaponMesh->AddLocalRotation(FRotator(0.f, BaseTurnRate * DeltaTime, 0.f));
		}
	}

}

void AWeapon::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
	if(BlasterCharacter)
	{
		BlasterCharacter->SetOverlappingWeapon(this);
	}
}

void AWeapon::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
	if(BlasterCharacter)
	{
		BlasterCharacter->SetOverlappingWeapon(nullptr);
	}
}

void AWeapon::OnPingTooHigh(bool bPingTooHigh)
{
	bUseServerSideRewind = !bPingTooHigh;
}

void AWeapon::OnRep_WeaponState()
{
	switch(WeaponState)
	{
	case EWeaponState::EWS_Equipped:
		ShowPickupWidget(false);
		WeaponMesh->SetEnableGravity(false);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		if(WeaponType == EWeaponType::EWT_SubmachineGun)
		{
			WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			WeaponMesh->SetEnableGravity(true);
			WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		}
		EnableCustomDepth(false);
		break;
	case EWeaponState::EWS_Initial:
		break;
	case EWeaponState::EWS_Dropped:
		WeaponMesh->SetSimulatePhysics(true);
		WeaponMesh->SetEnableGravity(true);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
		WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECR_Ignore);
		WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECR_Ignore);
		WeaponMesh->SetCustomDepthStencilValue(CustomDepthValue);
		WeaponMesh->MarkRenderStateDirty();
		EnableCustomDepth(true);
		break;
	case EWeaponState::EWS_UnEquipped:
		ShowPickupWidget(false);
		WeaponMesh->SetEnableGravity(false);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		if(WeaponType == EWeaponType::EWT_SubmachineGun)
		{
			WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			WeaponMesh->SetEnableGravity(true);
			WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		}
		EnableCustomDepth(false);
		break;
	default:
		break;
	}
}

void AWeapon::ClientUpdateAmmo_Implementation(int32 ServerAmmo)
{
	if(HasAuthority()) return;
	Ammo = ServerAmmo;
	--Sequence;
	Ammo -= Sequence;
	SetHUDAmmo();
}

void AWeapon::ClientAddAmmo_Implementation(int32 AmmoToAdd)
{
	if(HasAuthority()) return;
	Ammo = FMath::Clamp(Ammo + AmmoToAdd, 0, MagCapacity);
	OwnerCharacter = OwnerCharacter == nullptr ? Cast<ABlasterCharacter>(GetOwner()) : OwnerCharacter;
	SetHUDAmmo();
}

void AWeapon::SpendRound()
{
	Ammo = FMath::Clamp(Ammo -1, 0, MagCapacity);
	SetHUDAmmo();
	if(HasAuthority())
	{
		ClientUpdateAmmo(Ammo);
	}
	else
	{
		Sequence++;
	}
}

void AWeapon::AddAmmo(int32 AmmoToAdd)
{
	Ammo = FMath::Clamp(Ammo + AmmoToAdd, 0, MagCapacity);
	SetHUDAmmo();
	ClientAddAmmo(AmmoToAdd);
}

void AWeapon::SetWeaponState(EWeaponState State)
{
	//This should only happen once, because the state is only set to EWS_Initial when spawning it from a weapon spawner
	//Later unused weapons will be despawned by the game mode
	if(WeaponState == EWeaponState::EWS_Initial && State != EWeaponState::EWS_Initial)
	{
		OnPickedUpDelegate.Broadcast();
	}
	WeaponState = State;
	switch(WeaponState)
	{
	case EWeaponState::EWS_Equipped:
		ShowPickupWidget(false);
		SetReplicateMovement(false);
		WeaponMesh->SetSimulatePhysics(false);
		WeaponMesh->SetEnableGravity(false);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		if(WeaponType == EWeaponType::EWT_SubmachineGun)
		{
			WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			WeaponMesh->SetEnableGravity(true);
			WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		}
		EnableCustomDepth(false);

		OwnerCharacter = OwnerCharacter == nullptr ? Cast<ABlasterCharacter>(GetOwner()) : OwnerCharacter;
		if(OwnerCharacter && bUseServerSideRewind)
		{
			//UE_LOG(LogTemp, Warning, TEXT("Binding"))
			OwnerController = OwnerController == nullptr ? Cast<ABlasterPlayerController>(OwnerCharacter->Controller) : OwnerController;
			if(OwnerController)
			{
				if(!OwnerController->OnHighPingChecked.IsBound())
				{
					OwnerController->OnHighPingChecked.AddDynamic(this, &ThisClass::OnPingTooHigh);
				}
			}
		}
		
		break;
	case EWeaponState::EWS_Initial:
		if(HasAuthority())
		{
			AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
			SetReplicateMovement(true);
		}
		WeaponMesh->SetSimulatePhysics(false);
		WeaponMesh->SetEnableGravity(false);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		WeaponMesh->SetCustomDepthStencilValue(CustomDepthValue);
		WeaponMesh->MarkRenderStateDirty();
		EnableCustomDepth(true);
		break;
	case EWeaponState::EWS_Dropped:
		if(HasAuthority())
		{
			AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
			SetReplicateMovement(true);
		}
		WeaponMesh->SetSimulatePhysics(true);
		WeaponMesh->SetEnableGravity(true);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
		WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECR_Ignore);
		WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECR_Ignore);
		WeaponMesh->SetCustomDepthStencilValue(CustomDepthValue);
		WeaponMesh->MarkRenderStateDirty();
		EnableCustomDepth(true);
		OwnerCharacter = OwnerCharacter == nullptr ? Cast<ABlasterCharacter>(GetOwner()) : OwnerCharacter;
		if(OwnerCharacter && bUseServerSideRewind)
		{
			OwnerController = OwnerController == nullptr ? Cast<ABlasterPlayerController>(OwnerCharacter->Controller) : OwnerController;
			if(OwnerController)
			{
				if(OwnerController->OnHighPingChecked.IsBound())
				{
					OwnerController->OnHighPingChecked.RemoveDynamic(this, &ThisClass::OnPingTooHigh);
				}
			}
		}
		break;
	case EWeaponState::EWS_UnEquipped:
		ShowPickupWidget(false);
		SetReplicateMovement(false);
		WeaponMesh->SetSimulatePhysics(false);
		WeaponMesh->SetEnableGravity(false);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		if(WeaponType == EWeaponType::EWT_SubmachineGun)
		{
			WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			WeaponMesh->SetEnableGravity(true);
			WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		}
		EnableCustomDepth(false);
		if(OwnerCharacter && bUseServerSideRewind)
		{
			OwnerController = OwnerController == nullptr ? Cast<ABlasterPlayerController>(OwnerCharacter->Controller) : OwnerController;
			if(OwnerController)
			{
				if(OwnerController->OnHighPingChecked.IsBound())
				{
					OwnerController->OnHighPingChecked.RemoveDynamic(this, &ThisClass::OnPingTooHigh);
				}
			}
		}
		break;
	default:
		break;
	}
}

void AWeapon::ShowPickupWidget(bool bShowWidget)
{
	if(PickupWidget)
	{
		PickupWidget->SetVisibility(bShowWidget);
	}
}

void AWeapon::Fire(const FVector& HitTarget)
{
	if(FireAnimation)
	{
		WeaponMesh->PlayAnimation(FireAnimation, false);
	}
	if(CasingClass)
	{
		const USkeletalMeshSocket* AmmoEject = WeaponMesh->GetSocketByName(FName("AmmoEject"));
		if (AmmoEject)
		{
			FTransform SocketTransform = AmmoEject->GetSocketTransform(WeaponMesh);
			if (CasingClass)
			{
				UWorld* World = GetWorld();
				FActorSpawnParameters SpawnParameters;
				if (World)
				{
					World->SpawnActor<ACasing>(CasingClass, SocketTransform.GetLocation(), SocketTransform.GetRotation().Rotator());
				}
			}
		}
	}
	
	SpendRound();
	
}

void AWeapon::Dropped()
{
	SetWeaponState(EWeaponState::EWS_Dropped);
	FDetachmentTransformRules DetachmentTransformRules(EDetachmentRule::KeepWorld, true);
	WeaponMesh->DetachFromComponent(DetachmentTransformRules);
	SetOwner(nullptr);
	OwnerCharacter = nullptr;
	OwnerController = nullptr;
}

void AWeapon::OnRep_Owner()
{
	Super::OnRep_Owner();
	
	if(Owner == nullptr)
	{
		OwnerCharacter = nullptr;
		OwnerController = nullptr;
	}else
	{
		SetHUDMagAmmo();
		SetHUDWeaponType();
		if(OwnerCharacter)
		{
			OwnerCharacter->UpdateHUDAmmo();
		}
	}
}

void AWeapon::SetHUDAmmo()
{
	OwnerCharacter = OwnerCharacter == nullptr ? Cast<ABlasterCharacter>(GetOwner()) : OwnerCharacter;
	if(OwnerCharacter)
	{
		OwnerController = OwnerController == nullptr ? Cast<ABlasterPlayerController>(OwnerCharacter->Controller) : OwnerController;
		if(OwnerController)
		{
			OwnerCharacter->UpdateHUDAmmo();
		}
	}
}

void AWeapon::SetHUDMagAmmo()
{

	OwnerCharacter = OwnerCharacter == nullptr ? Cast<ABlasterCharacter>(GetOwner()) : OwnerCharacter;
	if(OwnerCharacter)
	{
		OwnerController = OwnerController == nullptr ? Cast<ABlasterPlayerController>(OwnerCharacter->Controller) : OwnerController;
		if(OwnerController)
		{
			OwnerController->SetHUDMagText(MagCapacity);
		}
	}
}

void AWeapon::SetHUDWeaponType()
{
	OwnerCharacter = OwnerCharacter == nullptr ? Cast<ABlasterCharacter>(GetOwner()) : OwnerCharacter;
	if(OwnerCharacter)
	{
		OwnerController = OwnerController == nullptr ? Cast<ABlasterPlayerController>(OwnerCharacter->Controller) : OwnerController;
		if(OwnerController)
		{
			FText WeaponText = UEnum::GetDisplayValueAsText(WeaponType);
			OwnerController->SetHUDWeaponType(WeaponText.ToString());
		}
	}
}

void AWeapon::EnableCustomDepth(bool bEnable)
{
	if(WeaponMesh)
	{
		WeaponMesh->SetRenderCustomDepth(bEnable);
	}
}

FVector AWeapon::TraceEndWithScatter(const FVector& HitTarget)
{
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));
	if (MuzzleFlashSocket == nullptr) return FVector();
	
	const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
	const FVector TraceStart = SocketTransform.GetLocation();
	
	const FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal();
	const FVector SphereCenter = TraceStart + ToTargetNormalized * DistanceToSphere;
	const FVector RandVec = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius);
	const FVector EndLoc = SphereCenter + RandVec;
	const FVector ToEndLoc = EndLoc - TraceStart;
	/*
		DrawDebugSphere(GetWorld(), SphereCenter, SphereRadius, 12, FColor::Red, true);
		DrawDebugSphere(GetWorld(), EndLoc, 4.f, 12, FColor::Orange, true);
		DrawDebugLine(
			GetWorld(),
			TraceStart,
			FVector(TraceStart + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size()),
			FColor::Cyan,
			true);
	*/
	return FVector(TraceStart + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size());
}

void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWeapon, WeaponState)
	DOREPLIFETIME_CONDITION(AWeapon, bUseServerSideRewind, COND_OwnerOnly)
}
