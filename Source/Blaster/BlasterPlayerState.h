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

	virtual void OnRep_Score() override;
	void AddToScore(float ScoreAmount);
	void AddToElims(int32 ElimAmount);
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	UFUNCTION()
	virtual void OnRep_Elims();

private:


	UPROPERTY()
	class ABlasterCharacter* BlasterCharacter;
	UPROPERTY()
	class ABlasterPlayerController* BlasterController;

	UPROPERTY(ReplicatedUsing = OnRep_Elims)
	int32 Elims;

	UPROPERTY(ReplicatedUsing = OnRep_Team)
	ETeams Team{ETeams::ET_NoTeam};

	UFUNCTION()
	void OnRep_Team();

public:
	
	FORCEINLINE ETeams GetTeam() const {return Team;}
	void SetTeam(const ETeams InTeam);
	
	
};
