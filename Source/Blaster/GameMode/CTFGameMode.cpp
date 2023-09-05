// Fill out your copyright notice in the Description page of Project Settings.


#include "CTFGameMode.h"

#include "Blaster/BlasterPlayerController.h"
#include "Blaster/Character/BlasterCharacter.h"
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
	for(AActor* const TempActor : OrbSpawnPoints)
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
					
					check(RedOrb)
					RedOrb->OnOrbPickedUp.AddUObject(this, &ACTFGameMode::FlagPickedUp);
				}
			}
			else if(OrbSpawnPoint->GetTeam() == ETeams::ET_BlueTeam)
			{
				BlueOrbSpawnPoint = OrbSpawnPoint;
				if(BlueOrbSpawnPoint)
				{
					BlueOrbSpawnPoint->SpawnOrb(ETeams::ET_BlueTeam);

					check(BlueOrb)
					BlueOrb->OnOrbPickedUp.AddUObject(this, &ACTFGameMode::FlagPickedUp);
				}
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("%hs: Someone forgot to assign a team to the orb spawn point"), __FUNCTION__)
			}
		}
	}
}

void ACTFGameMode::FlagPickedUp(AOrb* PickedUpOrb) const //Note: Can be done with 1 for loop
{
	if(PickedUpOrb && PickedUpOrb->GetOwningBlasterCharacter())
	{
		if(PickedUpOrb->GetTeam() == ETeams::ET_BlueTeam)
		{
			for(FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
			{
				ABlasterPlayerController* TempBlasterPlayerController = Cast<ABlasterPlayerController>(*It);
				if(TempBlasterPlayerController)
				{
					TempBlasterPlayerController->ClientOrbAnnouncement(BlueOrb->GetOwningBlasterCharacter()->GetPlayerState(), 0);
				}
			}
		}
		if(PickedUpOrb->GetTeam() == ETeams::ET_RedTeam)
		{
			for(FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
			{
				ABlasterPlayerController* TempBlasterPlayerController = Cast<ABlasterPlayerController>(*It);
				if(TempBlasterPlayerController)
				{
					TempBlasterPlayerController->ClientOrbAnnouncement(RedOrb->GetOwningBlasterCharacter()->GetPlayerState(), 0);
				}
			}
		}
	}
}

void ACTFGameMode::PlayerEliminated(ABlasterCharacter* EliminatedCharacter,
                                    ABlasterPlayerController* VictimPlayerController, ABlasterPlayerController* AttackerController)
{
	ABlasterGameMode::PlayerEliminated(EliminatedCharacter, VictimPlayerController, AttackerController);

	if(EliminatedCharacter && EliminatedCharacter->GetHeldOrb() != nullptr)
	{
		EliminatedCharacter->GetHeldOrb()->Dropped(EliminatedCharacter->GetHeldOrb()->GetActorLocation());
		EliminatedCharacter->MulticastDropTheOrb();
	}
}

void ACTFGameMode::FlagCaptured(AOrb* InOrb, AOrbZone* InOrbZone)
{
	const bool bValidCapture = InOrb->GetTeam() != InOrbZone->GetTeam();
	if(!bValidCapture) return;
	
	ABlasterGameState* BGameState = Cast<ABlasterGameState>(GameState);
	if(BGameState)
	{
		if(InOrbZone->GetTeam() == ETeams::ET_BlueTeam)
		{
			BGameState->BlueTeamScores();
			if(RedOrb && RedOrb->GetOwningBlasterCharacter())
			{
				for(FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
				{
					ABlasterPlayerController* TempBlasterPlayerController = Cast<ABlasterPlayerController>(*It);
					if(TempBlasterPlayerController)
					{
						TempBlasterPlayerController->ClientOrbAnnouncement(RedOrb->GetOwningBlasterCharacter()->GetPlayerState(), 1);
					}
				}
				RedOrb->GetOwningBlasterCharacter()->MulticastDropTheOrb();
				RedOrb->Dropped(RedOrb->GetActorLocation());
				RedOrb->Destroy();
				RedOrbSpawnPoint->SpawnOrb(ETeams::ET_RedTeam);
				
				check(RedOrb)
				RedOrb->OnOrbPickedUp.AddUObject(this, &ACTFGameMode::FlagPickedUp);
			}
		}
		else if(InOrbZone->GetTeam() == ETeams::ET_RedTeam)
		{
			BGameState->RedTeamScores();
			if(BlueOrb && BlueOrb->GetOwningBlasterCharacter())
			{
				for(FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
				{
					ABlasterPlayerController* TempBlasterPlayerController = Cast<ABlasterPlayerController>(*It);
					if(TempBlasterPlayerController)
					{
						TempBlasterPlayerController->ClientOrbAnnouncement(BlueOrb->GetOwningBlasterCharacter()->GetPlayerState(), 1);
					}
				}
				BlueOrb->GetOwningBlasterCharacter()->MulticastDropTheOrb();
				BlueOrb->Dropped(BlueOrb->GetActorLocation());
				BlueOrb->Destroy();
				BlueOrbSpawnPoint->SpawnOrb(ETeams::ET_BlueTeam);
				
				check(BlueOrb)
				BlueOrb->OnOrbPickedUp.AddUObject(this, &ACTFGameMode::FlagPickedUp);
			}
		}
	}
}


