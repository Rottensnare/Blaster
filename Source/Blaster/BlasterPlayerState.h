// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "GameMode/Teams.h"
#include "BlasterPlayerState.generated.h"


/**
 * 
 */
UCLASS()
class BLASTER_API ABlasterPlayerState : public APlayerState
{
	GENERATED_BODY()
public:

	virtual void OnRep_Score() override; //Calls BlasterController->SetHUDScore
	void AddToScore(float ScoreAmount); //Adds ScoreAmount to the current score and calls BlasterController->SetHUDScore
	void AddToElims(int32 ElimAmount); //Adds ElimAmount to the current Elims and calls BlasterController->SetHUDElims
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	UFUNCTION()
	virtual void OnRep_Elims(); //Calls BlasterController->SetHUDElims

private:


	UPROPERTY()
	class ABlasterCharacter* BlasterCharacter;
	UPROPERTY()
	class ABlasterPlayerController* BlasterController;

	UPROPERTY(ReplicatedUsing = OnRep_Elims)
	int32 Elims; //How many eliminations the character has

	UPROPERTY(ReplicatedUsing = OnRep_Team)
	ETeams Team{ETeams::ET_NoTeam}; //Which team does this character belong to

	UFUNCTION()
	void OnRep_Team(); //Calls BlasterCharacter->SetTeamColor

public:
	
	FORCEINLINE ETeams GetTeam() const {return Team;}
	void SetTeam(const ETeams InTeam);
	
	
};
