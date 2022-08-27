#pragma once

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	EWT_AssaultRifle UMETA(DisplayName = "A Salt Rifle"),
	EWT_SubmachineGun UMETA(DisplayName = "Submachine Gun"),
	EWT_RocketLauncher UMETA(DisplayName = "RocketLauncher"),
	
	EWT_MAX UMETA(DisplayName  = "DefaultMAX")
};