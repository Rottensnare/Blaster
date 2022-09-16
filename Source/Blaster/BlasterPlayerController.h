// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BlasterPlayerState.h"
#include "GameFramework/PlayerController.h"
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
	//Sets HUD health bar percentage and Health text value based on values passed in
	void SetHUDHealth(float Health, float MaxHealth); 
	void SetHUDShields(float Shields, float MaxShields); //Very similar to SetHUDHealth function
	virtual void OnPossess(APawn* InPawn) override; //Called when controller get possessed. Calls SetHUDHealth and SetHUDShields.
	void SetHUDScore(float Score); //Updates player score in the HUD
	void SetHUDElims(int32 Elims); //Updates player eliminations in the HUD
	void SetHUDAmmo(int32 Ammo); //Updates player ammo in the HUD
	void SetHUDMagText(int32 MagAmmo); //Updates player magazine capacity in the HUD
	void SetHUDTotalAmmo(int32 TotalAmmo); //Updates player total ammo in the HUD
	void SetHUDWeaponType(FString WeaponType); //Updates weapon type in the HUD
	void SetMatchTimeText(float MatchTime); //Updates match time in the HUD
	void SetHUDAnnouncementTime(float WarmupTime); //Updates time for match start and cooldown timers in the HUD
	void BroadCastElim(APlayerState* Attacker, APlayerState* Victim); //Simply calls ClientElimAnnouncement function

	virtual float GetServerTime(); //Returns server time
	virtual void ReceivedPlayer() override; //Called after this PlayerController's viewport/net connection is associated with this player controller.
	void OnMatchStateSet(FName State, bool bTeamsMatch); //Sets matchstate and calls functions to handle the current match state
	void HandleCooldown(); //Handles the custom Cooldown match state behavior. 
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	float SingleTripTime = 0.f; //Client RTT / 2

	FOnHighPingChecked OnHighPingChecked; //Used by the weapon class. When ping is too high, SSR is disabled

	void AddChatBox(); //Creates ChatBox widget and handles the initialization
	void ToggleChatBox(); //Sets ESlateVisibility for the ChatBox

	//Called when player commits a chat message
	UFUNCTION()
	void OnChatCommitted(const FText& Text, ETextCommit::Type CommitMethod);

	//Client version of OnChatCommitted
	UFUNCTION(Client, Reliable)
	void ClientChatCommitted(const FText& Text, const FString& PlayerName);

	void SetHUDRedTeamScore(int32 RedScore); //Sets Red teams score in the HUD
	void SetHUDBlueTeamScore(int32 BlueScore); //Sets Blue teams score in the HUD

	//Called from the CTF GameMode class
	UFUNCTION(Client, Reliable)
	void ClientOrbAnnouncement(APlayerState* InOrbHolder, uint8 Selection);
	
