// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectile.h"

#include "Blaster/Blaster.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Components/BoxComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Sound/SoundCue.h"

// Sets default values
AProjectile::AProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	SetRootComponent(CollisionBox);
	CollisionBox->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECR_Block);
	CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECR_Block);
	CollisionBox->SetCollisionResponseToChannel(ECC_SkeletalMesh, ECR_Block);
	
}

// Called when the game starts or when spawned
void AProjectile::BeginPlay()
{
	Super::BeginPlay();

	if(HasAuthority())
	{
		CollisionBox->OnComponentHit.AddDynamic(this, &AProjectile::OnHit);
	}
	
	CollisionBox->OnComponentHit.AddDynamic(this, &AProjectile::OnHitClient);
	

	if(Tracer)
	{
		TracerComponent = UGameplayStatics::SpawnEmitterAttached(Tracer, CollisionBox, NAME_None, GetActorLocation(), GetActorRotation(), EAttachLocation::KeepWorldPosition);
	}
}

void AProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	FVector NormalImpulse, const FHitResult& HitResult)
{
	
	MulticastSetImpactEffects(OtherActor);
	
	Destroy();
}

void AProjectile::OnHitClient(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& HitResult)
{
	ServerSetImpactEffects(OtherActor);
}

void AProjectile::ServerSetImpactEffects(AActor* OtherActor)
{
	MulticastSetImpactEffects(OtherActor);
}

void AProjectile::MulticastSetImpactEffects(AActor* OtherActor)
{
	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
	if(BlasterCharacter)
	{
		ImpactParticles = CharacterImpactParticles;
		ImpactSound = CharacterImpactSound;
	}
	else
	{
		ImpactParticles = MetalImpactParticles;
		ImpactSound = MetalImpactSound;
	}

	ShowEffects();
}

void AProjectile::ShowEffects()
{
	
}


void AProjectile::Destroyed() //Replicated to clients
{
	
	if(ImpactParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, GetActorTransform());
	}
	if(ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
	}
}



