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

	virtual void BeginPlay() override;

	void DestroyTimerFinished();

	virtual void Destroyed() override;

	virtual void ShowEffects() override;
	
	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* TrailSystem;

	//Sound the projectile makes while flying through the air
	UPROPERTY(EditAnywhere)
	class USoundCue* ProjectileSoundCue;

	UPROPERTY(EditAnywhere)
	USceneComponent* RocketTailLocation;

	UPROPERTY()
	UAudioComponent* ProjectileLoopComponent;

	UPROPERTY(EditAnywhere)
	USoundAttenuation* LoopingSoundAttenuation;

	UPROPERTY(VisibleAnywhere)
	class URocketMovementComponent* RocketMovementComponent;
	
private:

	UPROPERTY(EditDefaultsOnly)
	class UStaticMeshComponent* RocketMesh;

	UPROPERTY(EditAnywhere)
	float InnerDamageRadius{100.f};

	UPROPERTY(EditAnywhere)
	float OuterDamageRadius{350.f};

	FTimerHandle DestroyTimer;

	UPROPERTY(EditAnywhere)
	float DestroyTime{3.f};

	UPROPERTY()
	class UNiagaraComponent* TrailSystemComponent;


public:
	
};
