// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileGrenade.h"

#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

AProjectileGrenade::AProjectileGrenade()
{
	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GrenadeProjectileMesh"));
	ProjectileMesh->SetupAttachment(RootComponent);
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->SetIsReplicated(true);
	ProjectileMovementComponent->bShouldBounce = true;
}



void AProjectileGrenade::BeginPlay()
{
	ImpactParticles = MetalImpactParticles;
	ImpactSound = MetalImpactSound;
	
	AActor::BeginPlay();

	StartDestroyTimer();
	SpawnTrailSystem();

	ProjectileMovementComponent->OnProjectileBounce.AddDynamic(this, &AProjectileGrenade::OnBounce);

	
}

void AProjectileGrenade::OnBounce(const FHitResult& ImpactResult, const FVector& ImpactVelocity)
{
	if(BounceSoundCue) //TODO: Add a timer or something so that the sound won't be so obnoxious at times
	{
		UGameplayStatics::PlaySoundAtLocation(this, BounceSoundCue, GetActorLocation());
	}
}

void AProjectileGrenade::Destroyed()
{
	ExplodeDamage();
	Super::Destroyed();

	
}