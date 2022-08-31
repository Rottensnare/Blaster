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

	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere)
	TArray<TSubclassOf<class APickup>> PickupClasses;

	UPROPERTY(VisibleAnywhere)
	APickup* SpawnedPickup;

	void SpawnPickup();
	UFUNCTION()
	void StartSpawnTimer(AActor* DestroyedActor);
	void SpawnTimerFinished();
private:

	FTimerHandle SpawnTimer;
	UPROPERTY(EditAnywhere)
	float TimeBetweenSpawns{15.f};
	
	

	

};
