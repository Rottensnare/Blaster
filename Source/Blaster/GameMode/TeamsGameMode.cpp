// Fill out your copyright notice in the Description page of Project Settings.


#include "TeamsGameMode.h"

#include "Blaster/BlasterPlayerController.h"
#include "Blaster/BlasterPlayerState.h"
#include "Blaster/GameState/BlasterGameState.h"
#include "Kismet/GameplayStatics.h"

ATeamsGameMode::ATeamsGameMode()
{
	bTeamsMatch = true;
}

void ATeamsGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	ABlasterGameState* BlasterGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));
	if(BlasterGameState)
	{
		ABlasterPlayerState* BPState = NewPlayer->GetPlayerState<ABlasterPlayerState>();
		if(BPState && BPState->GetTeam() == ETeams::ET_NoTeam)
		{
			if(BlasterGameState->BlueTeamPlayers.Num() >= BlasterGameState->RedTeamPlayers.Num())
			{
				BlasterGameState->RedTeamPlayers.AddUnique(BPState);
				BPState->SetTeam(ETeams::ET_RedTeam);
			}
			else
			{
				BlasterGameState->BlueTeamPlayers.AddUnique(BPState);
				BPState->SetTeam(ETeams::ET_BlueTeam);
			}
		}
	}
}

void ATeamsGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	ABlasterGameState* BlasterGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));
	ABlasterPlayerState* BPState = Exiting->GetPlayerState<ABlasterPlayerState>();
	if(BlasterGameState && BPState)
	{
		if(BlasterGameState->RedTeamPlayers.Contains(BPState))
		{
			BlasterGameState->RedTeamPlayers.Remove(BPState);
		}
		if(BlasterGameState->BlueTeamPlayers.Contains(BPState))
		{
			BlasterGameState->BlueTeamPlayers.Remove(BPState);
		}
	}
}

float ATeamsGameMode::CalculateDamage(AController* VictimPlayerController, AController* AttackerController,
	float BaseDamage)
{
	if(VictimPlayerController == nullptr || AttackerController == nullptr) return 0.f;
	ABlasterPlayerState* VictimPlayerState = VictimPlayerController->GetPlayerState<ABlasterPlayerState>();
	ABlasterPlayerState* AttackerPlayerState = AttackerController->GetPlayerState<ABlasterPlayerState>();
	if(VictimPlayerState == nullptr || AttackerPlayerState == nullptr) return 0.f;
	
	if(VictimPlayerState->GetTeam() != AttackerPlayerState->GetTeam())
	{
		return BaseDamage;
	}
	else
	{
		return 0.f;
	}
}

void ATeamsGameMode::PlayerEliminated(ABlasterCharacter* EliminatedCharacter,
	ABlasterPlayerController* VictimPlayerController, ABlasterPlayerController* AttackerController)
{
	Super::PlayerEliminated(EliminatedCharacter, VictimPlayerController, AttackerController);

	ABlasterGameState* const BlasterGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));
	ABlasterPlayerState* const AttackerPlayerState = AttackerController ? Cast<ABlasterPlayerState>(AttackerController->PlayerState) : nullptr;
	if(BlasterGameState && AttackerPlayerState)
	{
		if(AttackerPlayerState->GetTeam() == ETeams::ET_BlueTeam)
		{
			BlasterGameState->BlueTeamScores();
		}
		else if(AttackerPlayerState->GetTeam() == ETeams::ET_RedTeam)
		{
			BlasterGameState->RedTeamScores();
		}
	}
}

void ATeamsGameMode::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();

	ABlasterGameState* BlasterGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));
	if(BlasterGameState)
	{
		for(auto PState : BlasterGameState->PlayerArray)
		{
			ABlasterPlayerState* BPState = Cast<ABlasterPlayerState>(PState.Get());
			if(BPState && BPState->GetTeam() == ETeams::ET_NoTeam)
			{
				if(BlasterGameState->BlueTeamPlayers.Num() >= BlasterGameState->RedTeamPlayers.Num())
				{
					BlasterGameState->RedTeamPlayers.AddUnique(BPState);
					BPState->SetTeam(ETeams::ET_RedTeam);
				}
				else
				{
					BlasterGameState->BlueTeamPlayers.AddUnique(BPState);
					BPState->SetTeam(ETeams::ET_BlueTeam);
				}
			}
		}
	}
}
