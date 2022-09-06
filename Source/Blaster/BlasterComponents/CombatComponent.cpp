// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatComponent.h"

#include "Blaster/BlasterPlayerController.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/HUD/BlasterHUD.h"
#include "Blaster/Weapon/HitScanWeapon.h"
#include "Blaster/Weapon/Shotgun.h"
#include "Blaster/Weapon/Weapon.h"
#include "Camera/CameraComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Sound/SoundCue.h"

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

		if(Character->GetCameraComponent())
		{
			DefaultFOV = Character->GetCameraComponent()->FieldOfView;
			CurrentFOV = DefaultFOV;
		}
	}
	if(Character->HasAuthority())
	{
		InitializeCarriedAmmo();
	}
}

// Called every frame
void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if (Character && Character->IsLocallyControlled())
	{
		SetCrosshairsSpread(DeltaTime);
		
		FHitResult HitResult;
		TraceUnderCrosshairs(HitResult);
		HitTarget = HitResult.ImpactPoint;

		InterpFOV(DeltaTime);
	}
}

void UCombatComponent::SetAiming(bool bIsAiming)
{
	if(Character == nullptr || EquippedWeapon == nullptr) return;
	bAiming = bIsAiming; //Set here so that we don't need to wait for the server to tell us to aim.
	ServerSetAiming(bIsAiming);
	
	Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	if(Character->IsLocallyControlled() && EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SniperRifle)
	{
		Character->ShowSniperScopeWidget(bIsAiming);
		//BUG: Following code needs to be replicated
		if(bIsAiming) //BUG: Quickscoping is too effective, need to add a small delay before setting scatter usage
		{
			EquippedWeapon->SetUseScatter(false);
		}
		else
		{
			EquippedWeapon->SetUseScatter(true);
		}
	}
	bAimButtonPressed = bIsAiming; //For Locally Controlled Character
}

void UCombatComponent::ServerSetAiming_Implementation(bool bIsAiming)
{
	if(Character == nullptr || EquippedWeapon == nullptr) return;
	bAiming = bIsAiming;

	Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	if(EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SniperRifle && bIsAiming)
	{
		EquippedWeapon->SetUseScatter(false);
	}
	else if(EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SniperRifle && !bIsAiming)
	{
		EquippedWeapon->SetUseScatter(true);
	}
}

void UCombatComponent::OnRep_EquippedWeapon()
{
	if(EquippedWeapon && Character)
	{
		EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	
		const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("hand_rSocket"));
		if(HandSocket)
		{
			HandSocket->AttachActor(EquippedWeapon, Character->GetMesh());
		}
		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
		Character->bUseControllerRotationYaw = true;
		Character->GetWorldTimerManager().SetTimer(RefreshHUDTimer, this, &UCombatComponent::RefreshHUDTimerFinished, RefreshHUDDelay);
	}
}

void UCombatComponent::OnRep_SecondaryWeapon()
{
	if(SecondaryWeapon && Character)
	{
		SecondaryWeapon->SetWeaponState(EWeaponState::EWS_UnEquipped);
	
		const USkeletalMeshSocket* BackpackSocket = Character->GetMesh()->GetSocketByName(FName("BackpackSocket"));
		if(BackpackSocket)
		{
			BackpackSocket->AttachActor(SecondaryWeapon, Character->GetMesh());
		}
		Character->GetWorldTimerManager().SetTimer(RefreshHUDTimer, this, &UCombatComponent::RefreshHUDTimerFinished, RefreshHUDDelay);
	}
}


void UCombatComponent::Fire()
{
	if(!CanFire()) return;
	bCanFire = false;
	
	StartFireTimer();
	if(EquippedWeapon)
	{
		CrosshairShootFactor = FMath::Clamp(CrosshairShootFactor += EquippedWeapon->GetRecoilPerShot(), 0.f, EquippedWeapon->GetMaxRecoil());
		switch (EquippedWeapon->GetFireType())
		{
		case EFireType::EFT_Hitscan:
			FireHitscanWeapon();
			break;
		case EFireType::EFT_Projectile:
			FireProjectileWeapon();
			break;
		case EFireType::EFT_Shotgun:
			FireShotgunWeapon();
			break;
		default:
			break;
		}
	}
}

void UCombatComponent::FireProjectileWeapon()
{
	if(EquippedWeapon)
	{
		HitTarget = EquippedWeapon->bUseScatter ? EquippedWeapon->TraceEndWithScatter(HitTarget) : HitTarget;
		LocalFire(HitTarget);
		ServerFire(HitTarget);
	}
}

