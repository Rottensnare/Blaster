// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponSpawnPoint.generated.h"

//TODO: GameMode needs to destroy dropped weapons that are unused for a certain amount of time
//TODO: Create a drop table with probability for each weapon
//TODO: Create a loot pinata that spawns a weapon or pickup based on previously mentioned loot chance

UCLASS()
class BLASTER_API AWeaponSpawnPoint : public AActor 
{
	GENERATED_BODY()
	
public:	
	
	AWeaponSpawnPoint();
	virtual void Tick(float DeltaTime) override;
	UFUNCTION()
	void StartSpawnTimer(); 

protected:

	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere)
	TArray<TSubclassOf<class AWeapon>> WeaponClasses;

	UPROPERTY(VisibleAnywhere)
	AWeapon* SpawnedWeapon;

	void SpawnWeapon();
	
	UFUNCTION()
	void SpawnTimerFinished();
private:

	FTimerHandle SpawnTimer;
	UPROPERTY(EditAnywhere)
	float TimeBetweenSpawns{15.f};

	UPROPERTY(EditAnywhere)
	class UStaticMeshComponent* SpawnPodium;

	UPROPERTY(EditAnywhere)
	USceneComponent* SpawnLocation;
};
