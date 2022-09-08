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
	void AddCharacterOverlay();
	void AddWarmupWidget();
	void AddElimAnnouncement(FString Attacker, FString Victim);
	void AddChatBox();
	void ToggleChatBox();
	
	UFUNCTION()
	void OnChatCommitted(const FText& Text, ETextCommit::Type CommitMethod);
	
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

	UPROPERTY(EditAnywhere)
	TSubclassOf<class UChatBox> ChatBoxClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UChatBox* ChatBox;
	
public:

	FORCEINLINE void SetHUDPackage(const FHUDPackage& Package) {HUDPackage = Package;}
	FORCEINLINE void SetCrosshairSpread(const float Spread) {CrosshairSpread = Spread;}
	FORCEINLINE void SetCrosshairColor(FLinearColor Color) {CrosshairColor = Color;}
	
};