void UCombatComponent::FireHitscanWeapon()
{
	if(EquippedWeapon)
	{
		HitTarget = EquippedWeapon->bUseScatter ? EquippedWeapon->TraceEndWithScatter(HitTarget) : HitTarget;
		LocalFire(HitTarget);
		ServerFire(HitTarget);
	}
}

void UCombatComponent::FireShotgunWeapon()
{
	AShotgun* Shotgun = Cast<AShotgun>(EquippedWeapon);
	if (Shotgun && Character)
	{
		TArray<FVector_NetQuantize> HitTargets;
		Shotgun->ShotgunTraceEndWithScatter(HitTarget, HitTargets);
		if (!Character->HasAuthority()) ShotgunLocalFire(HitTargets);
		ServerShotgunFire(HitTargets);
	}
}

void UCombatComponent::FireButtonPressed(bool bPressed)
{
	bFireButtonPressed = bPressed;
	if(bFireButtonPressed && EquippedWeapon)
	{
		Fire();
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
		if(Character)
		{
			float DistanceToCharacter = (Character->GetActorLocation() - Start).Size();
			Start += CrosshairWorldDirection * (DistanceToCharacter + 100.f);
		}

		
		FVector End{Start + CrosshairWorldDirection * 10000.f};
		GetWorld()->LineTraceSingleByChannel(TraceHitResult, Start, End, ECollisionChannel::ECC_Visibility);
		if(!TraceHitResult.bBlockingHit)
		{
			TraceHitResult.ImpactPoint = End;
		}
		if(BlasterHUD)
		{
			if(TraceHitResult.GetActor() && TraceHitResult.GetActor()->Implements<UInteractWithCrosshairsInterface>())
			{
				BlasterHUD->SetCrosshairColor(FLinearColor::Red);
			}
			else
			{
				BlasterHUD->SetCrosshairColor(FLinearColor::White);
			}
		}
		
	}
}

void UCombatComponent::SetCrosshairsSpread(float DeltaTime)
{
	if(Character == nullptr || BlasterHUD == nullptr || EquippedWeapon == nullptr) return;
	
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
	if(bAiming)
	{
		CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor,  EquippedWeapon->GetAimAccuracy(), DeltaTime, 30.f);
	}
	else
	{
		CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor,  0.f, DeltaTime, 30.f);
	}

	CrosshairShootFactor = FMath::FInterpTo(CrosshairShootFactor, 0.f, DeltaTime, RecoilRecoverySpeed);
	
	const float Spread = FMath::Clamp((EquippedWeapon->GetBaseAccuracy() + CrosshairVelocityFactor + CrosshairShootFactor + CrosshairInAirFactor - CrosshairAimFactor), 0.f, 7.f);
	BlasterHUD->SetCrosshairSpread(Spread);
}


void UCombatComponent::InterpFOV(float DeltaTime)
{
	if(EquippedWeapon == nullptr) return;

	if(bAiming)
	{
		//Interp speed when zooming in is based on the weapon
		CurrentFOV = FMath::FInterpTo(CurrentFOV, EquippedWeapon->GetZoomFOV(), DeltaTime, EquippedWeapon->GetZoomInterpSpeed()); 
	}
	else
	{
		//Interp speed when zooming out is based on character
		CurrentFOV = FMath::FInterpTo(CurrentFOV, DefaultFOV, DeltaTime, ZoomInterpSpeed); 
	}
	if(Character && Character->GetCameraComponent())
	{
		Character->GetCameraComponent()->SetFieldOfView(CurrentFOV);
	}
}

void UCombatComponent::StartFireTimer()
{
	if(EquippedWeapon == nullptr || Character == nullptr) return;

	Character->GetWorldTimerManager().SetTimer(FireTimer, this, &UCombatComponent::FireTimerFinished, EquippedWeapon->GetFireDelay());
}

void UCombatComponent::FireTimerFinished()
{
	bCanFire = true;
	if(EquippedWeapon == nullptr) return;
	if(bFireButtonPressed && EquippedWeapon->IsAutomatic())
	{
		Fire();
	}
}

bool UCombatComponent::CanFire()
{
	if(EquippedWeapon == nullptr) return false;
	if(EquippedWeapon->IsEmpty() || !bCanFire ||CombatState != ECombatState::ECS_Unoccupied || bLocallyReloading) return false;
	return true;
}

