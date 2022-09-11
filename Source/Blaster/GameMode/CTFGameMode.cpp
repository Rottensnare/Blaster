// Fill out your copyright notice in the Description page of Project Settings.


#include "CTFGameMode.h"

#include "Blaster/GameState/BlasterGameState.h"
#include "Blaster/Items/Orb.h"
#include "Blaster/Items/OrbSpawnPoint.h"
#include "Blaster/Items/OrbZone.h"
#include "Kismet/GameplayStatics.h"




void ACTFGameMode::BeginPlay()
{
	Super::BeginPlay();

	HandleCTFStart();

	
}

void ACTFGameMode::HandleCTFStart()
{
	TArray<AActor*> OrbSpawnPoints;
	UGameplayStatics::GetAllActorsOfClass(this, AOrbSpawnPoint::StaticClass(), OrbSpawnPoints);
	for(auto TempActor : OrbSpawnPoints)
	{
		AOrbSpawnPoint* OrbSpawnPoint = Cast<AOrbSpawnPoint>(TempActor);
		if(OrbSpawnPoint)
		{
			if(OrbSpawnPoint->GetTeam() == ETeams::ET_RedTeam)
			{
				RedOrbSpawnPoint = OrbSpawnPoint;
				if(RedOrbSpawnPoint)
				{
					RedOrbSpawnPoint->SpawnOrb(ETeams::ET_RedTeam);
				}
			}
			else if(OrbSpawnPoint->GetTeam() == ETeams::ET_BlueTeam)
			{
				BlueOrbSpawnPoint = OrbSpawnPoint;
				if(BlueOrbSpawnPoint)
				{
					BlueOrbSpawnPoint->SpawnOrb(ETeams::ET_BlueTeam);
				}
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("ACTFGameMode::BeginPlay: Someone forgot to assign a team to the orb spawn point"))
			}
		}
	}
}

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


