// Fill out your copyright notice in the Description page of Project Settings.


#include "Casing.h"

#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

// Sets default values
ACasing::ACasing():
ShellImpulse(8.f)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	CasingMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CasingMesh"));
	SetRootComponent(CasingMesh);

	CasingMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	CasingMesh->SetSimulatePhysics(true);
	CasingMesh->SetEnableGravity(true);
	CasingMesh->SetNotifyRigidBodyCollision(true);

}

void ACasing::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	FVector NormalImpulse, const FHitResult& HitResult)
{
	if(ShellHitCue && !bSoundPlayedAlready)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ShellHitCue, GetActorLocation());
		bSoundPlayedAlready = true;
	}
	
}

// Called when the game starts or when spawned
void ACasing::BeginPlay()
{
	Super::BeginPlay();

	CasingMesh->OnComponentHit.AddDynamic(this, &ACasing::OnHit);
	//TODO: Create a global TArray of vectors so that no need to call FMath::FRandRange for every bullet fired TWICE. Just take a value off of the Vector array and advance on the array
	FVector RandomVector{FVector(FMath::FRandRange(.75f, 1.25f), 1.f, FMath::FRandRange(.75f, 1.25f))}; 
	CasingMesh->AddImpulse(GetActorForwardVector() * (ShellImpulse * RandomVector));

	SetLifeSpan(5.f);
}


