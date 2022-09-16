// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "BlasterHUD.generated.h"

//Holds the crosshair textures
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

	//Gets viewport center and calls DrawCrosshair for every crosshair texture with a spread value included.
	virtual void DrawHUD() override;
	//Creates and adds the character overlay to the viewport
	void AddCharacterOverlay();
	//Creates and adds the Warmup widget to the viewport
	void AddWarmupWidget();
	
	//Creats an ElimAnnouncement widget and adds it to the viewport.
	//Calls ElimAnnouncementWidget->SetElimAnnouncementText with the Attacker and Victim values.
	//Adjust location if there are multiple widgets currently visible
	//Binds a function to a timer, so that the widget can be destroyed after few seconds.
	void AddElimAnnouncement(FString Attacker, FString Victim);
	
	UPROPERTY(EditAnywhere, Category = "Player Status")
	TSubclassOf<class UUserWidget> CharacterOverlayClass;

	UPROPERTY(EditAnywhere, Category = "Announcements")
	TSubclassOf<UUserWidget> WarmupWidgetClass; //Warmup widget shows warmup time and text before match is in progress

	UPROPERTY()
	class UCharacterOverlay* BlasterOverlay;

	UPROPERTY()
	class UAnnouncement* WarmupWidget;

	


protected:

	virtual void BeginPlay() override;

private:

	UPROPERTY()
	class APlayerController* OwningPlayerController;

	FHUDPackage HUDPackage; //Contains crosshair textures

	//Draws crosshair textures based on the passed in values and texture size
	void DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread);
	
	UPROPERTY(EditAnywhere)
	float CrosshairSpreadMax{16.f}; //Maximum amount crosshairs can spread
	float CrosshairSpread; //Current crosshair spread amount

	FLinearColor CrosshairColor{FLinearColor::White}; //Will be red if aiming at a player

	UPROPERTY(EditAnywhere)
	TSubclassOf<class UElimAnnouncement> ElimAnnouncementClass;

	UPROPERTY(EditAnywhere)
	float ElimAnnouncementTime = 3.f; //How long the elimination announcement widget stays on screen

	UFUNCTION()
	void ElimAnnouncementTimerFinished(UElimAnnouncement* MsgToRemove); //Removes MsgToRemove from the viewport

	UPROPERTY()
	TArray<UElimAnnouncement*> ElimMessages;
	
	bool bDrawHUD{true}; //Prevents a crash that happens when restarting level in a packaged game

	
public:

	FORCEINLINE void SetHUDPackage(const FHUDPackage& Package) {HUDPackage = Package;} //Called from CombatComponent::SetCrosshairs
	FORCEINLINE void SetCrosshairSpread(const float Spread) {CrosshairSpread = Spread;} //Called from CombatComponent::SetCrosshairsSpread
	FORCEINLINE void SetCrosshairColor(FLinearColor Color) {CrosshairColor = Color;} //Called from CombatComponent::TraceUnderCrosshairs

	UFUNCTION(Client, Reliable)
	void SetDrawHUD(bool bInDrawHUD);
};

inline void ABlasterHUD::SetDrawHUD_Implementation(bool bInDrawHUD)
{bDrawHUD = bInDrawHUD;}
