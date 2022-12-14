// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"
#include "ProjectileGrenade.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API AProjectileGrenade : public AProjectile
{
	GENERATED_BODY()

public:


	AProjectileGrenade();
	virtual void Destroyed() override; //Calls ExplodeDamage


protected:

	//Calls StartDestroyTimer, SpawnTrailSystem, and binds OnBounce to OnProjectileBounce
	virtual void BeginPlay() override;

	//TODO: Make it so that the grenade explodes if it hits a pawn, also might need to check if pawn is friendly
	//Just plays bounce sound
	UFUNCTION()
	void OnBounce(const FHitResult& ImpactResult, const FVector& ImpactVelocity);


private:


	UPROPERTY(EditAnywhere)
	class USoundCue* BounceSoundCue;
	
};
