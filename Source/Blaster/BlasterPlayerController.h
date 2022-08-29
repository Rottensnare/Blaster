// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BlasterPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API ABlasterPlayerController : public APlayerController
{
	GENERATED_BODY()

public:

	void SetHUDHealth(float Health, float MaxHealth);
	virtual void OnPossess(APawn* InPawn) override;
	void SetHUDScore(float Score);
	void SetHUDElims(int32 Elims);
	void SetHUDAmmo(int32 Ammo);
	void SetHUDMagText(int32 MagAmmo);
	void SetHUDTotalAmmo(int32 TotalAmmo);
	void SetHUDWeaponType(FString WeaponType);
	void SetMatchTimeText(float MatchTime);
	void SetHUDAnnouncementTime(float WarmupTime);

	virtual float GetServerTime();
	virtual void ReceivedPlayer() override;
	void OnMatchStateSet(FName State);
	void HandleCooldown();
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
protected:

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	void SetHUDTime();
	void HandleMatchHasStarted();

	UFUNCTION(Server, Reliable)
	void ServerCheckMatchState();

	UFUNCTION(Server, Reliable)
	void ServerRequestServerTime(const float TimeOfClientRequest);

	UFUNCTION(Client, Reliable)
	void ClientReportServerTime(const float TimeOfClientRequest, const float TimeServerReceivedClientRequest);

	void PollInit();

	float ClientServerDelta{0.f};
	
	UPROPERTY(EditAnywhere, Category = Time)
	float TimeSyncFrequency{5.f};
	
	float TimeSyncRunningTime{0.f};

private:
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

	//Cached values that will be used to set HUD element values when Overlay is initialized.
	//Overlay will be initialized late so we need to do this.
	float HUDHealth;
	float HUDMaxHealth;
	float HUDScore;
	float HUDElims;
};
