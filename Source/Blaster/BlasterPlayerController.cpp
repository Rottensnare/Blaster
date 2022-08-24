// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterPlayerController.h"

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