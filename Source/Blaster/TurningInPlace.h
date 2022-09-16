#pragma once

//Used for determining which way to turn or if not to turn at all
UENUM(BlueprintType)
enum class ETurningInPlace : uint8 
{
	ETIP_Left UMETA(DisplayName = "Turning Left"),
	ETIP_Right UMETA(DisplayName = "Turning Right"),
	ETIP_NotTurning UMETA(DisplayName = "Not Turning"),

	ETIP_MAX UMETA(DisplayName = "DefaultMAX")
};
