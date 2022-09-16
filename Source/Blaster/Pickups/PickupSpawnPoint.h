// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PickupSpawnPoint.generated.h"

UCLASS()
class BLASTER_API APickupSpawnPoint : public AActor
{
	GENERATED_BODY()
	
public:	

	APickupSpawnPoint();
	virtual void Tick(float DeltaTime) override;

protected:

	virtual void BeginPlay() override; //Spawns a pickup if server

	UPROPERTY(EditAnywhere)
	TArray<TSubclassOf<class APickup>> PickupClasses; //Can be set in instances or blueprints

	UPROPERTY(VisibleAnywhere)
	APickup* SpawnedPickup; //Currently spawned pickup

	//Chooses random pickup class from PickupClasses, spawns it, binds StartSpawnTimer to SpawnedPickup->OnDestroyed
	void SpawnPickup();
	//Starts SpawnTimer
	UFUNCTION()
	void StartSpawnTimer(AActor* DestroyedActor);
	//Calls SpawnPickup if server
	void SpawnTimerFinished();
private:

	FTimerHandle SpawnTimer;
	UPROPERTY(EditAnywhere)
	float TimeBetweenSpawns{15.f};
	
	

	

};
