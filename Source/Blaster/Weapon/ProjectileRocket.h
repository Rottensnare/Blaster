// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"
#include "ProjectileRocket.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API AProjectileRocket : public AProjectile
{
	GENERATED_BODY()

public:
	AProjectileRocket();


protected:
	
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& HitResult) override;


private:

	UPROPERTY(EditDefaultsOnly)
	class UStaticMeshComponent* RocketMesh;

	UPROPERTY(EditAnywhere)
	float InnerDamageRadius{100.f};

	UPROPERTY(EditAnywhere)
	float OuterDamageRadius{350.f};


public:
	
};
