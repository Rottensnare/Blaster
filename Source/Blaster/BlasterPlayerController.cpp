// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterPlayerController.h"

#include "BlasterPlayerState.h"
#include "BlasterComponents/CombatComponent.h"
#include "Character/BlasterCharacter.h"
#include "Components/EditableTextBox.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "GameFramework/GameMode.h"
#include "GameMode/BlasterGameMode.h"
#include "GameState/Announcements.h"
#include "GameState/BlasterGameState.h"
#include "HUD/Announcement.h"
#include "HUD/BlasterHUD.h"
#include "HUD/CharacterOverlay.h"
#include "HUD/ChatBox.h"
#include "HUD/ReturnToMainMenu.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"




void ABlasterPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	BlasterHUD = Cast<ABlasterHUD>(GetHUD());
	ServerCheckMatchState();
}



void ABlasterPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	SetHUDTime();
	
	TimeSyncRunningTime += DeltaSeconds;
	if(IsLocalController() && TimeSyncRunningTime >= TimeSyncFrequency)
	{
		TimeSyncRunningTime = 0.f;
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
	}
	
	PollInit();

	CheckPing(DeltaSeconds);
}



void ABlasterPlayerController::SetHUDTime()
{
	float TimeLeft{0};
	if(MatchState == MatchState::WaitingToStart) TimeLeft = WarmupTime - GetServerTime() + LevelStartingTime;
	else if(MatchState == MatchState::InProgress) TimeLeft = WarmupTime + MatchTime - GetServerTime() + LevelStartingTime;
	else if(MatchState == MatchState::Cooldown) TimeLeft = CooldownTime + WarmupTime + MatchTime - GetServerTime() + LevelStartingTime;

	int32 SecondsLeft = FMath::CeilToInt(TimeLeft);
	
	if(HasAuthority())
	{
		BlasterGameMode = BlasterGameMode == nullptr ? Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this)) : BlasterGameMode;
		if(BlasterGameMode)
		{
			SecondsLeft = FMath::CeilToInt(BlasterGameMode->GetCountdownTime() + LevelStartingTime);
		}
	}
	
	if(CountdownInt != SecondsLeft) //Passes if check once every second
	{
		if(MatchState == MatchState::WaitingToStart || MatchState == MatchState::Cooldown)
		{
			SetHUDAnnouncementTime(TimeLeft);
		}
		if(MatchState == MatchState::InProgress)
		{
			SetMatchTimeText(TimeLeft);
		}
	}
	CountdownInt = SecondsLeft;
}

void ABlasterPlayerController::HandleMatchHasStarted(bool bTeamsMatch)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	if(BlasterHUD)
	{
		if(HasAuthority()) bShowTeamScores = bTeamsMatch;
		BlasterHUD->AddCharacterOverlay();
		if(BlasterHUD->WarmupWidget)
		{
			BlasterHUD->WarmupWidget->SetVisibility(ESlateVisibility::Hidden);
		}
		
		AddChatBox();

		if(!HasAuthority()) return;
		if(bTeamsMatch)
		{
			InitTeamScores();
		}
		else
		{
			HideTeamScores();
		}
	}
}

void ABlasterPlayerController::HideTeamScores()
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	
	bool bIsHUDValid = BlasterHUD
	&& BlasterHUD->BlasterOverlay
	&& BlasterHUD->BlasterOverlay->RedTeamScoreText
	&& BlasterHUD->BlasterOverlay->BlueTeamScoreText
	&& BlasterHUD->BlasterOverlay->TeamScoreDivider;
	if(bIsHUDValid)
	{
		BlasterHUD->BlasterOverlay->RedTeamScoreText->SetVisibility(ESlateVisibility::Collapsed);
		BlasterHUD->BlasterOverlay->BlueTeamScoreText->SetVisibility(ESlateVisibility::Collapsed);
		BlasterHUD->BlasterOverlay->TeamScoreDivider->SetVisibility(ESlateVisibility::Collapsed);;
	}
}

