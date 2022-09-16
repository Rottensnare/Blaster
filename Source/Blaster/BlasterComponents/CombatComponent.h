// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CombatState.h"
#include "Blaster/Weapon/WeaponType.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BLASTER_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	

	UCombatComponent();
	
	friend class ABlasterCharacter; // ABlasterCharacter will have full access to this class, including protected and private sections

	//Calls SetCrosshairsSpread, TraceUnderCrosshairs and InterpFOV
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	//Sets the crosshair textures and creates a HUDPackage which is sent to the HUD class
	bool SetCrosshairs();
	//Calls EquipPrimaryWeapon or EquipSecondaryWeapon
	void EquipWeapon(class AWeapon* WeaponToEquip);
	//Detaches weapons and swaps places by calling EquipPrimaryWeapon with secondary weapon and EquipSecondaryWeapon with EquippedWeapon
	void SwapWeapons();
	//Calls ServerReload and HandleReload
	void Reload();
	//Sets combat state to unoccupied and calls Fire() if player is pressing the fire button
	UFUNCTION(BlueprintCallable)
	void FinishReloading();
	//Calls Fire
	void FireButtonPressed(bool bPressed);
	//Adds to the total ammo of the correct ammo type. Calls to update HUD total ammo
	void PickupAmmo(const EWeaponType InWeaponType, const int32 InAmmoAmount);

	bool bLocallyReloading{false}; //For client side prediction

protected:
	//Sets base values for movement speed, FOV and calls InitializeCarriedAmmo
	virtual void BeginPlay() override;
	//Sets aiming boolean, calls ServerSetAiming, sets MaxWalkSpeed.
	//If using a sniper, shows sniper scope widget and enables or disables scatter
	void SetAiming(bool bIsAiming);
	//Does almost everything that SetAiming does but on the server
	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bIsAiming);

	//Sets weapon state to equipped state, attaches the weapon to the hand socket. Sets a few movement booleans and starts RefreshHUDTimer
	UFUNCTION()
	void OnRep_EquippedWeapon();
	//Sets weapon to UnEquipped state, attaches weapon to the backpack socket, starts RefreshHUDTimer
	UFUNCTION()
	void OnRep_SecondaryWeapon();
	//Calls various function to update the HUD. Attaches the weapon to the hand socket. Sets weapon state to Equipped. Calls SetCrosshairs
	void EquipPrimaryWeapon(AWeapon* WeaponToEquip);
	//Sets weapon state to UnEquipped, attaches weapon to the backpack socket.
	void EquipSecondaryWeapon(AWeapon* WeaponToEquip);

	//Plays fire montages and calls EquippedWeapon->Fire
	void LocalFire(const FVector_NetQuantize& TraceHitTarget);
	//Plays fire montage and calls Shotgun->FireShotgun
	void ShotgunLocalFire(const TArray<FVector_NetQuantize>& HitTargets);
	//Calls StartFireTimer, updates CrosshairShootFactor, calls different Fire functions depending on the EFireType.
	void Fire();
	//Calls LocalFire and ServerFire
	void FireProjectileWeapon();
	//Calls LocalFire and ServerFire
	void FireHitscanWeapon();
	//Calls Shotgun->ShotgunTraceEndWithScatter, ShotgunLocalFire, ServerShotgunFire
	void FireShotgunWeapon();

	//Calls MulticastFire
	UFUNCTION(Server, Reliable)
	void ServerFire(const FVector_NetQuantize& TraceHitTarget );

	//Calls LocalFire
	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector_NetQuantize& TraceHitTarget );

	//Calls MulticastShotgunFire
	UFUNCTION(Server, Reliable)
	void ServerShotgunFire(const TArray<FVector_NetQuantize>& TraceHitTargets);

	//Calls ShotgunLocalFire
	UFUNCTION(NetMulticast, Reliable)
	void MulticastShotgunFire(const TArray<FVector_NetQuantize>& TraceHitTargets);

	//Fills TraceHitResults with data based on a line trace. Calls blasterHUD to change crosshair color.
	void TraceUnderCrosshairs(FHitResult& TraceHitResult);

	//Calculates crosshair spread based on a number of variables. Calls BlasterHUD->SetCrosshairSpread with the calculated spread value
	void SetCrosshairsSpread(float DeltaTime);

	//Calls AmountToReload, updates ammo values, calls AddAmmo.
	UFUNCTION(Server, Reliable)
	void ServerReload();

	//Calls Character->PlayReloadMontage
	void HandleReload();

	//Calculates correct amount of ammo to reload
	int32 AmountToReload();

