// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "BlasterPlayerController.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHighPingChecked, bool, bPingTooHigh);

/**
 * 
 */
UCLASS()
class BLASTER_API ABlasterPlayerController : public APlayerController
{
	GENERATED_BODY()

public:

	
	
	void SetHUDHealth(float Health, float MaxHealth);
	void SetHUDShields(float Shields, float MaxShields);
	virtual void OnPossess(APawn* InPawn) override;
	void SetHUDScore(float Score);
	void SetHUDElims(int32 Elims);
	void SetHUDAmmo(int32 Ammo);
	void SetHUDMagText(int32 MagAmmo);
	void SetHUDTotalAmmo(int32 TotalAmmo);
	void SetHUDWeaponType(FString WeaponType);
	void SetMatchTimeText(float MatchTime);
	void SetHUDAnnouncementTime(float WarmupTime);
	void BroadCastElim(APlayerState* Attacker, APlayerState* Victim);

	virtual float GetServerTime();
	virtual void ReceivedPlayer() override;
	void OnMatchStateSet(FName State);
	void HandleCooldown();
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	float SingleTripTime = 0.f;

	FOnHighPingChecked OnHighPingChecked;

	void AddChatBox();
	void ToggleChatBox();

	UFUNCTION()
	void OnChatCommitted(const FText& Text, ETextCommit::Type CommitMethod);
	
	UFUNCTION(Client, Reliable)
	void ClientChatCommitted(const FText& Text, const FString& PlayerName);
	
protected:

	virtual void SetupInputComponent() override;
	void ShowReturnToMainMenu();
	
	virtual void BeginPlay() override;
	void CheckPing(float DeltaSeconds);
	virtual void Tick(float DeltaSeconds) override;
	void SetHUDTime();
	void HandleMatchHasStarted();

	UFUNCTION(Server, Reliable)
	void ServerCheckMatchState();

	UFUNCTION(Server, Reliable)
	void ServerRequestServerTime(const float TimeOfClientRequest);

	UFUNCTION(Client, Reliable)
	void ClientReportServerTime(const float TimeOfClientRequest, const float TimeServerReceivedClientRequest);
	
	UFUNCTION(Server, Reliable)
	void ServerChatCommitted(const FText& Text, const FString& PlayerName);
	
	void PollInit();

	float ClientServerDelta{0.f};
	
	UPROPERTY(EditAnywhere, Category = Time)
	float TimeSyncFrequency{5.f};
	
	float TimeSyncRunningTime{0.f};

	void HighPingWarning();
	void StopHighPingWarning();

	UFUNCTION(Server, Reliable)
	void ServerReportPingStatus(bool bHighPing);

	float HighPingRunningTime{0.f};
	UPROPERTY(EditAnywhere)
	float HighPingDuration{5.f};
	float PingAnimRunningTime{0.f};
	UPROPERTY(EditAnywhere)
	float CheckPingFrequency{20.f};
	UPROPERTY(EditAnywhere)
	float HighPingThreshold{85.f};

	UFUNCTION(Client, Reliable)
	void ClientElimAnnouncement(APlayerState* Attacker, APlayerState* Victim);

private:

	UPROPERTY(EditAnywhere, Category = HUD)
	TSubclassOf<class UUserWidget> ReturnToMainMenuClass;

	UPROPERTY()
	class UReturnToMainMenu* ReturnToMainMenu;

	bool bReturnToMainMenuOpen{false};
	
	UPROPERTY()
	class ABlasterHUD* BlasterHUD;

	UPROPERTY()
	class ABlasterGameMode* BlasterGameMode;

	float MatchTime{0.f};
	float WarmupTime{0.f};
	float LevelStartingTime{0.f};
	float CooldownTime{0.f};
	
	uint32 CountdownInt{0};

	UPROPERTY(ReplicatedUsing = OnRep_MatchState)
	FName MatchState{FName()};

	UFUNCTION()
	void OnRep_MatchState();

	UFUNCTION(Client, Reliable)
	void ClientJoinMidGame(FName InMatchState, float InWarmup, float InMatchTime, float InStartingTime, float InCooldownTime);

	UPROPERTY()
	class UCharacterOverlay* CharacterOverlay;

	bool bInitializeCharacterOverlay{false};

	UPROPERTY(EditAnywhere)
	TSubclassOf<class UChatBox> ChatBoxClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UChatBox* ChatBox;

	//Cached values that will be used to set HUD element values when Overlay is initialized.
	//Overlay will be initialized late so we need to do this.
	float HUDHealth;
	float HUDMaxHealth;
	bool bInitializeHealth{false}; // All bInit booleans are set to true when HUD is valid.
	float HUDShields;
	float HUDMaxShields;
	bool bInitializeShields{false}; 
	float HUDScore;
	float HUDElims;
	bool bInitializeDefeats{false}; 
	bool bInitializeScore{false};
	bool bInitializeShield{false};
	
	float HUDCarriedAmmo;
	bool bInitializeCarriedAmmo = false;
	float HUDWeaponAmmo;
	bool bInitializeWeaponAmmo = false;
};
