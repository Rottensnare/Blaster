// Fill out your copyright notice in the Description page of Project Settings.


#include "BuffComponent.h"

#include "Blaster/Character/BlasterCharacter.h"

// Sets default values for this component's properties
UBuffComponent::UBuffComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UBuffComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}




void UBuffComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	HealRampUp(DeltaTime);
	
}

void UBuffComponent::Heal(float InHealAmount, float InHealTime)
{
	bHealing = true;
	HealingRate = InHealAmount / InHealTime;
	AmountToHeal += InHealAmount;
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
