// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Blaster/GameMode/Teams.h"
#include "OrbZone.generated.h"

UCLASS()
class BLASTER_API AOrbZone : public AActor
{
	GENERATED_BODY()
	
public:	

	AOrbZone();

	UFUNCTION()
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);

protected:

	virtual void BeginPlay() override;


private:


	UPROPERTY(EditAnywhere)
	class UBoxComponent* OrbZoneComponent;

	UPROPERTY(EditAnywhere)
	ETeams Team;

	UPROPERTY()
	class ACTFGameMode* CTFGameMode;

public:

	FORCEINLINE ETeams GetTeam() const {return Team;}

};
