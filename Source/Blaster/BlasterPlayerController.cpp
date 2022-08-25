// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterPlayerController.h"

#include "Character/BlasterCharacter.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "HUD/BlasterHUD.h"
#include "HUD/CharacterOverlay.h"


void ABlasterPlayerController::BeginPlay()
{
	Super::BeginPlay();

	BlasterHUD = Cast<ABlasterHUD>(GetHUD());
}

void ABlasterPlayerController::SetHUDHealth(float Health, float MaxHealth)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	bool bIsHUDValid = BlasterHUD
		&& BlasterHUD->BlasterOverlay
		&& BlasterHUD->BlasterOverlay->HealthBar
		&& BlasterHUD->BlasterOverlay->HealthText;
	
	if(bIsHUDValid)
	{
		const float HealthPercentage = Health / MaxHealth;
		BlasterHUD->BlasterOverlay->HealthBar->SetPercent(HealthPercentage);
		//Using Truncate instead of Ceil, because it's funnier to see 0 when someone is close to dying.
		//They are going to be confused.
		//Might think this is a shit game for letting someone live with no HP.
		//Well whatever
		FString HealthText = FString::Printf(TEXT("%d / %d"), FMath::TruncToInt32(Health), FMath::TruncToInt32(MaxHealth));
		BlasterHUD->BlasterOverlay->HealthText->SetText(FText::FromString(HealthText));
	}
}

void ABlasterPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(InPawn);
	if(BlasterCharacter)
	{
		SetHUDHealth(BlasterCharacter->GetHealth(), BlasterCharacter->GetMaxHealth());
	}
}

void ABlasterPlayerController::SetHUDScore(float Score)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bIsHUDValid = BlasterHUD
		&& BlasterHUD->BlasterOverlay
		&& BlasterHUD->BlasterOverlay->ScoreAmount;

	if(bIsHUDValid)
	{
		FString ScoreText = FString::Printf(TEXT("%d"), FMath::FloorToInt(Score));
		BlasterHUD->BlasterOverlay->ScoreAmount->SetText(FText::FromString(ScoreText));
	}
}

void ABlasterPlayerController::SetHUDElims(int32 Elims)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bIsHUDValid = BlasterHUD
		&& BlasterHUD->BlasterOverlay
		&& BlasterHUD->BlasterOverlay->ElimsAmount;

	if(bIsHUDValid)
	{
		FString ElimsText = FString::Printf(TEXT("%d"), Elims);
		BlasterHUD->BlasterOverlay->ElimsAmount->SetText(FText::FromString(ElimsText));
	}
}

void ABlasterPlayerController::SetHUDAmmo(int32 Ammo)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bIsHUDValid = BlasterHUD
		&& BlasterHUD->BlasterOverlay
		&& BlasterHUD->BlasterOverlay->AmmoText;

	if(bIsHUDValid)
	{
		FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		BlasterHUD->BlasterOverlay->AmmoText->SetText(FText::FromString(AmmoText));
	}
}

void ABlasterPlayerController::SetHUDMagText(int32 MagAmmo)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bIsHUDValid = BlasterHUD
		&& BlasterHUD->BlasterOverlay
		&& BlasterHUD->BlasterOverlay->MagText;

	if(bIsHUDValid)
	{
		FString MagAmmoText = FString::Printf(TEXT("%d"), MagAmmo);
		BlasterHUD->BlasterOverlay->MagText->SetText(FText::FromString(MagAmmoText));
	}
}

void ABlasterPlayerController::SetHUDTotalAmmo(int32 TotalAmmo)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bIsHUDValid = BlasterHUD
		&& BlasterHUD->BlasterOverlay
		&& BlasterHUD->BlasterOverlay->TotalAmmoText;

	if(bIsHUDValid)
	{
		FString TotalText = FString::Printf(TEXT("%d"), TotalAmmo);
		BlasterHUD->BlasterOverlay->TotalAmmoText->SetText(FText::FromString(TotalText));
	}
}