void UCombatComponent::OnRep_CarriedAmmo()
{
	if(Character)
	{
		Character->UpdateHUDAmmo();
	}
}

void UCombatComponent::InitializeCarriedAmmo()
{
	CarriedAmmoMap.Emplace(EWeaponType::EWT_AssaultRifle, StartingARAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_RocketLauncher, StartingRocketAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_Pistol, StartingPistolAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_SubmachineGun, StartingSMGAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_Shotgun, StartingShotgunAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_SniperRifle, StartingSniperAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_GrenadeLauncher, StartingGrenadeLauncherAmmo);
}


void UCombatComponent::LocalFire(const FVector_NetQuantize& TraceHitTarget)
{
	if(EquippedWeapon == nullptr) return;
	if(Character && CombatState == ECombatState::ECS_Unoccupied)
	{
		Character->PlayFireMontage(bAiming);
		EquippedWeapon->Fire(TraceHitTarget);
	}
}

void UCombatComponent::ShotgunLocalFire(const TArray<FVector_NetQuantize>& HitTargets)
{
	AShotgun* Shotgun = Cast<AShotgun>(EquippedWeapon);
	if(Shotgun == nullptr || Character == nullptr) return;
	if(CombatState == ECombatState::ECS_Unoccupied)
	{
		Character->PlayFireMontage(bAiming);
		Shotgun->FireShotgun(HitTargets);
		CombatState = ECombatState::ECS_Unoccupied;
	}
}

void UCombatComponent::ServerFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	MulticastFire(TraceHitTarget);
}

void UCombatComponent::MulticastFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	if(Character && Character->IsLocallyControlled()) return;
	
	LocalFire(TraceHitTarget);
}

void UCombatComponent::ServerShotgunFire_Implementation(const TArray<FVector_NetQuantize>& TraceHitTargets)
{
	MulticastShotgunFire(TraceHitTargets);
}

void UCombatComponent::MulticastShotgunFire_Implementation(const TArray<FVector_NetQuantize>& TraceHitTargets)
{
	if(Character && Character->IsLocallyControlled()) return;
	ShotgunLocalFire(TraceHitTargets);
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
	if(CombatState != ECombatState::ECS_Unoccupied) return; 
	
	if(SecondaryWeapon != nullptr)
	{
		EquipPrimaryWeapon(WeaponToEquip);
	}else
	{
		EquipSecondaryWeapon(WeaponToEquip);
	}
}

void UCombatComponent::SwapWeapons() //BUG: When swapping weapons, clients HUD isn't updated correctly.
{
	if(EquippedWeapon == nullptr || SecondaryWeapon == nullptr || Character == nullptr || Character->GetCombatState() != ECombatState::ECS_Unoccupied) return;

	FDetachmentTransformRules DetachmentTransformRules(EDetachmentRule::KeepWorld, true);
	EquippedWeapon->GetWeaponMesh()->DetachFromComponent(DetachmentTransformRules);
	SecondaryWeapon->GetWeaponMesh()->DetachFromComponent(DetachmentTransformRules);
	
	EquippedWeaponLastFrame = EquippedWeapon;
	EquippedWeapon = nullptr;
	EquipPrimaryWeapon(SecondaryWeapon);
	EquipSecondaryWeapon(EquippedWeaponLastFrame);
	
}

void UCombatComponent::EquipPrimaryWeapon(AWeapon* WeaponToEquip)
{
	if(EquippedWeapon)
	{
		EquippedWeapon->Dropped();
	}
	EquippedWeapon = WeaponToEquip;
	if(Character == nullptr || Character->GetMesh() == nullptr) return;
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	
	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("hand_rSocket"));
	if(HandSocket)
	{
		HandSocket->AttachActor(EquippedWeapon, Character->GetMesh());
	}
	EquippedWeapon->SetOwner(Character);
	EquippedWeapon->SetHUDAmmo();
	EquippedWeapon->SetHUDMagAmmo();
	EquippedWeapon->SetHUDWeaponType();
	if(CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		TotalAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
		if(Character->HasAuthority())
		{
			Character->UpdateHUDAmmo();
		}
	}

	if(EquippedWeapon->GetEquipSound())
	{
		UGameplayStatics::PlaySoundAtLocation(this, EquippedWeapon->GetEquipSound(), Character->GetActorLocation());
	}
	
	Character->GetCharacterMovement()->bOrientRotationToMovement = false;
	Character->bUseControllerRotationYaw = true;
	
	SetCrosshairs();
}

