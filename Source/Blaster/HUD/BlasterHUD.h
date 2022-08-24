// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "BlasterHUD.generated.h"


USTRUCT(BlueprintType)
struct FHUDPackage
{
	GENERATED_BODY()

public:
	class UTexture2D* CrosshairsCenter;
	UTexture2D* CrosshairsBottom;
	UTexture2D* CrosshairsTop;
	UTexture2D* CrosshairsLeft;
	UTexture2D* CrosshairsRight;
	float CrosshairSpread;
};

UCLASS()
class BLASTER_API ABlasterHUD : public AHUD
{
	GENERATED_BODY()

public:

	virtual void DrawHUD() override;

	UPROPERTY(EditAnywhere, Category = "Player Status")
	TSubclassOf<class UUserWidget> CharacterOverlayClass;
	
	class UCharacterOverlay* BlasterOverlay;

protected:

	virtual void BeginPlay() override;
	void AddCharacterOverlay();

private:

	FHUDPackage HUDPackage;

	void DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread);
	
	UPROPERTY(EditAnywhere)
	float CrosshairSpreadMax{16.f};
	float CrosshairSpread;

	FLinearColor CrosshairColor{FLinearColor::White};
public:

	FORCEINLINE void SetHUDPackage(const FHUDPackage& Package) {HUDPackage = Package;}
	FORCEINLINE void SetCrosshairSpread(const float Spread) {CrosshairSpread = Spread;}
	FORCEINLINE void SetCrosshairColor(FLinearColor Color) {CrosshairColor = Color;}
	
};
