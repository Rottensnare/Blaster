// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyGameMode.h"

#include "MultiplayerSessionsSubsystem.h"
#include "GameFramework/GameStateBase.h"

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	 int32 NumOfPlayers = GameState.Get()->PlayerArray.Num();
	UGameInstance* GameInstance = GetGameInstance();
	if(GameInstance)
	{
		UMultiplayerSessionsSubsystem* Subsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
		checkf(Subsystem, TEXT("Subsystem was NULL"));
		if(NumOfPlayers >= Subsystem->DesiredNumPublicConnections)
		{
			UWorld* World = GetWorld();
			if(World)
			{
				bUseSeamlessTravel = true;

				FString MatchType = Subsystem->DesiredMatchType;
				if(MatchType == "FreeForAll")
				{
					World->ServerTravel(FString("/Game/Maps/Level_FFA_01?listen"));
				}
				else if (MatchType == "TDM")
				{
					World->ServerTravel(FString("/Game/Maps/Level_FFA_01?listen"));
				}
				else if(MatchType == "CTF")
				{
					World->ServerTravel(FString("/Game/Maps/Level_FFA_01?listen"));
				}
				
				
			}
		}
	}
	
}
