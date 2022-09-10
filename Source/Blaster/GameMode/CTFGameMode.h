// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TeamsGameMode.h"
#include "CTFGameMode.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API ACTFGameMode : public ATeamsGameMode
{
	GENERATED_BODY()

public:

	virtual void PlayerEliminated(ABlasterCharacter* EliminatedCharacter, ABlasterPlayerController* VictimPlayerController, ABlasterPlayerController* AttackerController) override;
	void FlagCaptured(class AOrb* InOrb, class AOrbZone* InOrbZone);
	
};
