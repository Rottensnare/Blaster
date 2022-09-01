// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "HitScanWeapon.generated.h"

/**
 * 
 */

//Used to determine what type of object was hit. Saves network bandwidth compared to sending an AActor* across the network.
UENUM(BlueprintType)
enum EHitType 
{
	EHT_Character,
	EHT_Other,

	EHT_MAX
};

UCLASS()
class BLASTER_API AHitScanWeapon : public AWeapon
{
	GENERATED_BODY()

public:

	virtual void Fire(const FVector& HitTarget) override;


protected:

	UFUNCTION(NetMulticast, Reliable)
	void MulticastSetImpactEffects(EHitType HitType, const FVector_NetQuantize& Location,  const FVector_NetQuantize& StartLocation = FVector());
	void WeaponTraceHit(const FVector& InTraceStart, const FVector& InHitTarget, FHitResult& OutHitResult);

	virtual void ShowEffects(const FVector& Location, const FVector& StartLocation = FVector());
	
	UPROPERTY(VisibleAnywhere)
	class UParticleSystem* ImpactParticles;

	UPROPERTY(EditAnywhere)
	UParticleSystem* CharacterImpactParticles;

	UPROPERTY(EditAnywhere)
	UParticleSystem* MetalImpactParticles;
	
	UPROPERTY(VisibleAnywhere)
	class USoundCue* ImpactSound;

	UPROPERTY(EditAnywhere)
	USoundCue* CharacterImpactSound;

	UPROPERTY(EditAnywhere)
	USoundCue* MetalImpactSound;
	
	UPROPERTY(EditAnywhere)
	float Damage{10.f};


private:

	UPROPERTY(EditAnywhere)
	UParticleSystem* BeamParticles;
	
};
