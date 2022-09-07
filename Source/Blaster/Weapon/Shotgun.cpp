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
		for(FVector_NetQuantize HitTarget : HitTargets)
		{
			const FVector End = Start + (HitTarget - Start) * 1.25f;
			FHitResult HitResult;
			WeaponTraceHit(Start, HitTarget, HitResult);
			if(HitResult.bBlockingHit)
			{
				if(ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(HitResult.GetActor()))
				{
					if(HitActors.Contains(BlasterCharacter))
					{
						HitActors[BlasterCharacter]++;
					}else
					{
						HitActors.Emplace(BlasterCharacter, 1);
					}
					MulticastSetImpactEffects(EHT_Character, HitResult.Location, SocketTransform.GetLocation());
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

		TArray<ABlasterCharacter*> HitCharacters;
		
		for(auto HitPair : HitActors)
		{
			if(InstigatorController && HitPair.Key)
			{
				bool bCauseAuthDamage = !bUseServerSideRewind || OwnerPawn->IsLocallyControlled();
				if(HasAuthority() && bCauseAuthDamage)
				{
					UGameplayStatics::ApplyDamage(HitPair.Key, Damage * HitPair.Value, InstigatorController, this, UDamageType::StaticClass());
				}
				HitCharacters.Add(HitPair.Key);
			}
		}
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
	
	const FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal();
	const FVector SphereCenter = TraceStart + ToTargetNormalized * DistanceToSphere;
	
	for (uint32 i = 0; i < NumberOfPellets; i++)
	{
		const FVector RandVec = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius);
		const FVector EndLoc = SphereCenter + RandVec;
		FVector ToEndLoc = EndLoc - TraceStart;
		ToEndLoc = TraceStart + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size();
		OutHitTargets.Add(ToEndLoc);
		
	}
}
