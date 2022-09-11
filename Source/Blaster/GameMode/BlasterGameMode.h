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
	virtual void Tick(float DeltaSeconds) override;
	virtual void PlayerEliminated(class ABlasterCharacter* EliminatedCharacter, class ABlasterPlayerController* VictimPlayerController, ABlasterPlayerController* AttackerController);
	virtual void RequestRespawn(ACharacter* EliminatedCharacter, AController* EliminatedController);
	virtual void OnMatchStateSet() override;
	virtual float CalculateDamage(AController* VictimPlayerController, AController* AttackerController, float BaseDamage);

	void PlayerLeftGame(class ABlasterPlayerState* PlayerLeaving);
	
	void SendChatMsg(const FText& Text, const FString& PlayerName);

	UPROPERTY(EditDefaultsOnly)
	float WarmupTime{10.f};

	UPROPERTY(EditDefaultsOnly)
	float MatchTime{120.f};

	UPROPERTY(EditDefaultsOnly)
	float CooldownTime{15.f};

	float LevelStartingTime{0.f};

protected:

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
	float ServerTotalTime{0.f};

public:

	FORCEINLINE float GetCountdownTime() const {return CountdownTime;}
	
};
