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
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void Heal(float InHealAmount, float InHealTime);
	void ReplenishShields(float InShieldAmount, float InRate);
	void BuffSpeed(float InBuffBaseSpeed, float InBuffCrouchSpeed, float InBuffTime);
	void BuffJump(float InBuffJumpVelocity, float InBuffTime);
	void SetInitialSpeeds(float BaseSpeed, float CrouchSpeed);
	void SetInitialJumpVelocity(float InVelocity);
	

protected:

	virtual void BeginPlay() override;
	void HealRampUp(float DeltaTime);
	void ShieldRampUp(float DeltaTime);


private:

	UPROPERTY()
	class ABlasterCharacter* Character;

	bool bHealing{false};
	float HealingRate{0.f};
	float AmountToHeal{0.f};

	bool bShielding{false};
	float ShieldRate{0.f};
	float AmountToShield{0.f};

	FTimerHandle SpeedBuffTimer;
	void ResetSpeed();
	float InitialBaseSpeed;
	float InitialCrouchSpeed;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastSpeedBuff(float InBaseSpeed, float InCrouchSpeed);

	FTimerHandle JumpBuffTimer;
	void ResetJump();
	float InitialJumpVelocity;
	
	UFUNCTION(NetMulticast, Reliable)
	void MulticastJumpBuff(float InJumpVelocity);

public:	

	

		
};
