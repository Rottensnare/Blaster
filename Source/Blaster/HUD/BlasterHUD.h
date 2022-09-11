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
	UPROPERTY()
	class UTexture2D* CrosshairsCenter;
	UPROPERTY()
	UTexture2D* CrosshairsBottom;
	UPROPERTY()
	UTexture2D* CrosshairsTop;
	UPROPERTY()
	UTexture2D* CrosshairsLeft;
	UPROPERTY()
	UTexture2D* CrosshairsRight;
	float CrosshairSpread;
};

UCLASS()
class BLASTER_API ABlasterHUD : public AHUD
{
	GENERATED_BODY()

public:

	virtual void DrawHUD() override;
	void AddCharacterOverlay();
	void AddWarmupWidget();
	void AddElimAnnouncement(FString Attacker, FString Victim);
	
	UPROPERTY(EditAnywhere, Category = "Player Status")
	TSubclassOf<class UUserWidget> CharacterOverlayClass;

	UPROPERTY(EditAnywhere, Category = "Announcements")
	TSubclassOf<UUserWidget> WarmupWidgetClass;

	UPROPERTY()
	class UCharacterOverlay* BlasterOverlay;

	UPROPERTY()
	class UAnnouncement* WarmupWidget;

	


protected:

	virtual void BeginPlay() override;

private:

	UPROPERTY()
	class APlayerController* OwningPlayerController;

	FHUDPackage HUDPackage;

	void DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread);
	
	UPROPERTY(EditAnywhere)
	float CrosshairSpreadMax{16.f};
	float CrosshairSpread;

	FLinearColor CrosshairColor{FLinearColor::White};

	UPROPERTY(EditAnywhere)
	TSubclassOf<class UElimAnnouncement> ElimAnnouncementClass;

	UPROPERTY(EditAnywhere)
	float ElimAnnouncementTime = 3.f;

	UFUNCTION()
	void ElimAnnouncementTimerFinished(UElimAnnouncement* MsgToRemove);

	UPROPERTY()
	TArray<UElimAnnouncement*> ElimMessages;
	bool bDrawHUD{true};

	
public:

	FORCEINLINE void SetHUDPackage(const FHUDPackage& Package) {HUDPackage = Package;}
	FORCEINLINE void SetCrosshairSpread(const float Spread) {CrosshairSpread = Spread;}
	FORCEINLINE void SetCrosshairColor(FLinearColor Color) {CrosshairColor = Color;}

	UFUNCTION(Client, Reliable)
	void SetDrawHUD(bool bInDrawHUD);
};

inline void ABlasterHUD::SetDrawHUD_Implementation(bool bInDrawHUD)
{bDrawHUD = bInDrawHUD;}
