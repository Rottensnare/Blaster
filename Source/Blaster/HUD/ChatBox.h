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

	//called from player controller. Creates a new ChatTextBlock widget and adds that to the chat ScrollBox
	void OnTextCommitted(const FText& Text, const FString& PlayerName);

	UPROPERTY(meta = (BindWidget))
	class UScrollBox* ChatTextBox; //Holds all the text blocks

	UPROPERTY(meta = (BindWidget))
	class UEditableTextBox* ChatInput; //Used by players to write to chat

	UPROPERTY(EditAnywhere)
	TSubclassOf<class UChatTextBlock> ChatTextBlockClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<UChatTextBlock*> ChatTextBlocks; //Currently not used

private:

	UPROPERTY()
	APlayerController* OwningController;

	UPROPERTY()
	class ABlasterPlayerState* BlasterPlayerState;
	
};
