// Fill out your copyright notice in the Description page of Project Settings.


#include "OrbSpawnPoint.h"

#include "Orb.h"
#include "Blaster/GameMode/CTFGameMode.h"
#include "Kismet/GameplayStatics.h"


AOrbSpawnPoint::AOrbSpawnPoint()
{

	PrimaryActorTick.bCanEverTick = false;
	bReplicates = false;

	SpawnPodium = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SpawnPodium"));
	SetRootComponent(SpawnPodium);

	SpawnLocation = CreateDefaultSubobject<USceneComponent>(TEXT("SpawnLocation"));
	SpawnLocation->SetupAttachment(RootComponent);
}

void AOrbSpawnPoint::SpawnOrb(const ETeams InTeam)
{
	if(!HasAuthority() || OrbClass == nullptr) return; //Game mode calls this so this is most likely unnecessary.
	FActorSpawnParameters SpawnParameters;
	AOrb* SpawnedOrb = GetWorld()->SpawnActor<AOrb>(OrbClass, SpawnLocation->GetComponentLocation(), SpawnLocation->GetComponentRotation());
	if(SpawnedOrb)
	{
		SpawnedOrb->SetTeam(InTeam);
		SpawnedOrb->SetMaterial();
		ACTFGameMode* CTFGameMode = Cast<ACTFGameMode>(UGameplayStatics::GetGameMode(this));
		if(CTFGameMode)
		{
			if(InTeam == ETeams::ET_RedTeam) CTFGameMode->SetRedOrb(SpawnedOrb);
			else if(InTeam == ETeams::ET_BlueTeam) CTFGameMode->SetBlueOrb(SpawnedOrb);
		}
	}
}


void AOrbSpawnPoint::BeginPlay()
{
	Super::BeginPlay();
	
}


