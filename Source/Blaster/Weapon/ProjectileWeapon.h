// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "ProjectileWeapon.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API AProjectileWeapon : public AWeapon
{
	GENERATED_BODY()

public:
	//Handles spawning the projectile actor and initializing its values
	virtual void Fire(const FVector& HitTarget) override;

private:


	UPROPERTY(EditAnywhere)
	TSubclassOf<class AProjectile> ProjectileClass;

	UPROPERTY(EditAnywhere)
	TSubclassOf<AProjectile> SSRProjectileClass; //Projectile for server-side rewind, only used by server
};
