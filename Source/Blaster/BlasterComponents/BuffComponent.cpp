// Fill out your copyright notice in the Description page of Project Settings.


#include "BuffComponent.h"

#include "CombatComponent.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"


UBuffComponent::UBuffComponent()
{

	PrimaryComponentTick.bCanEverTick = true;


}



void UBuffComponent::BeginPlay()
{
	Super::BeginPlay();
	
}




void UBuffComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	HealRampUp(DeltaTime);
	ShieldRampUp(DeltaTime);
	
}

void UBuffComponent::Heal(float InHealAmount, float InHealTime)
{
	bHealing = true;
	HealingRate = InHealAmount / InHealTime;
	AmountToHeal += InHealAmount;
}

void UBuffComponent::ReplenishShields(float InShieldAmount, float InRate)
{
	bShielding = true;
	ShieldRate = InShieldAmount / InRate;
	AmountToShield += InShieldAmount;
}

void UBuffComponent::BuffSpeed(float InBuffBaseSpeed, float InBuffCrouchSpeed, float InBuffTime)
{
	if(Character == nullptr) return;

	Character->GetWorldTimerManager().SetTimer(SpeedBuffTimer, this, &UBuffComponent::ResetSpeed, InBuffTime);

	MulticastSpeedBuff(InBuffBaseSpeed, InBuffCrouchSpeed);

	Character->GetCharacterMovement()->MaxWalkSpeed = InBuffBaseSpeed;
	Character->GetCharacterMovement()->MaxWalkSpeedCrouched = InBuffCrouchSpeed;
	if(Character->GetCombatComponent())
	{
		Character->GetCombatComponent()->SetSpeeds(InBuffCrouchSpeed, InBuffBaseSpeed);
	}
	
}

void UBuffComponent::BuffJump(float InBuffJumpVelocity, float InBuffTime)
{
	if(Character == nullptr) return;

	Character->GetWorldTimerManager().SetTimer(JumpBuffTimer, this, &UBuffComponent::ResetJump, InBuffTime);
	Character->GetCharacterMovement()->JumpZVelocity = InBuffJumpVelocity;

	MulticastJumpBuff(InBuffJumpVelocity);

}

void UBuffComponent::SetInitialSpeeds(float BaseSpeed, float CrouchSpeed)
{
	InitialBaseSpeed = BaseSpeed;
	InitialCrouchSpeed = CrouchSpeed;
}

void UBuffComponent::SetInitialJumpVelocity(float InVelocity)
{
	InitialJumpVelocity = InVelocity;
}

void UBuffComponent::HealRampUp(float DeltaTime)
{
	if(!bHealing || Character == nullptr || Character->IsEliminated()) return;

	const float HealThisFrame = HealingRate * DeltaTime;
	Character->SetHealth(FMath::Clamp( Character->GetHealth() + HealThisFrame, 0.f, Character->GetMaxHealth()));
	AmountToHeal -= HealThisFrame;
	if(Character->HasAuthority())
	{
		Character->UpdateHUDHealth();
	}
	if(AmountToHeal <= 0.f || Character->GetHealth() >= Character->GetMaxHealth())
	{
		bHealing = false;
		AmountToHeal = 0.f;
	}
}

void UBuffComponent::ShieldRampUp(float DeltaTime)
{
	if(!bShielding || Character == nullptr || Character->IsEliminated()) return;

	const float ShieldThisFrame = ShieldRate * DeltaTime;
	Character->SetShields(FMath::Clamp( Character->GetShields() + ShieldThisFrame, 0.f, Character->GetMaxShields()));
	AmountToShield -= ShieldThisFrame;
	if(Character->HasAuthority())
	{
		Character->UpdateHUDShields();
	}
	if(AmountToShield <= 0.f || Character->GetShields() >= Character->GetMaxShields())
	{
		bShielding = false;
		AmountToShield = 0.f;
	}
}

void UBuffComponent::ResetSpeed()
{
	if(Character == nullptr || !Character->GetCharacterMovement()) return;
	
	Character->GetCharacterMovement()->MaxWalkSpeed = InitialBaseSpeed;
	Character->GetCharacterMovement()->MaxWalkSpeedCrouched = InitialCrouchSpeed;
	if(Character->GetCombatComponent())
	{
		Character->GetCombatComponent()->SetSpeeds(InitialCrouchSpeed, InitialBaseSpeed);
	}

	MulticastSpeedBuff(InitialBaseSpeed, InitialCrouchSpeed);
}

void UBuffComponent::ResetJump()
{
	if(Character == nullptr) return;
	Character->GetCharacterMovement()->JumpZVelocity = InitialJumpVelocity;
	MulticastJumpBuff(InitialJumpVelocity);
}

void UBuffComponent::MulticastJumpBuff_Implementation(float InJumpVelocity)
{
	if(Character == nullptr || Character->HasAuthority()) return;
	Character->GetCharacterMovement()->JumpZVelocity = InJumpVelocity;
}

void UBuffComponent::MulticastSpeedBuff_Implementation(float InBaseSpeed, float InCrouchSpeed)
{
	if(Character == nullptr) return;
	if(!Character->HasAuthority())
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = InBaseSpeed;
		Character->GetCharacterMovement()->MaxWalkSpeedCrouched = InCrouchSpeed;
		if(Character->GetCombatComponent())
		{
			Character->GetCombatComponent()->SetSpeeds(InCrouchSpeed, InBaseSpeed);
		}
	}
	
}
