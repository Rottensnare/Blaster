// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickup.h"
#include "HealthPickup.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API AHealthPickup : public APickup
{
	GENERATED_BODY()

public:

	AHealthPickup();

protected:
	//Calls BuffComponent->Heal with member variable values
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult) override;


private:


	UPROPERTY(EditAnywhere)
	float HealAmount{75};

	UPROPERTY(EditAnywhere)
	float HealingTime{4.f};


};
