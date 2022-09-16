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

	//Handles firing the hitscan weapon. More info in the .cpp file
	virtual void Fire(const FVector& HitTarget) override;


protected:

	//Sets impact effects (Sound and particles) based on the object hit. Currently only 2 different types
	//Calls ShowEffects with Location and StartLocation
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastSetImpactEffects(EHitType HitType, const FVector_NetQuantize& Location,  const FVector_NetQuantize& StartLocation = FVector());
	//Performs a line trace based on passed in parameters. Gets the HitResult as an Out Parameter.
	void WeaponTraceHit(const FVector& InTraceStart, const FVector& InHitTarget, FHitResult& OutHitResult);

	//Plays particle and sound effects 
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


private:

	UPROPERTY(EditAnywhere)
	UParticleSystem* BeamParticles; //Currently a smoke trail for bullets
	
};
