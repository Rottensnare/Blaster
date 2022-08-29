// Fill out your copyright notice in the Description page of Project Settings.


#include "Shotgun.h"

#include "Blaster/Character/BlasterCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"

void AShotgun::Fire(const FVector& HitTarget)
{
	
	AWeapon::Fire(HitTarget);
	if(!HasAuthority()) return;
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr) return;
	AController* InstigatorController = OwnerPawn->GetController();

	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (MuzzleFlashSocket)
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		FVector Start = SocketTransform.GetLocation();
		FVector End = Start + (HitTarget - Start) * 1.25f;
		//Number of hits each character was hit. Needed because shotgun can hit multiple characters per shot.
		TMap<ABlasterCharacter*, uint32> HitActors;
		for (uint32 i = 0; i < NumberOfPellets; i++)
		{
			FHitResult HitResult;
			WeaponTraceHit(Start, HitTarget, HitResult);
			if(HitResult.bBlockingHit)
			{
				if(ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(HitResult.GetActor()))
				{
					if(InstigatorController)
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
		for(auto HitPair : HitActors)
		{
			if(InstigatorController && HitPair.Key)
			{
				UGameplayStatics::ApplyDamage(HitPair.Key, Damage * HitPair.Value, InstigatorController, this, UDamageType::StaticClass());
			}
		}
	}
	
}
