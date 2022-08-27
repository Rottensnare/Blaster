// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterPlayerController.h"

#include "BlasterPlayerState.h"
#include "BlasterComponents/CombatComponent.h"
#include "Character/BlasterCharacter.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "GameFramework/GameMode.h"
#include "GameMode/BlasterGameMode.h"
#include "GameState/BlasterGameState.h"
#include "HUD/Announcement.h"
#include "HUD/BlasterHUD.h"
#include "HUD/CharacterOverlay.h"
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

void ABlasterPlayerController::HandleMatchHasStarted()
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	if(BlasterHUD)
	{
		BlasterHUD->AddCharacterOverlay();
		if(BlasterHUD->WarmupWidget)
		{
			BlasterHUD->WarmupWidget->SetVisibility(ESlateVisibility::Hidden);
		}
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
	if(CharacterOverlay == nullptr) //Waiting for character overlay to be initialized so that HUD can be updated.
	{
		if(BlasterHUD && BlasterHUD->BlasterOverlay)
		{
			CharacterOverlay = BlasterHUD->BlasterOverlay;
			if(CharacterOverlay)
			{
				SetHUDHealth(HUDHealth, HUDMaxHealth);
				SetHUDScore(HUDScore);
				SetHUDElims(HUDElims);
				bInitializeCharacterOverlay = false;
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
	OnMatchStateSet(MatchState);
	
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

void ABlasterPlayerController::OnMatchStateSet(FName State)
{
	MatchState = State;

	if(MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
	else if(MatchState == MatchState::WaitingToStart)
	{
		
	}
	else if(MatchState == MatchState::Cooldown)
	{
		HandleCooldown();
	}
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
			const FString InfoText{"Top Scoring Players: "};
			BlasterHUD->WarmupWidget->InfoText->SetText(FText::FromString(InfoText));
			if(BlasterHUD->WarmupWidget->TopPlayerText)
			{
				ABlasterGameState* BlasterGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));
				if(BlasterGameState)
				{
					TArray<ABlasterPlayerState*> TopPlayers = BlasterGameState->TopScoringPlayers;
					if(TopPlayers.Num() <= 0)
					{
						const FString FailText{"No one won :/"};
						BlasterHUD->WarmupWidget->TopPlayerText->SetText(FText::FromString(FailText));
					}
					else if(TopPlayers.Num() == 1)
					{
						const FString PlayerText = FString::Printf(TEXT("%s"), *BlasterGameState->TopScoringPlayers[0]->GetPlayerName());
						BlasterHUD->WarmupWidget->TopPlayerText->SetText(FText::FromString(PlayerText));
					}else if(TopPlayers.Num() >= 2)
					{
						const FString PlayerText = FString::Printf(TEXT("%s & \n%s"), *BlasterGameState->TopScoringPlayers[0]->GetName(), *BlasterGameState->TopScoringPlayers[1]->GetPlayerName());
						BlasterHUD->WarmupWidget->TopPlayerText->SetText(FText::FromString(PlayerText));
					}
				}
			}
		}
	}
}

void ABlasterPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABlasterPlayerController, MatchState);
}
