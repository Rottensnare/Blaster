// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LagCompensationComponent.generated.h"

//Used to keep characters hitbox information for server side rewind.
//These boxes are only used for server side rewind.
USTRUCT(BlueprintType)
struct FBoxInformation
{
	GENERATED_BODY()

	UPROPERTY()
	FVector BoxLocation;
	UPROPERTY()
	FRotator BoxRotation;
	UPROPERTY()
	FVector BoxExtent;
	
};

//Used to store information about a hitboxes transform at a certain point in time
USTRUCT(BlueprintType)
struct FFramePackage
{
	GENERATED_BODY()
	
	UPROPERTY()
	float Time;

	UPROPERTY()
	TMap<FName, FBoxInformation> HitBoxInfo;

	UPROPERTY()
	class ABlasterCharacter* BlasterCharacter;
	
};

//Used to check if hit was successful and if it was a headshot
USTRUCT(BlueprintType)
struct FServerSideRewindResult
{
	GENERATED_BODY()

	UPROPERTY()
	bool bHitConfirmed;

	UPROPERTY()
	bool bHeadShot;
	
};

//Stores number of hits to each character hit and also tracks number of headshots
USTRUCT(BlueprintType)
struct FShotgunServerSideRewindResult
{
	GENERATED_BODY()

	UPROPERTY()
	TMap<ABlasterCharacter*, uint32> HeadShots;
	UPROPERTY()
	TMap<ABlasterCharacter*, uint32> BodyShots;
	
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BLASTER_API ULagCompensationComponent : public UActorComponent
{
	GENERATED_BODY()

public:	

	ULagCompensationComponent();
	friend class ABlasterCharacter;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	//Draws a debug box for each hitbox inside the passed Frame Package. Extreme performance hit. Only for debugging purposes.
	void ShowFramePackage(const FFramePackage& Package,const FColor& Color);
	
	//Main function for the server side rewind functionality
	FServerSideRewindResult ServerSideRewind(ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, const float HitTime);

	//Simply put, it is similar to the ServerSideRewind, but iterates over HitCharacters and HitLocations
	FShotgunServerSideRewindResult ShotgunServerSideRewind(const TArray<ABlasterCharacter*>& HitCharacters, const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations, const float HitTime);
	
	//Called by the clients weapon. Calls ServerSideRewind and if hit was successful, applies damage.
	UFUNCTION(Server, Reliable)
	void ServerScoreRequest(ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, const float HitTime, class AWeapon* DamageCauser);

	UFUNCTION(Server, Reliable)
	void ServerShotgunScoreRequest(const TArray<ABlasterCharacter*>& HitCharacters, const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations, const float HitTime, AWeapon* DamageCauser);
	
protected:

	virtual void BeginPlay() override;
	
	void SaveFramePackage();
	
	//Saves hitbox transforms for a given time on the server and adds it to a frame package
	void SaveFramePackage(FFramePackage& OutPackage);
	
	//Interpolates between frames to check if Character was hit between the saved FramePackages.
	FFramePackage InterpBetweenFrames(const FFramePackage& OlderFrame, const FFramePackage& YoungerFrame, const float HitTime);

	//Checks with line traces whether or not client hit the player at their specified place in time
	FServerSideRewindResult ConfirmHit(const FFramePackage& Package, ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation);

	FShotgunServerSideRewindResult ShotgunConfirmHit(const TArray<FFramePackage>& Packages, const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations);

	//Saves a FramePackage so that after moving the hitboxes they can be put back to their original places.
	void CacheBoxTransform(ABlasterCharacter* HitCharacter, FFramePackage& OutFramePackage);
	
	//Moves the characters hitboxes back in time to the time when the client claims to have hit the player, so that a Line Trace can be performed with the hitboxes.
	void MoveHitBoxes(ABlasterCharacter* HitCharacter, const FFramePackage& Package);
	
	//Resets the HitBoxes back to their original position and disables collision for them.
	void ResetHitBoxes(ABlasterCharacter* HitCharacter, const FFramePackage& Package);
	
	//Handles Enabling and Disabling of the character mesh collision, so that it wont interfere with the Line Trace. Will set collision to default value afterwards.
	void EnableCharacterMeshCollision(ABlasterCharacter* HitCharacter, ECollisionEnabled::Type CollisionEnabled);

	//In short, tries to find 2 FramePackages that are on both sides of the hit time, sends those 2 packages to InterpBetweenFrames and then returns the interpolated FramePackage
	FFramePackage GetFrameToCheck(ABlasterCharacter* HitCharacter, const float HitTime);


	
	
private:

	UPROPERTY()
	ABlasterCharacter* BlasterCharacter;

	UPROPERTY()
	class ABlasterPlayerController* BlasterController;

	//List that holds all the necessary FramePackages and has easy access to the tail and head of the list for easy adding and removing of packages.
	TDoubleLinkedList<FFramePackage> FrameHistory;

	//Over what period of time are FramePackages kept. Longer time helps high ping players, but causes 'Dying behind cover' problems.
	UPROPERTY(EditAnywhere)
	float MaxRecordTime{4.f};

	
public:	

	

		
};
