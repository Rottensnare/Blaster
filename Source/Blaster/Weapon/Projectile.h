// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

UCLASS()
class BLASTER_API AProjectile : public AActor
{
	GENERATED_BODY()
	
public:	

	AProjectile();
	virtual void Tick(float DeltaTime) override;
	virtual void Destroyed() override;


protected:

	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& HitResult);
	UFUNCTION()
	virtual void OnHitClient(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& HitResult);
	
	void ServerSetImpactEffects(AActor* OtherActor);
	
	void MulticastSetImpactEffects(AActor* OtherActor);

	virtual void ShowEffects();
	
	UPROPERTY(EditAnywhere)
	float Damage {10.f};

	
	UPROPERTY(EditAnywhere)
	class UBoxComponent* CollisionBox;

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

	UPROPERTY(VisibleAnywhere)
	class UProjectileMovementComponent* ProjectileMovementComponent;

private:
	
	

	UPROPERTY(EditAnywhere)
	 UParticleSystem* Tracer;

	UPROPERTY()
	UParticleSystemComponent* TracerComponent;




public:
	
	
	

};
