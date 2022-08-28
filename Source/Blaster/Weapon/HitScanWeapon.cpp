// Fill out your copyright notice in the Description page of Project Settings.


#include "HitScanWeapon.h"

#include "Blaster/Blaster.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

void AHitScanWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	if(!HasAuthority()) return;
	const APawn* InstigatorPawn = Cast<APawn>(GetOwner());
	if(InstigatorPawn == nullptr) return;
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));
	if (MuzzleFlashSocket)
	{
		const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		const FVector Start = SocketTransform.GetLocation();
		FVector End = Start + (HitTarget - Start) * 1.2f;
		FHitResult HitResult;
		UWorld* World = GetWorld();
		if (World)
		{
			World->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility);
		}
		if(HitResult.bBlockingHit)
		{
			ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(HitResult.GetActor());
			if(BlasterCharacter)
			{
				if(InstigatorPawn->GetController())
				{
					UGameplayStatics::ApplyDamage(BlasterCharacter, Damage, InstigatorPawn->GetController(), this, UDamageType::StaticClass());
					MulticastSetImpactEffects(EHT_Character, HitResult.Location);
				}
			}
			else
			{
				MulticastSetImpactEffects(EHT_Other, HitResult.Location);
			}
		}
	}
}

void AHitScanWeapon::MulticastSetImpactEffects_Implementation(EHitType HitType, const FVector_NetQuantize& Location)
{
	
	if(HitType == EHT_Character)
	{
		ImpactParticles = CharacterImpactParticles;
		ImpactSound = CharacterImpactSound;
	}
	else
	{
		ImpactParticles = MetalImpactParticles;
		ImpactSound = MetalImpactSound;
	}
	
	ShowEffects(Location);
}

void AHitScanWeapon::ShowEffects(const FVector& Location)
{
	if(ImpactParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(this, ImpactParticles, Location);
	}
	if(ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, Location);
	}
}
