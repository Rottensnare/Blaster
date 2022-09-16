#pragma once

#define TRACE_LENGTH 5000.f

#define CUSTOM_DEPTH_RED 247
#define CUSTOM_DEPTH_GREEN 248
#define CUSTOM_DEPTH_ORANGE 249
#define CUSTOM_DEPTH_PURPLE 250
#define  CUSTOM_DEPTH_BLUE 251
#define CUSTOM_DEPTH_TAN 252

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	EWT_AssaultRifle UMETA(DisplayName = "A Salt Rifle"),
	EWT_SubmachineGun UMETA(DisplayName = "Submachine Gun"),
	EWT_RocketLauncher UMETA(DisplayName = "RocketLauncher"),
	EWT_Pistol UMETA(DisplayName = "Pistol"),
	EWT_Shotgun UMETA(DisplayName = "Shotgun"),
	EWT_SniperRifle UMETA(DisplayName = "Shnaipier"),
	EWT_GrenadeLauncher UMETA(DisplayName = "Grenade Launcher"),
	
	EWT_MAX UMETA(DisplayName  = "DefaultMax")
};