void UCombatComponent::EquipSecondaryWeapon(AWeapon* WeaponToEquip)
{
	SecondaryWeapon = WeaponToEquip;
	if(SecondaryWeapon == nullptr || Character == nullptr || Character->GetMesh() == nullptr) return;
	SecondaryWeapon->SetWeaponState(EWeaponState::EWS_UnEquipped);
	SecondaryWeapon->SetOwner(Character);
	const USkeletalMeshSocket* BackpackSocket = Character->GetMesh()->GetSocketByName(FName("BackpackSocket"));
	if(BackpackSocket)
	{
		BackpackSocket->AttachActor(SecondaryWeapon, Character->GetMesh());
	}
}

void UCombatComponent::Reload()
{
	if(EquippedWeapon == nullptr) return;
	if(TotalAmmo > 0 && !EquippedWeapon->MagazineIsFull() && CombatState == ECombatState::ECS_Unoccupied && !bLocallyReloading)
	{
		ServerReload();
		HandleReload();
		bLocallyReloading = true;
	}
}

void UCombatComponent::FinishReloading()
{
	bLocallyReloading = false;
	if(Character && Character->HasAuthority())
	{
		CombatState = ECombatState::ECS_Unoccupied;
		
	}
	if(bFireButtonPressed)
	{
		Fire();
	}
}

void UCombatComponent::HandleReload()
{
	Character->PlayReloadMontage();
}

int32 UCombatComponent::AmountToReload()
{
	if(EquippedWeapon == nullptr) return 0;
	
	int32 MissingAmmo = EquippedWeapon->GetMagCapacity() - EquippedWeapon->GetAmmo();
	
	if(CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		//Returns the lowest value between missing ammo and carried ammo. 
		return FMath::Min(MissingAmmo, CarriedAmmoMap[EquippedWeapon->GetWeaponType()]);
	}
	
	return 0;
}

void UCombatComponent::OnRep_Aiming()
{
	if(Character && Character->IsLocallyControlled())
	{
		bAiming = bAimButtonPressed;
	}
}

void UCombatComponent::PickupAmmo(const EWeaponType InWeaponType, const int32 InAmmoAmount)
{
	if(CarriedAmmoMap.Contains(InWeaponType))
	{
		CarriedAmmoMap[InWeaponType] = FMath::Clamp(CarriedAmmoMap[InWeaponType] + InAmmoAmount, 0, MaxAmmo);
		
		BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : BlasterPlayerController;
		if(BlasterPlayerController)
		{
			if(EquippedWeapon && EquippedWeapon->GetWeaponType() == InWeaponType)
			{
				TotalAmmo = CarriedAmmoMap[InWeaponType];
				BlasterPlayerController->SetHUDTotalAmmo(TotalAmmo);
			}
		}
	}
}

void UCombatComponent::ServerReload_Implementation()
{
	if(Character == nullptr || EquippedWeapon == nullptr) return;
	CombatState = ECombatState::ECS_Reloading;
	
	int32 ReloadAmount = AmountToReload();
	if(CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmoMap[EquippedWeapon->GetWeaponType()] -= ReloadAmount;
		TotalAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
		EquippedWeapon->AddAmmo(ReloadAmount); //BUG: Someone is going to abuse this with animation cancels and reload much faster than intended
	}
	BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : BlasterPlayerController;
	if(BlasterPlayerController)
	{
		BlasterPlayerController->SetHUDTotalAmmo(TotalAmmo);
	}
	
	if(!Character->IsLocallyControlled()) HandleReload();
	
}

void UCombatComponent::OnRep_CombatState()
{
	switch (CombatState)
	{
	case ECombatState::ECS_Unoccupied:
		if(bFireButtonPressed)
		{
			Fire();
		}
		break;
	case ECombatState::ECS_Reloading:
		if(Character && !Character->IsLocallyControlled()) HandleReload();
		break;
	default:
		break;
	}
}

void UCombatComponent::RefreshHUDTimerFinished()
{
	SetCrosshairs();
	if(Character) Character->UpdateHUDAmmo();
}

void UCombatComponent::SetSpeeds(const float InAimSpeed, const float InBaseSpeed)
{
	BaseWalkSpeed = InBaseSpeed;
	AimWalkSpeed = InAimSpeed;
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UCombatComponent, SecondaryWeapon);
	DOREPLIFETIME(UCombatComponent, bAiming);
	DOREPLIFETIME(UCombatComponent, CombatState);
	DOREPLIFETIME_CONDITION(UCombatComponent, TotalAmmo, COND_OwnerOnly);
}


