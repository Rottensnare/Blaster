// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterGameState.h"

#include "Blaster/BlasterPlayerState.h"
#include "Net/UnrealNetwork.h"

void ABlasterGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABlasterGameState, TopScoringPlayers);
}

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
