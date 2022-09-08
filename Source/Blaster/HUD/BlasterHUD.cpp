// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterHUD.h"

#include "Announcement.h"
#include "CharacterOverlay.h"
#include "ElimAnnouncement.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"

void ABlasterHUD::DrawHUD()
{
	Super::DrawHUD();

	FVector2D ViewportSize;
	
	if(GEngine)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
		const FVector2D ViewportCenter(ViewportSize.X /2.f, ViewportSize.Y / 2.f);
		float SpreadScaled = CrosshairSpreadMax * CrosshairSpread;

		if(HUDPackage.CrosshairsCenter)
		{
			FVector2D Spread{0.f, 0.f};
			DrawCrosshair(HUDPackage.CrosshairsCenter, ViewportCenter, Spread);
		}
		if(HUDPackage.CrosshairsTop)
		{
			FVector2D Spread{0.f ,-SpreadScaled,};
			DrawCrosshair(HUDPackage.CrosshairsTop, ViewportCenter, Spread);
		}
		if(HUDPackage.CrosshairsBottom)
		{
			FVector2D Spread{0.f ,SpreadScaled,};
			DrawCrosshair(HUDPackage.CrosshairsBottom, ViewportCenter, Spread);
		}
		if(HUDPackage.CrosshairsLeft)
		{
			FVector2D Spread{-SpreadScaled, 0.f};
			DrawCrosshair(HUDPackage.CrosshairsLeft, ViewportCenter, Spread);
		}
		if(HUDPackage.CrosshairsRight)
		{
			FVector2D Spread{SpreadScaled, 0.f};
			DrawCrosshair(HUDPackage.CrosshairsRight, ViewportCenter, Spread);
		}
		
	}
}

void ABlasterHUD::BeginPlay()
{
	Super::BeginPlay();
	
}

void ABlasterHUD::AddCharacterOverlay()
{
	APlayerController* PlayerController = GetOwningPlayerController();
	if(PlayerController && CharacterOverlayClass)
	{
		BlasterOverlay = CreateWidget<UCharacterOverlay>(PlayerController, CharacterOverlayClass);
		BlasterOverlay->AddToViewport();
	}
}

void ABlasterHUD::AddWarmupWidget()
{
	APlayerController* PlayerController = GetOwningPlayerController();
	if(PlayerController && WarmupWidgetClass)
	{
		WarmupWidget = CreateWidget<UAnnouncement>(PlayerController, WarmupWidgetClass);
		WarmupWidget->AddToViewport();
	}
}

void ABlasterHUD::AddElimAnnouncement(FString Attacker, FString Victim)
{
	OwningPlayerController = OwningPlayerController == nullptr ? GetOwningPlayerController() : OwningPlayerController;
	if(OwningPlayerController && ElimAnnouncementClass)
	{
		UElimAnnouncement* ElimAnnouncementWidget = CreateWidget<UElimAnnouncement>(OwningPlayerController, ElimAnnouncementClass);
		if(ElimAnnouncementWidget)
		{
			ElimAnnouncementWidget->SetElimAnnouncementText(Attacker, Victim);
			ElimAnnouncementWidget->AddToViewport();
			for(auto Msg : ElimMessages)
			{
				if(Msg && Msg->AnnouncementBox)
				{
					UCanvasPanelSlot* CanvasSlot = UWidgetLayoutLibrary::SlotAsCanvasSlot(Msg->AnnouncementBox);
					if(CanvasSlot)
					{
						FVector2D Position = CanvasSlot->GetPosition();
						FVector2D NewPosition(CanvasSlot->GetPosition().X, Position.Y - CanvasSlot->GetSize().Y);
						CanvasSlot->SetPosition(NewPosition);
					}
				}
			}

			
			ElimMessages.Add(ElimAnnouncementWidget);

			FTimerHandle ElimMsgTimer;
			FTimerDelegate ElimMsgDelegate;
			ElimMsgDelegate.BindUFunction(this, FName("ElimAnnouncementTimerFinished"), ElimAnnouncementWidget);
			GetWorldTimerManager().SetTimer(ElimMsgTimer, ElimMsgDelegate, ElimAnnouncementTime, false);
		}
	}
}

void ABlasterHUD::DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread)
{
	const float TextureWidth = Texture->GetSizeX();
	const float TextureHeight = Texture->GetSizeY();
	const FVector2D TextureDrawpoint(
		ViewportCenter.X - (TextureWidth / 2.f) + Spread.X,
		ViewportCenter.Y - (TextureHeight / 2.f)+ Spread.Y);
	DrawTexture(Texture, TextureDrawpoint.X, TextureDrawpoint.Y, TextureWidth, TextureHeight, 0.f, 0.f, 1.f, 1.f, CrosshairColor);
}

void ABlasterHUD::ElimAnnouncementTimerFinished(UElimAnnouncement* MsgToRemove)
{
	if(MsgToRemove)
	{
		MsgToRemove->RemoveFromParent();
	}
}
