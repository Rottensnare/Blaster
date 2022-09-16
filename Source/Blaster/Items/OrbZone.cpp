// Fill out your copyright notice in the Description page of Project Settings.


#include "OrbZone.h"

#include "NiagaraComponent.h"
#include "Orb.h"
#include "Blaster/GameMode/CTFGameMode.h"
#include "Components/BoxComponent.h"


AOrbZone::AOrbZone()
{

	PrimaryActorTick.bCanEverTick = false;

	OrbZoneComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("OrbZoneComponent"));
	SetRootComponent(OrbZoneComponent);

	OrbZoneParticleComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("PickupEffectComponent"));
	OrbZoneParticleComponent->SetupAttachment(RootComponent);
	
}

void AOrbZone::BeginPlay()
{
	Super::BeginPlay();

	if(HasAuthority()) OrbZoneComponent->OnComponentBeginOverlap.AddDynamic(this, &AOrbZone::OnSphereOverlap);
	
	
}

void AOrbZone::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	//UE_LOG(LogTemp, Warning, TEXT("AOrbZone::OnSphereOverlap"))
	TArray<AActor*> AttachedActors;
	OtherActor->GetAttachedActors(AttachedActors);
	AOrb* Orb = nullptr;
	for(auto AttachedOrb : AttachedActors) //Check if actor has the orb attached to it. Might be unnecessary
	{
		Orb = Cast<AOrb>(AttachedOrb);
		if(Orb)
		{
			break;
		}
	}
	//UE_LOG(LogTemp, Warning, TEXT("AOrbZone::OnSphereOverlap"))
	
	if(Orb)
	{
		//UE_LOG(LogTemp, Warning, TEXT("AOrbZone::OnSphereOverlap: IF Orb"))
		if(Orb->GetTeam() != Team)
		{
			//UE_LOG(LogTemp, Warning, TEXT("AOrbZone::OnSphereOverlap: IF Orb->GetTeam != Team"))
			CTFGameMode = CTFGameMode == nullptr ? GetWorld()->GetAuthGameMode<ACTFGameMode>() : CTFGameMode;
			if(CTFGameMode)
			{
				//UE_LOG(LogTemp, Warning, TEXT("AOrbZone::OnSphereOverlap: IF CTFGameMode"))
				CTFGameMode->FlagCaptured(Orb, this); 
			}
		}
	}
}





