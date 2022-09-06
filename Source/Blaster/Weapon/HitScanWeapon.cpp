// Fill out your copyright notice in the Description page of Project Settings.


#include "HitScanWeapon.h"

#include "Blaster/Blaster.h"
#include "Blaster/BlasterPlayerController.h"
#include "Blaster/BlasterComponents/LagCompensationComponent.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"

void AHitScanWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);
	
	APawn* InstigatorPawn = Cast<APawn>(GetOwner());
	AController* InstigatorController = InstigatorPawn->GetController();
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));
	if (MuzzleFlashSocket)
	{
		const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		const FVector Start = SocketTransform.GetLocation();
		FVector End = Start + (HitTarget - Start) * 1.25f;
		FHitResult HitResult;
		WeaponTraceHit(Start, HitTarget, HitResult);
		
		if(HitResult.bBlockingHit)
		{
			ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(HitResult.GetActor());
			if(BlasterCharacter)
			{
				if(InstigatorController)
				{
					if(HasAuthority() && !bUseServerSideRewind)
					{
						UGameplayStatics::ApplyDamage(BlasterCharacter, Damage, InstigatorController, this, UDamageType::StaticClass());
					}
					if(!HasAuthority() && bUseServerSideRewind)
					{
						OwnerCharacter = OwnerCharacter == nullptr ? Cast<ABlasterCharacter>(InstigatorPawn) : OwnerCharacter;
						if(OwnerCharacter && OwnerCharacter->IsLocallyControlled())
						{
							OwnerController = OwnerController == nullptr ? Cast<ABlasterPlayerController>(OwnerCharacter->Controller) : OwnerController;
							if(OwnerController && OwnerCharacter->GetLagCompensationComponent())
							{
								//ClientHitTime is the time on the server when the client hit the target on their machine
								const float ClientHitTime = OwnerController->GetServerTime() - OwnerController->SingleTripTime;
								OwnerCharacter->GetLagCompensationComponent()->ServerScoreRequest(BlasterCharacter, Start, HitTarget, ClientHitTime, this);
							}
						}
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
}

void AHitScanWeapon::MulticastSetImpactEffects_Implementation(EHitType HitType, const FVector_NetQuantize& Location, const FVector_NetQuantize& StartLocation)
{
	
	if(HitType == EHT_Character)
	{
		ImpactParticles = CharacterImpactParticles;
		ImpactSound = CharacterImpactSound;
	}
	else if(HitType == EHT_Other)
	{
		ImpactParticles = MetalImpactParticles;
		ImpactSound = MetalImpactSound;
	}
	else if(HitType == EHT_MAX)
	{
		ImpactParticles = nullptr;
		ImpactSound = nullptr;
	}
	
	ShowEffects(Location, StartLocation);
}

void AHitScanWeapon::WeaponTraceHit(const FVector& InTraceStart, const FVector& InHitTarget, FHitResult& OutHitResult)
{
	const FVector End = InTraceStart + (InHitTarget - InTraceStart) * 1.2f;
	
	UWorld* World = GetWorld();
	if(World)
	{
		
		World->LineTraceSingleByChannel(OutHitResult, InTraceStart, End, ECC_Visibility);

		FVector BeamEnd = End;
		if(OutHitResult.bBlockingHit)
		{
			BeamEnd = OutHitResult.ImpactPoint;
		}
		DrawDebugSphere(World, BeamEnd, 16.f, 12, FColor::Orange, false, 5.f);
	}
}

void AHitScanWeapon::ShowEffects(const FVector& Location, const FVector& StartLocation)
{
	if(ImpactParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(this, ImpactParticles, Location);
	}
	if(ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, Location);
	}
	if(BeamParticles)
	{
		UParticleSystemComponent* ParticleSystemComponent = UGameplayStatics::SpawnEmitterAtLocation(this, BeamParticles, StartLocation);
		if(ParticleSystemComponent)
		{
			ParticleSystemComponent->SetVectorParameter(FName("Target"), Location);
		}
	}
}

