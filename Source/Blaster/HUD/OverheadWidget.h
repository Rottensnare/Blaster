// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OverheadWidget.generated.h"

/**
 * Used to show player's net role
 */
UCLASS()
class BLASTER_API UOverheadWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* DisplayText;

	//Sets the DisplayText text to TextToDisplay. Called from ShowPlayerNetRole
	void SetDisplayText(FString TextToDisplay);

	//Gets players Remote role and calls SetDisplayText with the created string
	UFUNCTION(BlueprintCallable)
	void ShowPlayerNetRole(APawn* InPawn);

protected:

	//Removes widget from parent when switching levels
	virtual void OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld) override;


private:


	
	
};