protected:

	virtual void SetupInputComponent() override; //Handles Quit and Chat toggling inputs
	//Creates menu widget, if open will call MenuTearDown, if closed will call MenuSetup.
	void ShowReturnToMainMenu(); 
	
	virtual void BeginPlay() override; //Calls ServerCheckMatchState
	void CheckPing(float DeltaSeconds); //Handles functionality for high ping based on CheckPingFrequency
	//Calls SetHUDTime, tries to synchronize time every so often, calls CheckPing
	virtual void Tick(float DeltaSeconds) override; 
	void SetHUDTime(); //Handles the HUD time based on the match state
	//Handles the start of the match, like adding the ChatBox, initializing team scores
	void HandleMatchHasStarted(bool bTeamsMatch = false); 
	void HideTeamScores(); //Hides team scores from the HUD
	void InitTeamScores(); //Initializes team scores in the HUD
	
	UFUNCTION(Server, Reliable)
	void ServerCheckMatchState(); //Gets values from the game mode, calls ClientJoinMidGame, adds the warmup widget

	UFUNCTION(Server, Reliable)
	void ServerRequestServerTime(const float TimeOfClientRequest); //Gets current server time

	//Gets server time and synchronizes the client time while taking lag into consideration
	UFUNCTION(Client, Reliable)
	void ClientReportServerTime(const float TimeOfClientRequest, const float TimeServerReceivedClientRequest);

	//Calls the game mode to handle the message delivery to other players
	UFUNCTION(Server, Reliable)
	void ServerChatCommitted(const FText& Text, const FString& PlayerName);
	
	void PollInit();

	float ClientServerDelta{0.f}; //Difference of the client and the server time
	
	UPROPERTY(EditAnywhere, Category = Time)
	float TimeSyncFrequency{5.f}; //How often should server and client time be synchronized
	
	float TimeSyncRunningTime{0.f};//0 to TimeSyncFrequency back to 0

	void HighPingWarning(); //If ping is too high, display HighPingImage and play the animation
	void StopHighPingWarning(); //Stops the ping warning

	UFUNCTION(Server, Reliable)
	void ServerReportPingStatus(bool bHighPing); //Tells server if client ping is too high

	float HighPingRunningTime{0.f}; //0 to CheckPingFrequency back to 0
	UPROPERTY(EditAnywhere)
	float HighPingDuration{5.f}; //High ping animation duration
	float PingAnimRunningTime{0.f}; //0 to HighPingDuration back to 0
	UPROPERTY(EditAnywhere)
	float CheckPingFrequency{20.f}; //How often is ping checked
	UPROPERTY(EditAnywhere)
	float HighPingThreshold{85.f}; //What is considered "high" ping

	//Calls AddElimAnnouncement with different parameters, depending on the passed in player states
	UFUNCTION(Client, Reliable)
	void ClientElimAnnouncement(APlayerState* Attacker, APlayerState* Victim);


	//Could have made an enum, but I think adding an enum for every little thing might cause problems if the project gets bigger
	void PlayOrbAnnouncementSound(const int32 SoundNumber) const;

	UPROPERTY(ReplicatedUsing = OnRep_ShowTeamScores)
	bool bShowTeamScores = false;

	UFUNCTION()
	void OnRep_ShowTeamScores(); //Calls InitTeamScores or HideTeamScores

	//Sets Warmup widget TopPlayer text
	FString GetInfoText(const TArray<class ABlasterPlayerState*>& PlayerStates);
	//Returns a FString based on which team won
	FString GetTeamsInfoText(class ABlasterGameState const * BlasterGameState);

private:

	UPROPERTY(EditAnywhere, Category = HUD)
	TSubclassOf<class UUserWidget> ReturnToMainMenuClass; //"Pause" menu class

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
	
	uint32 CountdownInt{0}; //Used in SetHUDTime

	UPROPERTY(ReplicatedUsing = OnRep_MatchState)
	FName MatchState{FName()}; //Current Match state

	UFUNCTION()
	void OnRep_MatchState(); //Calls HandleMatchHasStarted or HandleCooldown based on current match state

	//Update necessary values with passed in values.
	UFUNCTION(Client, Reliable)
	void ClientJoinMidGame(FName InMatchState, float InWarmup, float InMatchTime, float InStartingTime, float InCooldownTime);

	UPROPERTY()
	class UCharacterOverlay* CharacterOverlay;

	bool bInitializeCharacterOverlay{false};

	UPROPERTY(EditAnywhere)
	TSubclassOf<class UChatBox> ChatBoxClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UChatBox* ChatBox;

	//SoundCues for CTF game mode
	UPROPERTY(EditAnywhere)
	class USoundCue* YouHaveTheOrb;

	UPROPERTY(EditAnywhere)
	USoundCue* EnemyHasTheOrb;

	UPROPERTY(EditAnywhere)
	USoundCue* EnemyHasScored;

	UPROPERTY(EditAnywhere)
	USoundCue* YouHaveScored;

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
