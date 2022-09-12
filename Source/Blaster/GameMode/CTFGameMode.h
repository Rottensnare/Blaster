// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TeamsGameMode.h"
#include "CTFGameMode.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API ACTFGameMode : public ATeamsGameMode
{
	GENERATED_BODY()

public:

	virtual void PlayerEliminated(ABlasterCharacter* EliminatedCharacter, ABlasterPlayerController* VictimPlayerController, ABlasterPlayerController* AttackerController) override;
	void FlagCaptured(class AOrb* InOrb, class AOrbZone* InOrbZone);

protected:
	void HandleCTFStart();
	virtual void BeginPlay() override;
	UFUNCTION()
	void FlagPickedUp(AOrb* PickedUpOrb);
	

private:

	UPROPERTY(BlueprintReadOnly, Category = CTF, meta = (AllowPrivateAccess = "true"))
	AOrb* BlueOrb;

	UPROPERTY(BlueprintReadOnly, Category = CTF, meta = (AllowPrivateAccess = "true"))
	AOrb* RedOrb;

	UPROPERTY(VisibleAnywhere)
	class AOrbSpawnPoint* RedOrbSpawnPoint;

	UPROPERTY(VisibleAnywhere)
	AOrbSpawnPoint* BlueOrbSpawnPoint;

public:

	FORCEINLINE void SetRedOrb(AOrb* InRedOrb) {RedOrb = InRedOrb;}
	FORCEINLINE void SetBlueOrb(AOrb* InBlueOrb) {BlueOrb = InBlueOrb;}
	
};
