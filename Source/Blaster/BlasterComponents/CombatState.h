#pragma once

//Used to check if player can perform certain actions
UENUM(BlueprintType)
enum class ECombatState : uint8
{
	ECS_Unoccupied UMETA(DisplayName = "Unoccupied"),
	ECS_Reloading UMETA(DisplayName = "Reloading"),
	
	ECS_MAX UMETA(DisplayName = "DefaultMax")
};