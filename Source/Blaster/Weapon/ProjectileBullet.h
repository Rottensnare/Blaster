// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"
#include "ProjectileBullet.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API AProjectileBullet : public AProjectile
{
	GENERATED_BODY()

public:

	//Initializes ProjectileMovementComponent values
	AProjectileBullet();

#if WITH_EDITOR
	//Keeps different blueprint values for the same thing in sync with each other
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	
protected:
	//Pretty much the same as Fire function for other weapon types
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& HitResult) override;
	virtual void BeginPlay() override;
};
