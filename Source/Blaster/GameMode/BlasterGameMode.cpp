// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterGameMode.h"

#include "Blaster/BlasterPlayerController.h"
#include "Blaster/BlasterPlayerState.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/GameState/BlasterGameState.h"
#include "Blaster/HUD/BlasterHUD.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"

namespace MatchState
{
	const FName Cooldown = FName("Cooldown");
}


ABlasterGameMode::ABlasterGameMode()
{
	bDelayedStart = true;
}

void ABlasterGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if(MatchState == MatchState::WaitingToStart)
	{
		CountdownTime = WarmupTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if(CountdownTime <= 0.f)
		{
			bMatchEnding = false;
			StartMatch();
		}
		
	}
	else if(MatchState == MatchState::InProgress)
	{
		CountdownTime = WarmupTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if(CountdownTime <= 0.f)
		{
			SetMatchState(MatchState::Cooldown);
			
		}
	}
	else if(MatchState == MatchState::Cooldown)
	{
		CountdownTime = CooldownTime + WarmupTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if(!bMatchEnding)
		{
			for(FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
			{
				ABlasterPlayerController* TempBlasterPlayerController = Cast<ABlasterPlayerController>(*It);
				if(TempBlasterPlayerController)
				{
					ABlasterHUD* TempHUD = Cast<ABlasterHUD>(TempBlasterPlayerController->GetHUD());
					if(TempHUD)
					{
						TempHUD->bDrawHUD = false;
					}
				}
			}
			bMatchEnding = true;
		}
		if(CountdownTime <= 0.f && !bRestartingGame)
		{
			UWidgetLayoutLibrary::RemoveAllWidgets(this);
#if WITH_EDITOR
			bUseSeamlessTravel = false;
			RestartGame();
			bRestartingGame = true;
			UE_LOG(LogTemp, Warning, TEXT("With Editor"))
#else 
			UE_LOG(LogTemp, Warning, TEXT("Not with Editor"))
			
			LevelStartingTime = 0.f;
			bUseSeamlessTravel = true;
			bRestartingGame = true;
			RestartGame();
			/*
			UWorld* World = GetWorld();
			if(World)
			{
				bUseSeamlessTravel = true;
				World->ServerTravel("/Game/ThirdPerson/Maps/Level_FFA_01?listen");
			}
			UGameplayStatics::OpenLevel(this, FName("/Game/Maps/Level_OpenWorld_01?listen"));
			*/
#endif
		}
	}
}

void ABlasterGameMode::PlayerEliminated(ABlasterCharacter* EliminatedCharacter,
                                        ABlasterPlayerController* VictimPlayerController, ABlasterPlayerController* AttackerController)
{
	if(AttackerController == nullptr || AttackerController->PlayerState == nullptr) return;
	if(VictimPlayerController == nullptr || VictimPlayerController->PlayerState == nullptr) return;
	ABlasterPlayerState* AttackerPlayerState = AttackerController ? Cast<ABlasterPlayerState>(AttackerController->PlayerState) : nullptr;
	ABlasterPlayerState* VictimPlayerState = VictimPlayerController ? Cast<ABlasterPlayerState>(VictimPlayerController->PlayerState) : nullptr;
	ABlasterGameState* BlasterGameState = GetGameState<ABlasterGameState>();
	
	if(AttackerPlayerState && AttackerPlayerState != VictimPlayerState)
	{
		TArray<ABlasterPlayerState*> PlayersInTheLead;
		for(auto LeadPlayer : BlasterGameState->TopScoringPlayers)
		{
			PlayersInTheLead.Add(LeadPlayer);
		}
		
		AttackerPlayerState->AddToScore(50.f);
		AttackerPlayerState->AddToElims(1);
		BlasterGameState->UpdateTopScore(AttackerPlayerState);

		if(BlasterGameState->TopScoringPlayers.Contains(AttackerPlayerState))
		{
			ABlasterCharacter* Leader = Cast<ABlasterCharacter>(AttackerPlayerState->GetPawn());
			if(Leader)
			{
				Leader->MulticastGainedTheLead();
			}
		}

		for(int i = 0; i < PlayersInTheLead.Num(); i++)
		{
			if(!BlasterGameState->TopScoringPlayers.Contains(PlayersInTheLead[i]))
			{
				ABlasterCharacter* Loser = Cast<ABlasterCharacter>(PlayersInTheLead[i]->GetPawn());
				if(Loser)
				{
					Loser->MultiCastLostTheLead();
				}
			}
		}
	}
	
	for(FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		ABlasterPlayerController* TempBlasterPlayerController = Cast<ABlasterPlayerController>(*It);
		if(TempBlasterPlayerController && AttackerPlayerState && VictimPlayerState)
		{
			TempBlasterPlayerController->BroadCastElim(AttackerPlayerState, VictimPlayerState);
			
		}
	}
	
	if(EliminatedCharacter == nullptr) return;

	EliminatedCharacter->Elim(false);
	
}

void ABlasterGameMode::SendChatMsg(const FText& Text, const FString& PlayerName)
{
	for(FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		ABlasterPlayerController* TempBlasterPlayerController = Cast<ABlasterPlayerController>(*It);
		if(TempBlasterPlayerController)
		{
			TempBlasterPlayerController->ClientChatCommitted(Text, PlayerName);
		}
	}
}


void ABlasterGameMode::RequestRespawn(ACharacter* EliminatedCharacter, AController* EliminatedController)
{
	if(EliminatedCharacter)
	{
		EliminatedCharacter->Reset();
		EliminatedCharacter->Destroy();
	}
	if(EliminatedController)
	{
		TArray<AActor*> PlayerStarts;
		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStarts);
		int32 Selection = FMath::RandRange(0, PlayerStarts.Num()-1);
		RestartPlayerAtPlayerStart(EliminatedController, PlayerStarts[Selection]);
	}
}

void ABlasterGameMode::OnMatchStateSet()
{
	Super::OnMatchStateSet();

	for(FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		ABlasterPlayerController* BlasterController = Cast<ABlasterPlayerController>(*It);
		if(BlasterController)
		{
			BlasterController->OnMatchStateSet(MatchState, bTeamsMatch);
		}
	}
}

float ABlasterGameMode::CalculateDamage(AController* VictimPlayerController, AController* AttackerController,
	float BaseDamage)
{
	return BaseDamage;
}

void ABlasterGameMode::PlayerLeftGame(ABlasterPlayerState* PlayerLeaving)
{
	if(PlayerLeaving == nullptr) return;
	ABlasterGameState* BlasterGameState = GetGameState<ABlasterGameState>();
	if(BlasterGameState && BlasterGameState->TopScoringPlayers.Contains(PlayerLeaving))
	{
		BlasterGameState->TopScoringPlayers.Remove(PlayerLeaving);
	}
	
	ABlasterCharacter* CharacterLeaving = Cast<ABlasterCharacter>(PlayerLeaving->GetPawn());
	if(CharacterLeaving)
	{
		CharacterLeaving->Elim(true);
	}
}

void ABlasterGameMode::BeginPlay()
{
	Super::BeginPlay();

	LevelStartingTime = 0.f;
}
