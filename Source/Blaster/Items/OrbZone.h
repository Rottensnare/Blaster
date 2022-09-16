// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Blaster/GameMode/Teams.h"
#include "OrbZone.generated.h"

//Class for the CTF capture area. Score when bringing the enemy orb to your team's OrbZone
UCLASS()
class BLASTER_API AOrbZone : public AActor
{
	GENERATED_BODY()
	
public:	

	AOrbZone();

	//Checks if the overlapping actor has the orb and is of the correct team. Calls CTFGameMode->FlagCaptured
	UFUNCTION()
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);

protected:

	//Binds OnSphereOverlap to OrbZoneComponent->OnComponentBeginOverlap if server
	virtual void BeginPlay() override;


private:

	//Scoring area collision box
	UPROPERTY(EditAnywhere)
	class UBoxComponent* OrbZoneComponent;

	UPROPERTY(EditAnywhere)
	ETeams Team; //Which team does this zone belong to

	UPROPERTY()
	class ACTFGameMode* CTFGameMode; //Reference to the CTF game mode

	//Particle system to show the capture area. Color different depending on the team
	UPROPERTY(VisibleAnywhere)
	class UNiagaraComponent* OrbZoneParticleComponent; 

	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* OrbZoneParticleSystem;


public:

	FORCEINLINE ETeams GetTeam() const {return Team;}

};