void ABlasterPlayerController::InitTeamScores()
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	
	bool bIsHUDValid = BlasterHUD
	&& BlasterHUD->BlasterOverlay
	&& BlasterHUD->BlasterOverlay->RedTeamScoreText
	&& BlasterHUD->BlasterOverlay->BlueTeamScoreText
	&& BlasterHUD->BlasterOverlay->TeamScoreDivider;
	if(bIsHUDValid)
	{
		BlasterHUD->BlasterOverlay->RedTeamScoreText->SetText(FText::FromString("0"));
		BlasterHUD->BlasterOverlay->BlueTeamScoreText->SetText(FText::FromString("0"));
		BlasterHUD->BlasterOverlay->TeamScoreDivider->SetText(FText::FromString("|"));
	}
}

void ABlasterPlayerController::SetHUDRedTeamScore(int32 RedScore)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	
	bool bIsHUDValid = BlasterHUD
	&& BlasterHUD->BlasterOverlay
	&& BlasterHUD->BlasterOverlay->RedTeamScoreText;
	if(bIsHUDValid)
	{
		BlasterHUD->BlasterOverlay->RedTeamScoreText->SetText(FText::FromString(FString::Printf(TEXT("%d"), RedScore))); //Seems perfectly readable to me :)
	}
}

void ABlasterPlayerController::SetHUDBlueTeamScore(int32 BlueScore)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	
	bool bIsHUDValid = BlasterHUD
	&& BlasterHUD->BlasterOverlay
	&& BlasterHUD->BlasterOverlay->BlueTeamScoreText;
	if(bIsHUDValid)
	{
		BlasterHUD->BlasterOverlay->BlueTeamScoreText->SetText(FText::FromString(FString::Printf(TEXT("%d"), BlueScore)));
	}
}

void ABlasterPlayerController::ServerCheckMatchState_Implementation()
{
	ABlasterGameMode* GameMode = Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this));
	if(GameMode)
	{
		WarmupTime = GameMode->WarmupTime;
		MatchTime = GameMode->MatchTime;
		LevelStartingTime = GameMode->LevelStartingTime;
		MatchState = GameMode->GetMatchState();
		CooldownTime = GameMode->CooldownTime;
		ClientJoinMidGame(MatchState, WarmupTime, MatchTime, LevelStartingTime, CooldownTime);

		if(BlasterHUD && MatchState == MatchState::WaitingToStart)
		{
			UE_LOG(LogTemp, Warning, TEXT("ServerCheckMatchState"))
			BlasterHUD->AddWarmupWidget();
		}
	}
}

void ABlasterPlayerController::PollInit()
{
	if (CharacterOverlay == nullptr)
	{
		if (BlasterHUD && BlasterHUD->BlasterOverlay)
		{
			CharacterOverlay = BlasterHUD->BlasterOverlay;
			if (CharacterOverlay)
			{
				if (bInitializeHealth) SetHUDHealth(HUDHealth, HUDMaxHealth);
				if (bInitializeShield) SetHUDShields(HUDShields, HUDMaxShields);
				if (bInitializeScore) SetHUDScore(HUDScore);
				if (bInitializeDefeats) SetHUDElims(HUDElims);
				if (bInitializeCarriedAmmo) SetHUDTotalAmmo(HUDCarriedAmmo);
				if (bInitializeWeaponAmmo) SetHUDAmmo(HUDWeaponAmmo);
			}
		}
	}
}

void ABlasterPlayerController::CheckPing(float DeltaSeconds)
{
	HighPingRunningTime += DeltaSeconds;
	if(HighPingRunningTime >= CheckPingFrequency)
	{
		PlayerState = PlayerState == nullptr ? GetPlayerState<APlayerState>() : PlayerState;
		if(PlayerState)
		{
			if(PlayerState->GetCompressedPing() * 4 > HighPingThreshold) //GetCompressedPing returns ping / 4 to compress it to a uint8
			{
				HighPingWarning();
				PingAnimRunningTime = 0.f;
				ServerReportPingStatus(true);
			}
			else
			{
				ServerReportPingStatus(false);
			}
		}
		HighPingRunningTime = 0.f;
	}
	if(BlasterHUD &&
		BlasterHUD->BlasterOverlay &&
		BlasterHUD->BlasterOverlay->HighPingAnimation &&
		BlasterHUD->BlasterOverlay->IsAnimationPlaying(BlasterHUD->BlasterOverlay->HighPingAnimation))
	{
		PingAnimRunningTime += DeltaSeconds;
		if(PingAnimRunningTime >= HighPingDuration)
		{
			StopHighPingWarning();
		}
	}
}

