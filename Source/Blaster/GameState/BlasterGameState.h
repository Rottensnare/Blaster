// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "BlasterGameState.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API ABlasterGameState : public AGameState
{
	GENERATED_BODY()


public:

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	//Updates the highest score and the playerstate with the highest score
	void UpdateTopScore(class ABlasterPlayerState* TopScoringPlayer);

	//Array holding the playerstates with the highest score.
	//By score, meaning eliminations, because personal score is currently tied only to kills
	UPROPERTY(Replicated)
	TArray< ABlasterPlayerState* > TopScoringPlayers;

	//Adds 1 to the Red Team Score. Calls FirstPlayerController->SetHUDRedTeamScore
	void RedTeamScores();
	//Adds 1 to the Blue Team Score. Calls FirstPlayerController->SetHUDBlueTeamScore
	void BlueTeamScores();

	//Array holding all the players in the Red Team
	UPROPERTY()
	TArray<ABlasterPlayerState*> RedTeamPlayers;
	//Array holding all the players in the Blue Team
	UPROPERTY()
	TArray<ABlasterPlayerState*> BlueTeamPlayers;

	UPROPERTY(ReplicatedUsing = OnRep_RedTeamScore)
	float RedTeamScore{0.f};

	UPROPERTY(ReplicatedUsing = OnRep_BlueTeamScore)
	float BlueTeamScore{0.f};

	//Calls FirstPlayerController->SetHUDRedTeamScore for clients
	UFUNCTION()
	void OnRep_RedTeamScore();
	//Calls FirstPlayerController->SetHUDBlueTeamScore for clients
	UFUNCTION()
	void OnRep_BlueTeamScore();

	
protected:




private:

	UPROPERTY()
	class ABlasterPlayerController* FirstPlayerController; //Local player controller. Needs to be changed if using a dedicated server

	float TopScore{0.f}; //Current top score
};
