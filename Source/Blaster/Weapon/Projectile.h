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
	

	bool bUseServerSideRewind{false};
	FVector_NetQuantize TraceStart;
	FVector_NetQuantize100 InitialVelocity; //Initial projectile velocity. Used in AProjectileWeapon::Fire
	
	UPROPERTY(EditAnywhere)
	float InitialSpeed{25000.f};

	//Projectile weapon values will override this value when spawning the projectile
	UPROPERTY(EditAnywhere)
	float Damage {10.f};

	//Projectile weapon values will override this value when spawning the projectile
	UPROPERTY(EditAnywhere)
	float HeadShotMultiplier{2.f};

protected:

	virtual void BeginPlay() override;

	//Simply calls MulticastSetImpactEffects
	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& HitResult);
	//calls ServerSetImpactEffects
	UFUNCTION()
	virtual void OnHitClient(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& HitResult);
	//Calls MulticastSetImpactEffects
	void ServerSetImpactEffects(AActor* OtherActor);
	//Sets impact effects and calls ShowEffects
	void MulticastSetImpactEffects(AActor* OtherActor);

	//Does nada currently
	virtual void ShowEffects();

	//Spawns a niagara trail system
	void SpawnTrailSystem();

	//Calls Destroy
	void DestroyTimerFinished();

	//Starts DestroyTimer
	void StartDestroyTimer();

	//Causes radial damage with falloff
	void ExplodeDamage();
	
	UPROPERTY(EditDefaultsOnly)
	class UStaticMeshComponent* ProjectileMesh;
	
	UPROPERTY(EditAnywhere)
	class UBoxComponent* CollisionBox; //Projectile collision

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

	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* TrailSystem;
	
	UPROPERTY()
	class UNiagaraComponent* TrailSystemComponent;

	FTimerHandle DestroyTimer;

	UPROPERTY(EditAnywhere)
	float DestroyTime{3.f};

		
	UPROPERTY(EditAnywhere)
	float InnerDamageRadius{100.f}; //Max damage inside this radius

	//Interpolates damage between this and InnerDamageRadius. Outside this radius no damage applied
	UPROPERTY(EditAnywhere)
	float OuterDamageRadius{350.f}; 

private:

	UPROPERTY(EditAnywhere)
	 UParticleSystem* Tracer;

	UPROPERTY()
	UParticleSystemComponent* TracerComponent;




public:
	
	
	

};