void ABlasterPlayerController::ServerReportPingStatus_Implementation(bool bHighPing)
{
	OnHighPingChecked.Broadcast(bHighPing);
}

void ABlasterPlayerController::HighPingWarning()
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	
	bool bIsHUDValid = BlasterHUD
	&& BlasterHUD->BlasterOverlay
	&& BlasterHUD->BlasterOverlay->HighPingImage
	&& BlasterHUD->BlasterOverlay->HighPingAnimation;
	
	if(bIsHUDValid)
	{
		BlasterHUD->BlasterOverlay->HighPingImage->SetOpacity(1.f);
		BlasterHUD->BlasterOverlay->PlayAnimation(BlasterHUD->BlasterOverlay->HighPingAnimation, 0.f, 5);
	}
}

void ABlasterPlayerController::StopHighPingWarning()
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	
	bool bIsHUDValid = BlasterHUD
	&& BlasterHUD->BlasterOverlay
	&& BlasterHUD->BlasterOverlay->HighPingImage
	&& BlasterHUD->BlasterOverlay->HighPingAnimation;
	
	if(bIsHUDValid)
	{
		BlasterHUD->BlasterOverlay->HighPingImage->SetOpacity(0.f);
		if(BlasterHUD->BlasterOverlay->IsAnimationPlaying(BlasterHUD->BlasterOverlay->HighPingAnimation))
		{
			BlasterHUD->BlasterOverlay->StopAnimation(BlasterHUD->BlasterOverlay->HighPingAnimation);
		}
	}
}

void ABlasterPlayerController::OnRep_ShowTeamScores()
{
	if(bShowTeamScores)
	{
		InitTeamScores();
	}
	else
	{
		HideTeamScores();
	}
}



void ABlasterPlayerController::BroadCastElim(APlayerState* Attacker, APlayerState* Victim)
{
	ClientElimAnnouncement(Attacker, Victim);
}

void ABlasterPlayerController::ClientElimAnnouncement_Implementation(APlayerState* Attacker, APlayerState* Victim)
{
	APlayerState* Self = GetPlayerState<APlayerState>();
	if(Attacker && Victim && Self)
	{
		BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
		if(BlasterHUD)
		{
			if(Attacker != Self && Victim != Self)
			{
				BlasterHUD->AddElimAnnouncement(Attacker->GetPlayerName(), Victim->GetPlayerName());
				return;
			} 
			if(Attacker == Self && Victim != Self)
			{
				BlasterHUD->AddElimAnnouncement("You", Victim->GetPlayerName());
				return;
			}
			if(Victim == Self && Attacker != Self)
			{
				BlasterHUD->AddElimAnnouncement(Attacker->GetPlayerName(), "You");
				return;
			}
			if(Attacker == Self && Victim == Self)
			{
				BlasterHUD->AddElimAnnouncement("You", "Yourself");
				return;
			}
			if(Attacker == Victim && Attacker != Self)
			{
				BlasterHUD->AddElimAnnouncement(Attacker->GetPlayerName(), "Themselves");
			}
			
		}
	}
}


