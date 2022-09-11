// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blaster/GameMode/Teams.h"
#include "GameFramework/Actor.h"
#include "OrbSpawnPoint.generated.h"

UCLASS()
class BLASTER_API AOrbSpawnPoint : public AActor
{
	GENERATED_BODY()
	
public:	

	AOrbSpawnPoint();

	void SpawnOrb(const ETeams InTeam);

	

protected:

	virtual void BeginPlay() override;
	
private:

	UPROPERTY(EditAnywhere)
	TSubclassOf<class AOrb> OrbClass;

	UPROPERTY(EditAnywhere)
	class UStaticMeshComponent* SpawnPodium;

	UPROPERTY(EditAnywhere)
	USceneComponent* SpawnLocation;

	UPROPERTY(EditAnywhere)
	ETeams Team;
	
public:	

	FORCEINLINE ETeams GetTeam() const {return Team;}

};
