// Fill out your copyright notice in the Description page of Project Settings.


#include "ReturnToMainMenu.h"

#include "MultiplayerSessionsSubsystem.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Components/Button.h"
#include "GameFramework/GameModeBase.h"

void UReturnToMainMenu::MenuSetup()
{

	AddToViewport();
	SetVisibility(ESlateVisibility::Visible);
	bIsFocusable = true;

	UWorld* World = GetWorld();
	if(World)
	{
		PlayerController = PlayerController == nullptr ? World->GetFirstPlayerController() : PlayerController;
		if(PlayerController)
		{
			FInputModeGameAndUI InputModeGameAndUI;
			InputModeGameAndUI.SetWidgetToFocus(TakeWidget());
			PlayerController->SetInputMode(InputModeGameAndUI);
			PlayerController->SetShowMouseCursor(true);
		}
	}
	
	if(MainMenuButton && !MainMenuButton->OnClicked.IsAlreadyBound(this, &UReturnToMainMenu::ReturnButtonClicked))
	{
		MainMenuButton->OnClicked.AddDynamic(this, &UReturnToMainMenu::ReturnButtonClicked);
	}

	UGameInstance* GameInstance = GetGameInstance();
	if(GameInstance)
	{
		MultiplayerSessionsSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
		if(MultiplayerSessionsSubsystem && !MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.IsAlreadyBound(this, &UReturnToMainMenu::OnDestroySession))
		{
			MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.AddDynamic(this, &UReturnToMainMenu::OnDestroySession);
		}
	}
}

void UReturnToMainMenu::MenuTearDown()
{
	RemoveFromParent();
	UWorld* World = GetWorld();
	if(World)
	{
		PlayerController = PlayerController == nullptr ? World->GetFirstPlayerController() : PlayerController;
		if(PlayerController)
		{
			FInputModeGameOnly InputModeGame;
			PlayerController->SetInputMode(InputModeGame);
			PlayerController->SetShowMouseCursor(false);
		}
	}
	if(MainMenuButton && MainMenuButton->OnClicked.IsAlreadyBound(this, &UReturnToMainMenu::ReturnButtonClicked))
	{
		MainMenuButton->OnClicked.RemoveDynamic(this, &UReturnToMainMenu::ReturnButtonClicked);
	}
	if(MultiplayerSessionsSubsystem && MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.IsAlreadyBound(this, &UReturnToMainMenu::OnDestroySession))
	{
		MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.RemoveDynamic(this, &UReturnToMainMenu::OnDestroySession);
	}
}

bool UReturnToMainMenu::Initialize()
{
	if(!Super::Initialize()) return false;
	
	return true;
}

void UReturnToMainMenu::OnDestroySession(bool bWasSuccessful)
{
	if(!bWasSuccessful)
	{
		MainMenuButton->SetIsEnabled(true);
		return;
	}
	
	UWorld* World = GetWorld();
	if(World)
	{
		AGameModeBase* GameMode = World->GetAuthGameMode<AGameModeBase>();
		if(GameMode)
		{
			GameMode->ReturnToMainMenuHost();
		}
		else
		{
			PlayerController = PlayerController == nullptr ? World->GetFirstPlayerController() : PlayerController;
			if(PlayerController)
			{
				PlayerController->ClientReturnToMainMenuWithTextReason(FText());
			}
		}
		
	}
}

void UReturnToMainMenu::OnPlayerLeftGame()
{
	if(MultiplayerSessionsSubsystem)
	{
		MultiplayerSessionsSubsystem->DestroySession();
	}
}

void UReturnToMainMenu::ReturnButtonClicked()
{
	MainMenuButton->SetIsEnabled(false);

	UWorld* World = GetWorld();
	if(World)
	{
		APlayerController* FirstPlayerController = World->GetFirstPlayerController();
		if(FirstPlayerController)
		{
			ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(FirstPlayerController->GetPawn());
			if(BlasterCharacter)
			{
				
				BlasterCharacter->ServerLeaveGame();
				BlasterCharacter->OnLeftGame.AddDynamic(this, &UReturnToMainMenu::OnPlayerLeftGame);
			}
			else
			{
				MainMenuButton->SetIsEnabled(true);
			}
		}
	}
	
	
}
