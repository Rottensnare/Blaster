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
	void StartSpawnTimer(); //Starts SpawnTimer and calls SpawnTimerFinished in the end

protected:

	virtual void BeginPlay() override; //Calls SpawnWeapon if host

	UPROPERTY(EditAnywhere)
	TArray<TSubclassOf<class AWeapon>> WeaponClasses; //Selectable from instance or blueprint

	UPROPERTY(VisibleAnywhere)
	AWeapon* SpawnedWeapon;

	void SpawnWeapon();//Spawns random weapon from WeaponClasses, handles initialization
	
	UFUNCTION()
	void SpawnTimerFinished(); //Calls SpawnWeapon
private:

	FTimerHandle SpawnTimer;
	UPROPERTY(EditAnywhere)
	float TimeBetweenSpawns{15.f};

	UPROPERTY(EditAnywhere)
	class UStaticMeshComponent* SpawnPodium;

	UPROPERTY(EditAnywhere)
	USceneComponent* SpawnLocation;
};
