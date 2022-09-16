// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BlasterGameMode.h"
#include "TeamsGameMode.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API ATeamsGameMode : public ABlasterGameMode
{
	GENERATED_BODY()

public:

	ATeamsGameMode();
	//Called when player has joined/logged in. Assigns player to either team depending how many are in each one.
	virtual void PostLogin(APlayerController* NewPlayer) override;
	//Removes player from the team they were in when logging out/leaving the game
	virtual void Logout(AController* Exiting) override;
	//Checks if player is friendly and returns damage value based on that. I.E. no friendly fire allowed
	virtual float CalculateDamage(AController* VictimPlayerController, AController* AttackerController, float BaseDamage) override;
	//Adds score to team based on which player was eliminated
	virtual void PlayerEliminated(ABlasterCharacter* EliminatedCharacter, ABlasterPlayerController* VictimPlayerController, ABlasterPlayerController* AttackerController) override;

protected:
	//Assigns players to teams when the match starts
	virtual void HandleMatchHasStarted() override;
	
};
