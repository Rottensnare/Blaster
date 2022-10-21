// Fill out your copyright notice in the Description page of Project Settings.


#include "PickupSpawnPoint.h"

#include "Pickup.h"
#include "SpeedPickup.h"

// Sets default values
APickupSpawnPoint::APickupSpawnPoint()
{

	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

}


void APickupSpawnPoint::BeginPlay()
{
	Super::BeginPlay();

	if(HasAuthority())
	{
		SpawnPickup();
	}
}

void APickupSpawnPoint::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void APickupSpawnPoint::SpawnPickup()
{
	int32 NumOfPickupClasses = PickupClasses.Num();
	if(NumOfPickupClasses > 0)
	{
		int32 Selection = FMath::RandRange(0, NumOfPickupClasses - 1);
		SpawnedPickup = GetWorld()->SpawnActor<APickup>(PickupClasses[Selection], GetActorLocation(), GetActorRotation());
		if(HasAuthority() && SpawnedPickup)
		{
			SpawnedPickup->OnDestroyed.AddDynamic(this, &APickupSpawnPoint::StartSpawnTimer);
		}
		if(ASpeedPickup* TempBuff = Cast<ASpeedPickup>(SpawnedPickup))
		{
			TempBuff->SetSpeedBuffTime(BuffTime);
		}
	}
}

void APickupSpawnPoint::StartSpawnTimer(AActor* DestroyedActor)
{
	GetWorldTimerManager().SetTimer(SpawnTimer, this, &APickupSpawnPoint::SpawnTimerFinished, TimeBetweenSpawns);
}

void APickupSpawnPoint::SpawnTimerFinished()
{
	if(HasAuthority())
	{
		SpawnPickup();
	}
}




