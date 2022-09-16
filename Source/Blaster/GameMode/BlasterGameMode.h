// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "BlasterGameMode.generated.h"

/**
 * 
 */
namespace MatchState
{
	extern BLASTER_API const FName Cooldown; //Match duration has been reached. Display winner and begin cooldown timer
}


UCLASS()
class BLASTER_API ABlasterGameMode : public AGameMode
{
	GENERATED_BODY()

public:

	ABlasterGameMode();
	//Keeps track of match time and sets match state based on passed time. Restarts game mode when cooldown time has ended.
	virtual void Tick(float DeltaSeconds) override;
	//Handles Scoring. Calls PlayerState->AddToScore, AddToElims, BlasterGameState->UpdateTopScore.
	//Calls Leader->MulticastGainedTheLead() or Loser->MultiCastLostTheLead()
	//Iterates through all player controllers and calls BroadCastElim. Fianlly calls Elim on the eliminated character
	virtual void PlayerEliminated(class ABlasterCharacter* EliminatedCharacter, class ABlasterPlayerController* VictimPlayerController, ABlasterPlayerController* AttackerController);
	//Resets and destroys eliminated character. Spawns player at a random spawn point
	virtual void RequestRespawn(ACharacter* EliminatedCharacter, AController* EliminatedController);
	//Iterates through all player controllers and calls BlasterController->OnMatchStateSet
	virtual void OnMatchStateSet() override;
	//Just returns BaseDamage
	virtual float CalculateDamage(AController* VictimPlayerController, AController* AttackerController, float BaseDamage);

	//Remove left player from top scoring player array. Call Elim for the leaving character so that the weapons are dropped.
	void PlayerLeftGame(class ABlasterPlayerState* PlayerLeaving);

	//Iterates through all the player controllers and calls ClientChatCommitted with the passed in message
	void SendChatMsg(const FText& Text, const FString& PlayerName);

	UPROPERTY(EditDefaultsOnly)
	float WarmupTime{10.f};

	UPROPERTY(EditDefaultsOnly)
	float MatchTime{120.f};

	UPROPERTY(EditDefaultsOnly)
	float CooldownTime{15.f};

	float LevelStartingTime{0.f};

protected:

	//Gets server time
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere)
	bool bTeamsMatch{false};
	
	bool bMatchEnding{false};

private:
	
	UPROPERTY(VisibleAnywhere)
	float CountdownTime{0.f};

	UPROPERTY(VisibleAnywhere)
	bool bRestartingGame{false};

	UPROPERTY(VisibleAnywhere)
	float ServerTotalTime{0.f}; //EXPERIMENTAL FOR BUG HUNTING

public:

	FORCEINLINE float GetCountdownTime() const {return CountdownTime;}
	
};
