// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickup.h"
#include "Blaster/Weapon/WeaponType.h"
#include "AmmoPickup.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API AAmmoPickup : public APickup
{
	GENERATED_BODY()

public:

	


protected:

	//Calls CombatComponent->PickupAmmo with member variable values
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult) override;


private:

	UPROPERTY(EditAnywhere)
	int32 AmmoAmount{30}; //How much ammo this pickup gives

	UPROPERTY(EditDefaultsOnly)
	EWeaponType WeaponType{EWeaponType::EWT_Pistol}; //For which weapon type this ammo is for

	
	
};
