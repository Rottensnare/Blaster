// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "LobbyGameMode.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API ALobbyGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	//If enough players have joined, start the match with the correct match type.
	//Gets the desired match type from the MultiplayerSessionsSubsystem which in turn was set from the start menu
	virtual void PostLogin(APlayerController* NewPlayer) override;
	
protected:


private:
	
};
