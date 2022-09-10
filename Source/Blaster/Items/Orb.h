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

	AOrb();
	void Dropped(const FVector& InLocation);
	void PickedUp();

protected:

	virtual void BeginPlay() override;

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

public:	

	

};