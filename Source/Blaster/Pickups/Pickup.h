// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Pickup.generated.h"

UCLASS()
class BLASTER_API APickup : public AActor
{
	GENERATED_BODY()
	
public:	

	APickup();
	virtual void Tick(float DeltaTime) override; //Adds local rotation to the pickup
	virtual void Destroyed() override; //Plays sound and spawns a particle effect at actor location for all players

protected:
	
	virtual void BeginPlay() override; //Starts OverlapBeginTimer

	//Calls Destroy
	UFUNCTION()
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);
	//Does nada
	UFUNCTION()
	void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UPROPERTY(VisibleAnywhere)
	class UNiagaraComponent* PickupEffectComponent;

	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* PickedUpEffect;


private:

	//Checks for overlap events
	UPROPERTY(EditAnywhere)
	class USphereComponent* OverlapSphere;

	UPROPERTY(EditAnywhere)
	class USoundCue* PickupSound;

	UPROPERTY()
	class USoundAttenuation* PickupAttenuation;

	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* PickupMesh;

	//Needed because if a character is standing inside a pickup spawn, the pickup is destroyed so fast that the respawn function won't get bound.
	FTimerHandle OverlapBeginTimer;
	float OverlapBeginTime{0.2f};
	//Binds OnSphereOverlap to OnComponentBeginOverlap
	void OverlapBeginTimerFinished();


	UPROPERTY(EditAnywhere)
	float BaseTurnRate{45.f}; //Rotation rate for the pickup
	

public:	
	
	

};


