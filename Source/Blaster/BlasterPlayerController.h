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

	virtual float GetServerTime();
	virtual void ReceivedPlayer() override;
	

protected:

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	void SetHUDTime();

	UFUNCTION(Server, Reliable)
	void ServerRequestServerTime(const float TimeOfClientRequest);

	UFUNCTION(Client, Reliable)
	void ClientReportServerTime(const float TimeOfClientRequest, const float TimeServerReceivedClientRequest);

	float ClientServerDelta{0.f};
	
	UPROPERTY(EditAnywhere, Category = Time)
	float TimeSyncFrequency{5.f};
	
	float TimeSyncRunningTime{0.f};

private:
	UPROPERTY()
	class ABlasterHUD* BlasterHUD;

	float MatchTime{120.f};
	uint32 CountdownInt{0};
};
