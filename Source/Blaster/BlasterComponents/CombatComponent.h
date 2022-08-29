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
	// Sets default values for this component's properties
	UCombatComponent();
	
	friend class ABlasterCharacter; // ABlasterCharacter will have full access to this class, including protected and private sections

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	bool SetCrosshairs();
	void EquipWeapon(class AWeapon* WeaponToEquip);
	void Reload();
	UFUNCTION(BlueprintCallable)
	void FinishReloading();
	void FireButtonPressed(bool bPressed);
	void PickupAmmo(const EWeaponType InWeaponType, const int32 InAmmoAmount);


protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	
	void SetAiming(bool bIsAiming);

	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bIsAiming);

	UFUNCTION()
	void OnRep_EquippedWeapon();
	void Fire();

	UFUNCTION(Server, Reliable)
	void ServerFire(const FVector_NetQuantize& TraceHitTarget );

	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector_NetQuantize& TraceHitTarget );
	
	void TraceUnderCrosshairs(FHitResult& TraceHitResult);

	void SetCrosshairsSpread(float DeltaTime);

	UFUNCTION(Server, Reliable)
	void ServerReload();

	void HandleReload();

	int32 AmountToReload();

private:

	UPROPERTY()
	class ABlasterCharacter* Character;
	UPROPERTY()
	class ABlasterPlayerController* BlasterPlayerController;
	UPROPERTY()
	class ABlasterHUD* BlasterHUD;
	
	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
	AWeapon* EquippedWeapon;

	UPROPERTY(Replicated)
	bool bAiming;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float BaseWalkSpeed;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float AimWalkSpeed;

	bool bFireButtonPressed;

	FVector HitTarget;

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
	float ZoomInterpSpeed{20.f};

	void InterpFOV(float DeltaTime);

	FTimerHandle FireTimer;

	void StartFireTimer();
	void FireTimerFinished();
	
	bool bCanFire{true};

	bool CanFire();

	//For current WeaponType
	UPROPERTY(EditAnywhere , ReplicatedUsing = OnRep_CarriedAmmo)
	int32 TotalAmmo;

	UFUNCTION()
	void OnRep_CarriedAmmo();

	//TODO: Implement max ammo per weapon type
	TMap<EWeaponType, int32> CarriedAmmoMap;

	UPROPERTY(EditAnywhere)
	int32 MaxAmmo{420};
	
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
	
	void InitializeCarriedAmmo();

	UPROPERTY(BlueprintReadOnly ,ReplicatedUsing = OnRep_CombatState, meta = (AllowPrivateAccess = "true"))
	ECombatState CombatState{ECombatState::ECS_Unoccupied};

	UFUNCTION()
	void OnRep_CombatState();
	

public:

	FORCEINLINE int32 GetTotalAmmo() const {return TotalAmmo;}
};
