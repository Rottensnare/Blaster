// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HitScanWeapon.h"
#include "Shotgun.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API AShotgun : public AHitScanWeapon
{
	GENERATED_BODY()


public:
	//Handles firing the shotgun. More info in the .cpp file
	virtual void FireShotgun(const TArray<FVector_NetQuantize>& HitTargets);
	//Calculates scatter for each pellet. Called from CombatComponent
	void ShotgunTraceEndWithScatter(const FVector& HitTarget, TArray<FVector_NetQuantize>& OutHitTargets);

private:

	UPROPERTY(EditAnywhere)
	uint8 NumberOfPellets{12};
};
