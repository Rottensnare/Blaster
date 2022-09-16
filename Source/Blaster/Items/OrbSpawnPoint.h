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

	//Spawns orb and sets its initial values. Calls CTFGameMod->SetRedOrb or SetBlueOrb
	void SpawnOrb(const ETeams InTeam);

	

protected:

	virtual void BeginPlay() override; //Nada
	
private:

	UPROPERTY(EditAnywhere)
	TSubclassOf<class AOrb> OrbClass;

	UPROPERTY(EditAnywhere)
	class UStaticMeshComponent* SpawnPodium;

	UPROPERTY(EditAnywhere)
	USceneComponent* SpawnLocation;

	UPROPERTY(EditAnywhere)
	ETeams Team; //Which team does this spawner belong to

	UPROPERTY(VisibleAnywhere)
	class UNiagaraComponent* OrbSpawnParticleComponent;

	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* OrbSpawnParticleSystem;
	
public:	

	FORCEINLINE ETeams GetTeam() const {return Team;}

};