void ABlasterPlayerController::OnRep_MatchState()
{
	if(MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
	else if(MatchState == MatchState::Cooldown)
	{
		HandleCooldown();
	}
}

void ABlasterPlayerController::ClientJoinMidGame_Implementation(FName InMatchState, float InWarmup, float InMatchTime, float InStartingTime, float InCooldownTime)
{
	if(HasAuthority()) return;
	MatchState = InMatchState;
	WarmupTime = InWarmup;
	MatchTime = InMatchTime;
	LevelStartingTime = InStartingTime;
	CooldownTime = InCooldownTime;
	OnMatchStateSet(MatchState, bShowTeamScores);
	
	if(BlasterHUD && MatchState == MatchState::WaitingToStart)
	{
		UE_LOG(LogTemp, Warning, TEXT("ClientJoinMidGame"))
		BlasterHUD->AddWarmupWidget();
	}
}

void ABlasterPlayerController::ClientReportServerTime_Implementation(const float TimeOfClientRequest,
                                                                     const float TimeServerReceivedClientRequest)
{
	const float RoundTripTime = GetWorld()->GetTimeSeconds() - TimeOfClientRequest;
	SingleTripTime = RoundTripTime / 2;
	const float CurrentServerTime = TimeServerReceivedClientRequest + (0.5f * RoundTripTime);
	ClientServerDelta = CurrentServerTime - GetWorld()->GetTimeSeconds();
}

void ABlasterPlayerController::ServerRequestServerTime_Implementation(const float TimeOfClientRequest)
{
	const float ServerTimeOfRReceipt = GetWorld()->GetTimeSeconds();
	ClientReportServerTime(TimeOfClientRequest, ServerTimeOfRReceipt);
}



void ABlasterPlayerController::SetHUDHealth(const float Health, const float MaxHealth)
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
	else
	{
		bInitializeCharacterOverlay = true;
		HUDHealth = Health;
		HUDMaxHealth = MaxHealth;
	}
}

void ABlasterPlayerController::SetHUDShields(float Shields, float MaxShields)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	bool bIsHUDValid = BlasterHUD
		&& BlasterHUD->BlasterOverlay
		&& BlasterHUD->BlasterOverlay->ShieldBar
		&& BlasterHUD->BlasterOverlay->ShieldText;
	
	if(bIsHUDValid)
	{
		const float ShieldPercentage = Shields / MaxShields;
		BlasterHUD->BlasterOverlay->ShieldBar->SetPercent(ShieldPercentage);
		FString ShieldText = FString::Printf(TEXT("%d / %d"), FMath::TruncToInt32(Shields), FMath::TruncToInt32(MaxShields));
		BlasterHUD->BlasterOverlay->ShieldText->SetText(FText::FromString(ShieldText));
	}
	else
	{
		bInitializeCharacterOverlay = true;
		HUDShields = Shields;
		HUDMaxShields = MaxShields;
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
	else
	{
		bInitializeCharacterOverlay = true;
		HUDScore = Score;
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
	else
	{
		bInitializeCharacterOverlay = true;
		HUDElims = Elims;
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
	}else
	{
		bInitializeWeaponAmmo = true;
		HUDWeaponAmmo = Ammo;
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
		FString MagAmmoText = FString::Printf(TEXT("/ %d"), MagAmmo);
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
	}else
	{
		bInitializeCarriedAmmo = true;
		HUDCarriedAmmo = TotalAmmo;
	}
}

void ABlasterPlayerController::SetHUDWeaponType(FString WeaponType)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bIsHUDValid = BlasterHUD
		&& BlasterHUD->BlasterOverlay
		&& BlasterHUD->BlasterOverlay->WeaponTypeText;

	if(bIsHUDValid)
	{
		BlasterHUD->BlasterOverlay->WeaponTypeText->SetText(FText::FromString(WeaponType));
	}
}

void ABlasterPlayerController::SetMatchTimeText(float InMatchTime)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bIsHUDValid = BlasterHUD
		&& BlasterHUD->BlasterOverlay
		&& BlasterHUD->BlasterOverlay->MatchTimeText;

	if(bIsHUDValid)
	{
		const int32 MatchMinutes = InMatchTime / 60.f;
		const int32 MatchSeconds = InMatchTime - (MatchMinutes * 60);
		FString MatchTimeText = FString::Printf(TEXT("%02d : %02d"), MatchMinutes, MatchSeconds);
		BlasterHUD->BlasterOverlay->MatchTimeText->SetText(FText::FromString(MatchTimeText));
	}
}

