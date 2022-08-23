// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatComponent.h"

#include "Blaster/BlasterPlayerController.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/HUD/BlasterHUD.h"
#include "Blaster/Weapon/Weapon.h"
#include "Components/SphereComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UCombatComponent::UCombatComponent():
BaseWalkSpeed(600.f),
AimWalkSpeed(350.f)
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

// Called when the game starts
void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	if(Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
	}
}

// Called every frame
void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	SetCrosshairsSpread(DeltaTime);
	if (Character && Character->IsLocallyControlled())
	{
		FHitResult HitResult;
		TraceUnderCrosshairs(HitResult);
		HitTarget = HitResult.ImpactPoint;
	}
}

void UCombatComponent::SetAiming(bool bIsAiming)
{
	bAiming = bIsAiming; //Set here so that we don't need to wait for the server to tell us to aim.
	ServerSetAiming(bIsAiming);
	if(Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	}
}

void UCombatComponent::ServerSetAiming_Implementation(bool bIsAiming)
{
	bAiming = bIsAiming;
	if(Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	}
}

void UCombatComponent::OnRep_EquippedWeapon()
{
	if(EquippedWeapon && Character)
	{
		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
		Character->bUseControllerRotationYaw = true;
	}
}

void UCombatComponent::FireButtonPressed(bool bPressed)
{
	bFireButtonPressed = bPressed;
	if(bFireButtonPressed)
	{
		FHitResult HitResult;
		TraceUnderCrosshairs(HitResult);
		ServerFire(HitResult.ImpactPoint);
	}
}

void UCombatComponent::TraceUnderCrosshairs(FHitResult& TraceHitResult)
{
	FVector2D ViewPortSize;
	if(GEngine)
	{
		GEngine->GameViewport->GetViewportSize(ViewPortSize);
	}

	FVector2D CrosshairLocation(ViewPortSize.X / 2.f, ViewPortSize.Y / 2.f);
	FVector CrosshairWorldPosition, CrosshairWorldDirection;
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0),
		CrosshairLocation,
		CrosshairWorldPosition,
		CrosshairWorldDirection);

	if(bScreenToWorld)
	{
		FVector Start{CrosshairWorldPosition};
		FVector End{Start + CrosshairWorldDirection * 10000.f};
		GetWorld()->LineTraceSingleByChannel(TraceHitResult, Start, End, ECollisionChannel::ECC_Visibility);
		if(!TraceHitResult.bBlockingHit)
		{
			TraceHitResult.ImpactPoint = End;
		}
	}
}

void UCombatComponent::SetCrosshairsSpread(float DeltaTime)
{
	if(Character == nullptr || BlasterHUD == nullptr) return;
	
	FVector2D WalkSpeedRange{0.f, Character->GetCharacterMovement()->MaxWalkSpeed};
	FVector2D VelocityMultiplierRange(0.f, 1.f);
	FVector Velocity = Character->GetVelocity();
	Velocity.Z = 0.f;
	
	CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange, VelocityMultiplierRange, Velocity.Size());

	if(Character->GetMovementComponent()->IsFalling())
	{
		CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 1.75f, DeltaTime, 10.f);
	}else
	{
		CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 0.f, DeltaTime, 10.f);
	}
	
	BlasterHUD->SetCrosshairSpread(CrosshairVelocityFactor + CrosshairInAirFactor);
}

void UCombatComponent::MulticastFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	if(EquippedWeapon == nullptr) return;
	if(Character)
	{
		Character->PlayFireMontage(bAiming);
		EquippedWeapon->Fire(TraceHitTarget);
		
	}
}

void UCombatComponent::ServerFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	MulticastFire(TraceHitTarget);
}

bool UCombatComponent::SetCrosshairs()
{
	if(Character == nullptr || Character->Controller == nullptr) return false;

	BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : BlasterPlayerController;
	if(BlasterPlayerController)
	{
		BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(BlasterPlayerController->GetHUD()) : BlasterHUD;
		if(BlasterHUD)
		{
			FHUDPackage HUDPackage;
			if (EquippedWeapon)
			{
				
				HUDPackage.CrosshairsCenter = EquippedWeapon->GetCrosshairTextures()[0];
				HUDPackage.CrosshairsTop = EquippedWeapon->GetCrosshairTextures()[1];
				HUDPackage.CrosshairsBottom = EquippedWeapon->GetCrosshairTextures()[2];
				HUDPackage.CrosshairsLeft = EquippedWeapon->GetCrosshairTextures()[3];
				HUDPackage.CrosshairsRight = EquippedWeapon->GetCrosshairTextures()[4];
			}else
			{
				HUDPackage.CrosshairsCenter = nullptr;
				HUDPackage.CrosshairsTop = nullptr;
				HUDPackage.CrosshairsBottom = nullptr;
				HUDPackage.CrosshairsLeft = nullptr;
				HUDPackage.CrosshairsRight = nullptr;
				
			}
			
			BlasterHUD->SetHUDPackage(HUDPackage);
			return true;
			
		}
	}
	return false;
}

void UCombatComponent::EquipWeapon(AWeapon* WeaponToEquip)
{
	if(Character == nullptr || WeaponToEquip == nullptr) return;

	EquippedWeapon = WeaponToEquip;
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	
	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("hand_rSocket"));
	if(HandSocket)
	{
		HandSocket->AttachActor(EquippedWeapon, Character->GetMesh());
	}
	EquippedWeapon->SetOwner(Character);

	Character->GetCharacterMovement()->bOrientRotationToMovement = false;
	Character->bUseControllerRotationYaw = true;


	SetCrosshairs();
	
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UCombatComponent, bAiming);
}


