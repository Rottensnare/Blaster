// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterGameState.h"

#include "Blaster/BlasterPlayerController.h"
#include "Blaster/BlasterPlayerState.h"
#include "Net/UnrealNetwork.h"



void ABlasterGameState::UpdateTopScore(ABlasterPlayerState* TopScoringPlayer)
{
	if(TopScoringPlayers.Num() == 0)
	{
		TopScoringPlayers.Add(TopScoringPlayer);
		TopScore = TopScoringPlayer->GetScore();
	}else if(TopScoringPlayer->GetScore() == TopScore)
	{
		TopScoringPlayers.AddUnique(TopScoringPlayer);
	}else if(TopScoringPlayer->GetScore() > TopScore)
	{
		TopScoringPlayers.Empty();
		TopScoringPlayers.AddUnique(TopScoringPlayer);
		TopScore = TopScoringPlayer->GetScore();
	}
}

void ABlasterGameState::RedTeamScores()
{
	++RedTeamScore;

	FirstPlayerController = FirstPlayerController == nullptr ? Cast<ABlasterPlayerController>(GetWorld()->GetFirstPlayerController()) : FirstPlayerController;
	if(FirstPlayerController)
	{
		FirstPlayerController->SetHUDRedTeamScore(RedTeamScore);
	}
}

void ABlasterGameState::BlueTeamScores()
{
	++BlueTeamScore;

	FirstPlayerController = FirstPlayerController == nullptr ? Cast<ABlasterPlayerController>(GetWorld()->GetFirstPlayerController()) : FirstPlayerController;
	if(FirstPlayerController)
	{
		FirstPlayerController->SetHUDBlueTeamScore(BlueTeamScore);
	}
}

void ABlasterGameState::OnRep_RedTeamScore()
{
	FirstPlayerController = FirstPlayerController == nullptr ? Cast<ABlasterPlayerController>(GetWorld()->GetFirstPlayerController()) : FirstPlayerController;
	if(FirstPlayerController)
	{
		FirstPlayerController->SetHUDRedTeamScore(RedTeamScore);
	}
}

void ABlasterGameState::OnRep_BlueTeamScore()
{
	FirstPlayerController = FirstPlayerController == nullptr ? Cast<ABlasterPlayerController>(GetWorld()->GetFirstPlayerController()) : FirstPlayerController;
	if(FirstPlayerController)
	{
		FirstPlayerController->SetHUDBlueTeamScore(BlueTeamScore);
	}
}

void ABlasterGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABlasterGameState, TopScoringPlayers);
	DOREPLIFETIME(ABlasterGameState, RedTeamScore);
	DOREPLIFETIME(ABlasterGameState, BlueTeamScore);
}
