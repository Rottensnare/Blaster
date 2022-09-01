// Fill out your copyright notice in the Description page of Project Settings.


#include "Shotgun.h"

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
			FHitResult HitResult;
			WeaponTraceHit(Start, HitTarget, HitResult);
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
		if(HasAuthority())
		{
			for(auto HitPair : HitActors)
			{
				if(InstigatorController && HitPair.Key)
				{
					UGameplayStatics::ApplyDamage(HitPair.Key, Damage * HitPair.Value, InstigatorController, this, UDamageType::StaticClass());
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
