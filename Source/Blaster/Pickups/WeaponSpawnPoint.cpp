// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponSpawnPoint.h"

#include "Blaster/Weapon/Weapon.h"


AWeaponSpawnPoint::AWeaponSpawnPoint()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	SpawnPodium = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SpawnPodium"));
	SetRootComponent(SpawnPodium);

	SpawnLocation = CreateDefaultSubobject<USceneComponent>(TEXT("SpawnLocation"));
	SpawnLocation->SetupAttachment(RootComponent);

}

// Called when the game starts or when spawned
void AWeaponSpawnPoint::BeginPlay()
{
	Super::BeginPlay();

	if(HasAuthority())
	{
		SpawnWeapon();
	}
}

void AWeaponSpawnPoint::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AWeaponSpawnPoint::SpawnWeapon()
{
	int32 NumOfWeaponClasses = WeaponClasses.Num();
	if(NumOfWeaponClasses > 0)
	{
		int32 Selection = FMath::RandRange(0, NumOfWeaponClasses - 1);
		SpawnedWeapon = GetWorld()->SpawnActor<AWeapon>(WeaponClasses[Selection], SpawnLocation->GetComponentLocation(), GetActorRotation());
		if(HasAuthority() && SpawnedWeapon)
		{
			SpawnedWeapon->SetWeaponState(EWeaponState::EWS_Initial);
			//When weapon is picked up start the call StartSpawnTimer. Called from the weapon class.
			SpawnedWeapon->OnPickedUpDelegate.AddDynamic(this, &AWeaponSpawnPoint::StartSpawnTimer);
		}
	}
}

void AWeaponSpawnPoint::StartSpawnTimer()
{
	GetWorldTimerManager().SetTimer(SpawnTimer, this, &AWeaponSpawnPoint::SpawnTimerFinished, TimeBetweenSpawns);
}

void AWeaponSpawnPoint::SpawnTimerFinished()
{
	if(HasAuthority())
	{
		SpawnWeapon();
	}
}



