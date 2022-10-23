// Fill out your copyright notice in the Description page of Project Settings.


#include "Shotgun.h"

#include "Blaster/BlasterPlayerController.h"
#include "Blaster/BlasterComponents/LagCompensationComponent.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"


void AShotgun::FireShotgun(const TArray<FVector_NetQuantize>& HitTargets)
{
	AWeapon::Fire(FVector());
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr) return;
	AController* InstigatorController = OwnerPawn->GetController();
	
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (MuzzleFlashSocket)
	{
		const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		const FVector Start = SocketTransform.GetLocation();
		
		//Tracks how many pellets hit which character
		TMap<ABlasterCharacter*, uint32> HitActors;
		//Tracks how many headshots per character
		TMap<ABlasterCharacter*, uint32> HeadshotHitMap;
		
		//Iterating through all the passed in hit targets
		for(FVector_NetQuantize HitTarget : HitTargets)
		{
			const FVector End = Start + (HitTarget - Start) * 1.25f;
			FHitResult HitResult;
			WeaponTraceHit(Start, HitTarget, HitResult); //Line trace for each hit target
			if(HitResult.bBlockingHit)
			{
				if(ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(HitResult.GetActor()))
				{
					const bool bHeadShot = HitResult.BoneName == FName("head");
					if(bHeadShot) //If headshot, increase HeadshotHitMap value by one based on character hit
					{
						if(HitActors.Contains(BlasterCharacter)) HeadshotHitMap[BlasterCharacter]++;
						else HeadshotHitMap.Emplace(BlasterCharacter, 1);
					}
					else //If Bodyshot, increase HitActors value by one based on character hit
					{
						if(HitActors.Contains(BlasterCharacter)) HitActors[BlasterCharacter]++;
						else HitActors.Emplace(BlasterCharacter, 1);
					}
					//BUG: Crash Log says the following line will might cause the crash. Map.h pair != null assertion failed, Line 656
					MulticastSetImpactEffects(EHT_Character, HitResult.Location, SocketTransform.GetLocation()); //TODO: Make a version for shotgun, so that we don't call this multiple times per frame
				}
				else
				{
					MulticastSetImpactEffects(EHT_Other, HitResult.Location, SocketTransform.GetLocation());
				}
			}
			else
			{
				MulticastSetImpactEffects(EHT_MAX, End, SocketTransform.GetLocation());
			}
			
		}

		TArray<ABlasterCharacter*> HitCharacters; //Used for iterating all the hit characters and applying damage at once instead of once per pellet
		
		TMap<ABlasterCharacter*, float> DamageMap; //How much damage each character took
		for(auto HitPair : HitActors) //Calculating damage done with bodyshots
		{
			if(HitPair.Key)
			{
				DamageMap.Emplace(HitPair.Key, HitPair.Value * Damage);
				HitCharacters.AddUnique(HitPair.Key);
			}
		}
		for(auto HitPair : HeadshotHitMap) //Calculating damage done with headshots
		{
			if(HitPair.Key)
			{
				if(DamageMap.Contains(HitPair.Key)) HeadshotHitMap[HitPair.Key] += HitPair.Value * Damage * HeadshotMultiplier;
				else DamageMap.Emplace(HitPair.Key, HitPair.Value * Damage * HeadshotMultiplier);
				HitCharacters.AddUnique(HitPair.Key); //Needs to be unique, otherwise damage received might be multiple times that of the actual damage intended
			}
		}
		for(auto DamagePair : DamageMap) //Apply damage if server
		{
			if(DamagePair.Key && InstigatorController)
			{
				bool bCauseAuthDamage = !bUseServerSideRewind || OwnerPawn->IsLocallyControlled();
				if(HasAuthority() && bCauseAuthDamage)
				{
					UGameplayStatics::ApplyDamage(DamagePair.Key, DamagePair.Value, InstigatorController, this, UDamageType::StaticClass());
				}
			}
		}
		//For clients and if the weapon uses Server-Side Rewind
		if(!HasAuthority() && bUseServerSideRewind)
		{
			OwnerCharacter = OwnerCharacter == nullptr ? Cast<ABlasterCharacter>(OwnerPawn) : OwnerCharacter;
			if(OwnerCharacter && OwnerCharacter->IsLocallyControlled())
			{
				OwnerController = OwnerController == nullptr ? Cast<ABlasterPlayerController>(OwnerCharacter->Controller) : OwnerController;
				if(OwnerController && OwnerCharacter->GetLagCompensationComponent() && OwnerCharacter->IsLocallyControlled())
				{
					//ClientHitTime is the time on the server when the client hit the target on their machine
					const float ClientHitTime = OwnerController->GetServerTime() - OwnerController->SingleTripTime;
					OwnerCharacter->GetLagCompensationComponent()->ServerShotgunScoreRequest(HitCharacters, Start, HitTargets, ClientHitTime, this);
				}
			}
		}
	}
}

void AShotgun::ShotgunTraceEndWithScatter(const FVector& HitTarget, TArray<FVector_NetQuantize>& OutHitTargets)
{
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));
	if (MuzzleFlashSocket == nullptr) return;
	
	const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
	const FVector TraceStart = SocketTransform.GetLocation();
	
	const FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal(); //GetSafeNormal returns ZeroVector if vector is too small to normalize
	const FVector SphereCenter = TraceStart + ToTargetNormalized * DistanceToSphere;
	
	for (uint32 i = 0; i < NumberOfPellets; i++) 
	{
		const FVector RandVec = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius);
		const FVector EndLoc = SphereCenter + RandVec;
		FVector ToEndLoc = EndLoc - TraceStart;
		ToEndLoc = TraceStart + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size(); //TRACE_LENGTH defined in WeaponType.h. Currently 5000.f
		OutHitTargets.Add(ToEndLoc); 
		
	}
}
