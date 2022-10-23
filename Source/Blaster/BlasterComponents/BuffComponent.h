// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BuffComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BLASTER_API UBuffComponent : public UActorComponent
{
	GENERATED_BODY()

public:	

	UBuffComponent();
	friend class ABlasterCharacter;
	//Calls HealRampUp and ShieldRampUp
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	//Sets healing variables
	void Heal(float InHealAmount, float InHealTime);
	//Sets shield variables
	void ReplenishShields(float InShieldAmount, float InRate);
	//Starts the SpeedBuffTimer, calls multicast version, changes max walking speeds
	void BuffSpeed(float InBuffBaseSpeed, float InBuffCrouchSpeed, float InBuffTime);
	//Starts JumpBuffTimer, increases character JumpZVelocity, calls multicast version
	void BuffJump(float InBuffJumpVelocity, float InBuffTime);
	//Sets initial speeds for future reference 
	void SetInitialSpeeds(float BaseSpeed, float CrouchSpeed);
	//Sets initial jump velocity for future reference
	void SetInitialJumpVelocity(float InVelocity);
	

protected:

	virtual void BeginPlay() override; //Nada
	//Heals character, decreases AmountToHeal, calls Character->UpdateHUDHealth()
	void HealRampUp(float DeltaTime);
	//Same  as HealRampUp but for shields
	void ShieldRampUp(float DeltaTime);


private:

	UPROPERTY()
	class ABlasterCharacter* Character;

	bool bHealing{false};
	float HealingRate{0.f}; //How much healing per frame
	float AmountToHeal{0.f}; //Total amount of healing

	bool bShielding{false};
	float ShieldRate{0.f}; //How much shielding per frame
	float AmountToShield{0.f}; //Total amount of shielding 

	FTimerHandle SpeedBuffTimer; //Timer for speed buff
	void ResetSpeed(); //Sets character movement speed back to defaults. Calls multicast version and CombatComponent->SetSpeeds
	float InitialBaseSpeed; //Holds character base speed for future reference
	float InitialCrouchSpeed; //Holds character base crouch speed for future reference

	//Same as the server version but for clients
	UFUNCTION(NetMulticast, Reliable)
	void MulticastSpeedBuff(float InBaseSpeed, float InCrouchSpeed);

	FTimerHandle JumpBuffTimer; //Timer for jump buff
	void ResetJump(); //Resets JumpZVelocity back to default, calls multicast version
	float InitialJumpVelocity; //Holds character base jump velocity for future reference

	//Same as the server version but for clients
	UFUNCTION(NetMulticast, Reliable)
	void MulticastJumpBuff(float InJumpVelocity);

public:	

	

		
};