void ABlasterPlayerController::SetHUDAnnouncementTime(float InWarmupTime)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bIsHUDValid = BlasterHUD
		&& BlasterHUD->WarmupWidget
		&& BlasterHUD->WarmupWidget->WarmupTimeText;

	if(bIsHUDValid)
	{
		const int32 MatchMinutes = FMath::FloorToInt(InWarmupTime / 60.f);
		const int32 MatchSeconds = InWarmupTime - (MatchMinutes * 60);
		const FString AnnouncementTimeText = FString::Printf(TEXT("%02d : %02d"), MatchMinutes, MatchSeconds);
		BlasterHUD->WarmupWidget->WarmupTimeText->SetText(FText::FromString(AnnouncementTimeText));
		UE_LOG(LogTemp, Warning, TEXT("SetHUDAnnouncement: %02d : %02d"),MatchMinutes, MatchSeconds)
	}
}


float ABlasterPlayerController::GetServerTime()
{
	if(HasAuthority()) return GetWorld()->GetTimeSeconds();
	else return GetWorld()->GetTimeSeconds() + ClientServerDelta;
}

void ABlasterPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();

	if(IsLocalController())
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
	}
}

void ABlasterPlayerController::OnMatchStateSet(FName State, bool bTeamsMatch)
{
	MatchState = State;

	if(MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted(bTeamsMatch);
	}
	else if(MatchState == MatchState::WaitingToStart)
	{
		
	}
	else if(MatchState == MatchState::Cooldown)
	{
		HandleCooldown();
	}
}

FString ABlasterPlayerController::GetInfoText(const TArray<ABlasterPlayerState*>& PlayerStates)
{
	ABlasterGameState* BlasterGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));
	if(BlasterGameState == nullptr) return FString();
	
	FString InfoTextString;
	if(PlayerStates.Num() <= 0)
	{
		const FString FailText{Announcements::ThereIsNoWinner};
		BlasterHUD->WarmupWidget->TopPlayerText->SetText(FText::FromString(FailText));
	}
	else if(PlayerStates.Num() == 1)
	{
		const FString PlayerText = FString::Printf(TEXT("%s"), *BlasterGameState->TopScoringPlayers[0]->GetPlayerName());
		BlasterHUD->WarmupWidget->TopPlayerText->SetText(FText::FromString(PlayerText));
	}else if(PlayerStates.Num() >= 2)
	{
		const FString PlayerText = FString::Printf(TEXT("%s & \n%s"), *BlasterGameState->TopScoringPlayers[0]->GetName(), *BlasterGameState->TopScoringPlayers[1]->GetPlayerName());
		BlasterHUD->WarmupWidget->TopPlayerText->SetText(FText::FromString(PlayerText));
	}

	return InfoTextString;
}

FString ABlasterPlayerController::GetTeamsInfoText(ABlasterGameState const * BlasterGameState)
{
	if(BlasterGameState == nullptr) return FString();
	FString InfoTextString{""};

	if(BlasterGameState->BlueTeamScore > BlasterGameState->RedTeamScore)
	{
		InfoTextString = Announcements::BlueTeamWins;
	}
	else if(BlasterGameState->BlueTeamScore < BlasterGameState->RedTeamScore)
	{
		InfoTextString = Announcements::RedTeamWins;
	}
	else
	{
		InfoTextString = Announcements::TeamsDraw;
	}
	
	return InfoTextString;
}

void ABlasterPlayerController::HandleCooldown()
{
	if(ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(GetPawn()))
	{
		BlasterCharacter->bDisableGameplay = true;
		if(BlasterCharacter->GetCombatComponent())
		{
			BlasterCharacter->GetCombatComponent()->FireButtonPressed(false);
		}
	}
	
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	if(BlasterHUD)
	{
		BlasterHUD->BlasterOverlay->RemoveFromParent();
		if(BlasterHUD->WarmupWidget && BlasterHUD->WarmupWidget->InfoText)
		{
			BlasterHUD->WarmupWidget->SetVisibility(ESlateVisibility::Visible);
			const FString InfoText{Announcements::TopScoringPlayers};
			BlasterHUD->WarmupWidget->InfoText->SetText(FText::FromString(InfoText));
			if(BlasterHUD->WarmupWidget->TopPlayerText)
			{
				ABlasterGameState* BlasterGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));
				if(BlasterGameState)
				{
					FString InfoTextString = bShowTeamScores ? GetTeamsInfoText(BlasterGameState) : GetInfoText(BlasterGameState->TopScoringPlayers);
					BlasterHUD->WarmupWidget->TopPlayerText->SetText(FText::FromString(InfoTextString));
				}
			}
		}
	}
}

