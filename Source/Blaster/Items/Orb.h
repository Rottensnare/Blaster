// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blaster/GameMode/Teams.h"
#include "GameFramework/Actor.h"
#include "Orb.generated.h"



UCLASS()
class BLASTER_API AOrb : public AActor
{
	GENERATED_BODY()
	
public:	

	DECLARE_MULTICAST_DELEGATE_OneParam(FOnOrbPickedUp, AOrb*)
	
	AOrb();

	//Detaches orb from owner character. Plays DropSound, set collision back to QueryOnly
	UFUNCTION(NetMulticast, Reliable)
	void Dropped(const FVector& InLocation);

	//Broadcasts using OnOrbPickedUp delegate and sets collision to NoCollision
	void PickedUp();
	void SetMaterial(); //Set orb material based on team
	virtual void Destroyed() override; //Nada

	FOnOrbPickedUp OnOrbPickedUp; //Used by CTF game mode
	
protected:

	//Binds OnSphereOverlap to AreaSphere->OnComponentBeginOverlap
	virtual void BeginPlay() override;

	//Calls BlasterCharacter->ServerAttachOrb
	UFUNCTION()
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);

private:

	UPROPERTY(EditAnywhere)
	class UStaticMeshComponent* OrbMesh;

	UPROPERTY(EditDefaultsOnly)
	class USphereComponent* AreaSphere;

	UPROPERTY(EditAnywhere)
	ETeams Team;

	UPROPERTY(EditAnywhere)
	class USoundCue* PickupSound;

	UPROPERTY(EditAnywhere)
	USoundCue* DropSound;

	UPROPERTY()
	class USoundAttenuation* PickupAttenuation;

	UPROPERTY(EditAnywhere)
	UMaterialInstance* RedMaterial;

	UPROPERTY(EditAnywhere)
	UMaterialInstance* BlueMaterial;

	UPROPERTY()
	class ABlasterCharacter* OwningCharacter;

public:	

	FORCEINLINE ETeams GetTeam() const {return Team;}
	FORCEINLINE void SetTeam(const ETeams InTeam) {Team = InTeam;}
	FORCEINLINE ABlasterCharacter* GetOwningBlasterCharacter() const {return OwningCharacter;}
	FORCEINLINE void SetOwningCharacter(ABlasterCharacter* const InBlasterCharacter) {OwningCharacter = InBlasterCharacter;}
	

};
