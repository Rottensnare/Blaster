// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CharacterOverlay.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API UCharacterOverlay : public UUserWidget
{
	GENERATED_BODY()

public:

	UPROPERTY(meta = (BindWidget))
	class UProgressBar* HealthBar;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* HealthText;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* ShieldBar;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ShieldText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ScoreAmount;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ElimsAmount;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* AmmoText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* MagText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TotalAmmoText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* WeaponTypeText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* MatchTimeText;

	UPROPERTY(meta = (BindWidget))
	class UImage* HighPingImage;

	UPROPERTY(meta = (BindWidgetAnim), Transient) 
	class UWidgetAnimation* HighPingAnimation;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* BlueTeamScoreText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* RedTeamScoreText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TeamScoreDivider;


protected:



private:
	
};
