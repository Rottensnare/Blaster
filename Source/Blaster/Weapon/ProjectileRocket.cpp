// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileRocket.h"
#include "Niagara/Classes/NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "RocketMovementComponent.h"
#include "Components/AudioComponent.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

AProjectileRocket::AProjectileRocket()
{

	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RocketMesh"));
	ProjectileMesh->SetupAttachment(RootComponent);
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	RocketTailLocation = CreateDefaultSubobject<USceneComponent>(TEXT("RocketTailLocation"));
	RocketTailLocation->SetupAttachment(RootComponent);

	RocketMovementComponent = CreateDefaultSubobject<URocketMovementComponent>(TEXT("RocketMovementComponent"));
	RocketMovementComponent->bRotationFollowsVelocity = true;
	RocketMovementComponent->SetIsReplicated(true);
	RocketMovementComponent->ProjectileGravityScale = 0.4f;
	RocketMovementComponent->InitialSpeed = 12000.f;
	RocketMovementComponent->MaxSpeed = 12000.f;
}

void AProjectileRocket::BeginPlay()
{
	Super::BeginPlay();

	SpawnTrailSystem();
	
	if(ProjectileSoundCue && LoopingSoundAttenuation)
	{
		ProjectileLoopComponent = UGameplayStatics::SpawnSoundAttached(
			ProjectileSoundCue,
			GetRootComponent(),
			FName(),
			GetActorLocation(),
			EAttachLocation::KeepWorldPosition,
			false,
			1.f,
			1.f,
			0.f, LoopingSoundAttenuation,
			(USoundConcurrency*)nullptr,
			false);
	}
}



void AProjectileRocket::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                              FVector NormalImpulse, const FHitResult& HitResult)
{
	if(OtherActor == GetOwner())
	{
		UE_LOG(LogTemp, Warning, TEXT("Stop hitting yourself!"))
		return;
	}
	APawn* FiringPawn = GetInstigator();
	if(FiringPawn)
	{
		AController* FiringController = FiringPawn->GetController();
		if(FiringController)
		{
			UGameplayStatics::ApplyRadialDamageWithFalloff(
				this,
				Damage,
				Damage / 8.f,
				GetActorLocation(),
				InnerDamageRadius,
				OuterDamageRadius,
				1.f,
				UDamageType::StaticClass(),
				TArray<AActor*>(),
				this,
				FiringController
				);
		}
	}
	
	MulticastSetImpactEffects(OtherActor);
	StartDestroyTimer();
}

void AProjectileRocket::Destroyed()
{
	
}

void AProjectileRocket::ShowEffects()
{
	Super::ShowEffects();

	if(ProjectileLoopComponent)
	{
		ProjectileLoopComponent->Stop();
		ProjectileLoopComponent->DestroyComponent();
	}

	if(ProjectileMesh)
	{
		ProjectileMesh->SetVisibility(false);
		
	}
	if(CollisionBox)
	{
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	if(TrailSystemComponent && TrailSystemComponent->GetSystemInstanceController())
	{
		TrailSystemComponent->GetSystemInstanceController()->Deactivate();
	}

	if(ImpactParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, GetActorTransform());
	}
	if(ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
	}
}
