// Fill out your copyright notice in the Description page of Project Settings.


#include "HitScanWeapon.h"

#include "Blaster/Blaster.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Particles/ParticleSystemComponent.h"
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
		FVector End = Start + (HitTarget - Start) * 1.25f;
		FHitResult HitResult;
		WeaponTraceHit(Start, HitTarget, HitResult);
		
		if(HitResult.bBlockingHit)
		{
			ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(HitResult.GetActor());
			if(BlasterCharacter)
			{
				if(InstigatorPawn->GetController())
				{
					UGameplayStatics::ApplyDamage(BlasterCharacter, Damage, InstigatorPawn->GetController(), this, UDamageType::StaticClass());
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
	const FVector End = bUseScatter ? TraceEndWithScatter(InTraceStart, InHitTarget) : InTraceStart + (InHitTarget - InTraceStart) * 1.2f;
	
	UWorld* World = GetWorld();
	if(World)
	{
		World->LineTraceSingleByChannel(OutHitResult, InTraceStart, End, ECC_Visibility);

		FVector BeamEnd = End;
		if(OutHitResult.bBlockingHit)
		{
			BeamEnd = OutHitResult.ImpactPoint;
		}
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

FVector AHitScanWeapon::TraceEndWithScatter(const FVector& TraceStart, const FVector& HitTarget)
{
	FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal();
	FVector SphereCenter = TraceStart + ToTargetNormalized * DistanceToSphere;

	FVector RandVec = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius);
	FVector EndLoc = SphereCenter + RandVec;
	FVector ToEndLoc = EndLoc - TraceStart;
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