void ABlasterPlayerController::ShowReturnToMainMenu()
{
	if(ReturnToMainMenuClass == nullptr) return;
	if(ReturnToMainMenu == nullptr)
	{
		ReturnToMainMenu = CreateWidget<UReturnToMainMenu>(this, ReturnToMainMenuClass);
	}
	if(ReturnToMainMenu)
	{
		bReturnToMainMenuOpen = !bReturnToMainMenuOpen;
		if(bReturnToMainMenuOpen)
		{
			ReturnToMainMenu->MenuSetup();
		}
		else
		{
			ReturnToMainMenu->MenuTearDown();
		}
	}
}

void ABlasterPlayerController::ToggleChatBox()
{
	if(ChatBox && ChatBox->ChatInput)
	{
		if(ChatBox->GetVisibility() == ESlateVisibility::Collapsed)
		{
			ChatBox->SetVisibility(ESlateVisibility::Visible);
			FInputModeGameAndUI InputModeGameAndUI;
			InputModeGameAndUI.SetWidgetToFocus(ChatBox->ChatInput->TakeWidget());
			SetInputMode(InputModeGameAndUI);
			SetShowMouseCursor(true);
		}
		else if(ChatBox->GetVisibility() == ESlateVisibility::Visible)
		{
			ChatBox->SetVisibility(ESlateVisibility::Collapsed);
			FInputModeGameOnly InputModeGameOnly;
			SetInputMode(InputModeGameOnly);
			SetShowMouseCursor(false);
		}
	}
}

void ABlasterPlayerController::AddChatBox()
{
	if(ChatBoxClass)
	{
		ChatBox = CreateWidget<UChatBox>(this, ChatBoxClass);
		if(ChatBox && ChatBox->ChatInput)
		{
			ChatBox->AddToViewport();
			ChatBox->ChatInput->SetHintText(FText(FText::FromString("-")));
			ChatBox->SetVisibility(ESlateVisibility::Collapsed);
			ChatBox->ChatInput->RevertTextOnEscape = true;
			ChatBox->ChatInput->OnTextCommitted.AddDynamic(this, &ABlasterPlayerController::OnChatCommitted);
		}
	}
}


void ABlasterPlayerController::OnChatCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	if(CommitMethod != ETextCommit::OnEnter) return;
	
	FString PlayerName{""};
	
	APlayerState* TempPlayerState = GetPlayerState<APlayerState>();
	if(TempPlayerState)
	{
		PlayerName = TempPlayerState->GetPlayerName();
	}
	
	ServerChatCommitted(Text, PlayerName); //TODO: Add a delay so that some absolute cucumber can't spam the chat.
}

void ABlasterPlayerController::ClientChatCommitted_Implementation(const FText& Text, const FString& PlayerName)
{
	if(ChatBox)
	{
		ChatBox->OnTextCommitted(Text, PlayerName);
	}
}

void ABlasterPlayerController::ServerChatCommitted_Implementation(const FText& Text, const FString& PlayerName)
{
	BlasterGameMode->SendChatMsg(Text, PlayerName);
}


void ABlasterPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if(InputComponent == nullptr) return;

	InputComponent->BindAction("Quit", IE_Pressed, this, &ABlasterPlayerController::ShowReturnToMainMenu);
	InputComponent->BindAction("Chat", IE_Pressed, this, &ABlasterPlayerController::ToggleChatBox);
	
}

void ABlasterPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABlasterPlayerController, MatchState);
	DOREPLIFETIME(ABlasterPlayerController, bShowTeamScores);
}
