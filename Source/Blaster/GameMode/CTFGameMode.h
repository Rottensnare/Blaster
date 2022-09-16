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
	//If orb carrier killed, calls the necessary drop functions
	virtual void PlayerEliminated(ABlasterCharacter* EliminatedCharacter, ABlasterPlayerController* VictimPlayerController, ABlasterPlayerController* AttackerController) override;

	//Adds score to the correct team. Calls ClienOrbAnnouncements for all player controllers.
	//Drops the orb and destroys it. Calls SpawnOrb and binds FlagPickedUp to the spawned orb.
	void FlagCaptured(class AOrb* InOrb, class AOrbZone* InOrbZone);

protected:
	
	//Gets all Orb spawn points with GetAllActorsOfClass and sets the team orb spawn points
	//Calls SpawnOrb for both team's orbs. Binds FlagPickedUp to OnOrbPickedUp
	void HandleCTFStart();
	
	virtual void BeginPlay() override; //Calls HandleCTFStart
	
	//Iterates through all the player controllers and calls ClientOrbAnnouncement with different parameters
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

	FORCEINLINE void SetRedOrb(AOrb* InRedOrb) {RedOrb = InRedOrb;} //Called from the OrbSpawnPoint class
	FORCEINLINE void SetBlueOrb(AOrb* InBlueOrb) {BlueOrb = InBlueOrb;}
	
};
