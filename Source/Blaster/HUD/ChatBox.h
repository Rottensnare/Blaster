// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ChatBox.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API UChatBox : public UUserWidget
{
	GENERATED_BODY()

public:
	
	void OnTextCommitted(const FText& Text, const FString& PlayerName);

	UPROPERTY(meta = (BindWidget))
	class UScrollBox* ChatTextBox;

	UPROPERTY(meta = (BindWidget))
	class UEditableTextBox* ChatInput;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class UChatTextBlock> ChatTextBlockClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<UChatTextBlock*> ChatTextBlocks;

private:

	UPROPERTY()
	APlayerController* OwningController;

	UPROPERTY()
	class ABlasterPlayerState* BlasterPlayerState;
	
};