private:

	UPROPERTY()
	class ABlasterCharacter* Character;
	UPROPERTY()
	class ABlasterPlayerController* BlasterPlayerController;
	UPROPERTY()
	class ABlasterHUD* BlasterHUD;
	
	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon, VisibleAnywhere)
	AWeapon* EquippedWeapon{nullptr};

	UPROPERTY(ReplicatedUsing = OnRep_SecondaryWeapon, VisibleAnywhere)
	AWeapon* SecondaryWeapon{nullptr};

	UPROPERTY()
	AWeapon* EquippedWeaponLastFrame; //Used to save the equipped weapon temporarily when swapping weapons

	UPROPERTY(ReplicatedUsing = "OnRep_Aiming")
	bool bAiming{false};

	bool bAimButtonPressed{false};

	UFUNCTION()
	void OnRep_Aiming(); //Sets bAiming boolean for clients

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float BaseWalkSpeed;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float AimWalkSpeed;

	bool bFireButtonPressed;

	FVector HitTarget; //Current target hit with a line trace

	float CrosshairVelocityFactor;
	float CrosshairInAirFactor;
	float CrosshairAimFactor;
	float CrosshairShootFactor;

	UPROPERTY(EditDefaultsOnly)
	float RecoilRecoverySpeed{20.f};

	float DefaultFOV; //Set in BeginPlay

	float CurrentFOV;

	UPROPERTY(EditAnywhere, Category = Combat)
	float ZoomedFOV{30.f};

	UPROPERTY(EditAnywhere, Category = Combat)
	float ZoomInterpSpeed{20.f}; //How fast FOV changes when going from not aiming to aiming and back

	void InterpFOV(float DeltaTime); //Interpolates FOV for a smooth FOV Change

	FTimerHandle FireTimer; //For resetting bCanFire boolean. 

	void StartFireTimer(); //Starts FireTimer
	void FireTimerFinished(); //Sets bCanFire to true and calls Fire if weapon is automatic and fire button is pressed
	
	bool bCanFire{true};

	bool CanFire(); //Checks if character can fire

	//For current WeaponType
	UPROPERTY(EditAnywhere , ReplicatedUsing = OnRep_CarriedAmmo)
	int32 TotalAmmo; //Amount of total ammo of currently equipped weapon

	UFUNCTION()
	void OnRep_CarriedAmmo(); //Calls Character->UpdateHUDAmmo

	//TODO: Implement max ammo per weapon type
	TMap<EWeaponType, int32> CarriedAmmoMap; //Stores total ammo values for each weapon type

	UPROPERTY(EditAnywhere)
	int32 MaxAmmo{420}; //Max ammo any weapon can have
	
	UPROPERTY(EditAnywhere)
	int32 StartingARAmmo{60};
	UPROPERTY(EditAnywhere)
	int32 StartingRocketAmmo{8};
	UPROPERTY(EditAnywhere)
	int32 StartingPistolAmmo{50};
	UPROPERTY(EditAnywhere)
	int32 StartingSMGAmmo{120};
	UPROPERTY(EditAnywhere)
	int32 StartingShotgunAmmo{24};
	UPROPERTY(EditAnywhere)
	int32 StartingSniperAmmo{20};
	UPROPERTY(EditAnywhere)
	int32 StartingGrenadeLauncherAmmo{8};
	
	void InitializeCarriedAmmo(); //Initialize CarriedAmmoMap values

	UPROPERTY(BlueprintReadOnly ,ReplicatedUsing = OnRep_CombatState, meta = (AllowPrivateAccess = "true"))
	ECombatState CombatState{ECombatState::ECS_Unoccupied}; //Current combat state

	UFUNCTION()
	void OnRep_CombatState();

	FTimerHandle RefreshHUDTimer; //Timer for making sure variables necessary for HUD are set
	float RefreshHUDDelay{0.1f};
	void RefreshHUDTimerFinished(); //Calls SetCrosshairs() and Character->UpdateHUDAmmo
	
	

public:

	FORCEINLINE int32 GetTotalAmmo() const {return TotalAmmo;}
	void SetSpeeds(const float InAimSpeed, const float InBaseSpeed);
};
