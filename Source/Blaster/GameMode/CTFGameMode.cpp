// Fill out your copyright notice in the Description page of Project Settings.


#include "CTFGameMode.h"

#include "Blaster/GameState/BlasterGameState.h"
#include "Blaster/Items/Orb.h"
#include "Blaster/Items/OrbZone.h"

void ACTFGameMode::PlayerEliminated(ABlasterCharacter* EliminatedCharacter,
                                    ABlasterPlayerController* VictimPlayerController, ABlasterPlayerController* AttackerController)
{
	ABlasterGameMode::PlayerEliminated(EliminatedCharacter, VictimPlayerController, AttackerController);

	
}

void ACTFGameMode::FlagCaptured(AOrb* InOrb, AOrbZone* InOrbZone)
{
	bool bValidCapture = InOrb->GetTeam() != InOrbZone->GetTeam();
	if(!bValidCapture) return;
	UE_LOG(LogTemp, Warning, TEXT("ACTFGameMode::FlagCaptured: IF bValidCapture"))
	ABlasterGameState* BGameState = Cast<ABlasterGameState>(GameState);
	if(BGameState)
	{
		if(InOrbZone->GetTeam() == ETeams::ET_BlueTeam)
		{
			BGameState->BlueTeamScores();
			InOrb->Dropped(InOrb->GetActorLocation());
		}
		else if(InOrbZone->GetTeam() == ETeams::ET_RedTeam)
		{
			BGameState->RedTeamScores();
			InOrb->Dropped(InOrb->GetActorLocation());
		}
	}
}
