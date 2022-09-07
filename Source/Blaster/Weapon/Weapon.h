// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponType.h"
#include "Weapon.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPickedUp);

UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	EWS_Initial UMETA(DisplayName = "Initial State"),
	EWS_Equipped UMETA(DisplayName = "Equipped State"),
	EWS_Dropped UMETA(DisplayName = "Dropped State"),
	EWS_UnEquipped UMETA(DisplayName = "UnEquipped State"),
	
	EWS_MAX UMETA(DisplayName = "MAX"),
};

UENUM()
enum class EFireType : uint8
{
	EFT_Hitscan UMETA(DisplayName = "Hitscan Weapon"),
	EFT_Projectile UMETA(DisplayName = "Projectile Weapon"),
	EFT_Shotgun UMETA(DisplayName = "Shotgun Weapon"),

	EFT_MAX UMETA(DisplayName = "DefaultMAX")
};

UCLASS()
class BLASTER_API AWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWeapon();

	//If someone forgets to add crosshairs, this will prevent a crash of type "Array out of bounds".
	virtual void OnConstruction(const FTransform& Transform) override;
	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	void ShowPickupWidget(bool bShowWidget);
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	//Plays fire animation, ejects a casing if any, then calls SpendRound. Specific functionality implemented in the derived classes.
	virtual void Fire(const FVector& HitTarget);
	void Dropped();
	virtual void OnRep_Owner() override;
	void SetHUDAmmo();
	void SetHUDMagAmmo();
	void SetHUDWeaponType();
	void AddAmmo(int32 AmmoToAdd);

	void EnableCustomDepth(bool bEnable);
	
	bool bDestroyWeapon = false;

	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	bool bUseScatter{false};
	
	//Delay between shots. Lower is better.
	UPROPERTY(EditAnywhere, Category = Combat)
	float FireDelay{.15f};
	
	FVector TraceEndWithScatter(const FVector& HitTarget);

	FOnPickedUp OnPickedUpDelegate;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);
	UFUNCTION()
	void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
	void OnPingTooHigh(bool bPingTooHigh);

	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	float DistanceToSphere{800.f};
	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	float SphereRadius{75.f};

	UPROPERTY(EditAnywhere)
	float Damage{10.f};

	UPROPERTY(EditAnywhere, Replicated)
	bool bUseServerSideRewind{false};

	UPROPERTY(EditAnywhere)
	bool bCanUseServerSideRewind{true};

	UPROPERTY()
	class ABlasterCharacter* OwnerCharacter;

	UPROPERTY()
	class ABlasterPlayerController* OwnerController;

private:
	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	class USkeletalMeshComponent* WeaponMesh;

	//When overlapped, Character can pickup the weapon, also the PickupWidget will be shown.
	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	class USphereComponent* AreaSphere;

	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_WeaponState)
	EWeaponState WeaponState;

	UPROPERTY(EditAnywhere)
	EFireType FireType;
	
	//For clients to set the WeaponState
	UFUNCTION()
	void OnRep_WeaponState();

	//Displays info about weapon when overlapping with the AreaSphere
	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	class UWidgetComponent* PickupWidget;

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	class UAnimationAsset* FireAnimation;

	//Class used to spawn a casing in the Fire function
	UPROPERTY(EditAnywhere)
	TSubclassOf<class ACasing> CasingClass;

	//Elements can be null. 0: Center, 1: Up, 2: Down, 3: Left, 4: Right.
	UPROPERTY(EditAnywhere, Category = Crosshairs)
	TArray<class UTexture2D*> Crosshairs;

	UPROPERTY(EditAnywhere)
	float ZoomedFOV{30.f};

	//How fast camera zooms in when aiming. Higher is better.
	UPROPERTY(EditAnywhere)
	float ZoomInterpSpeed{20.f};

	//Higher is better. 0.6 is default
	UPROPERTY(EditDefaultsOnly)
	float AimAccuracy{0.6f};

	//How accurate is the gun when doing nothing
	UPROPERTY(EditDefaultsOnly)
	float BaseAccuracy{0.9f};

	//Max amount of crosshair spread firing the weapon can cause. Lower is better.
	UPROPERTY(EditDefaultsOnly)
	float MaxRecoil{2.f};

	//Lower is better. Slowly adds up, need to wait. Characters combat component has Recoil recovery speed.
	UPROPERTY(EditDefaultsOnly)
	float RecoilPerShot{0.2f};

	UPROPERTY(EditAnywhere, Category = Combat)
	bool bAutomatic{true};

	//Current ammo in the magazine/weapon
	UPROPERTY(EditAnywhere)
	int32 Ammo;

	//Number of unprocessed server requests for ammo
	//Incremented in SpendRound, Decremented in ClientUpdateAmmo
	int32 Sequence{0};

	UFUNCTION(Client, Reliable)
	void ClientUpdateAmmo(int32 ServerAmmo);

	UFUNCTION(Client, Reliable)
	void ClientAddAmmo(int32 AmmoToAdd);
	
	//Decreases ammo by 1, then calls SetHUDAmmo
	void SpendRound();

	UPROPERTY(EditAnywhere)
	int32 MagCapacity;
	
	UPROPERTY(EditAnywhere)
	EWeaponType WeaponType;

	UPROPERTY(EditAnywhere)
	class USoundCue* ReloadSound;

	UPROPERTY(EditAnywhere)
	USoundCue* EquipSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	int32 CustomDepthValue{CUSTOM_DEPTH_TAN};

	UPROPERTY(EditAnywhere)
	float BaseTurnRate{25.f};
	

	
public:

	void SetWeaponState(EWeaponState State);
	FORCEINLINE USphereComponent* GetAreaSphere() const {return AreaSphere;}
	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const {return WeaponMesh;}
	FORCEINLINE TArray<UTexture2D*> GetCrosshairTextures() const {return Crosshairs;}
	FORCEINLINE float GetZoomFOV() const {return ZoomedFOV;}
	FORCEINLINE float GetZoomInterpSpeed() const {return ZoomInterpSpeed;}
	FORCEINLINE float GetAimAccuracy() const {return AimAccuracy;}
	FORCEINLINE float GetMaxRecoil() const {return MaxRecoil;}
	FORCEINLINE float GetRecoilPerShot() const {return RecoilPerShot;}
	FORCEINLINE bool IsAutomatic() const {return bAutomatic;}
	FORCEINLINE float GetFireDelay() const {return FireDelay;}
	FORCEINLINE bool IsEmpty() const {return Ammo <= 0;}
	FORCEINLINE EWeaponType GetWeaponType() const {return WeaponType;}
	FORCEINLINE EFireType GetFireType() const {return FireType;}
	FORCEINLINE bool MagazineIsFull() const {return Ammo >= MagCapacity;}
	FORCEINLINE int32 GetAmmo() const {return Ammo;}
	FORCEINLINE int32 GetMagCapacity() const {return MagCapacity;}
	FORCEINLINE USoundCue* GetReloadSound() const {return ReloadSound;}
	FORCEINLINE USoundCue* GetEquipSound() const {return EquipSound;}
	FORCEINLINE float GetBaseAccuracy() const {return BaseAccuracy;}
	FORCEINLINE void SetUseScatter(bool bInUseScatter) {bUseScatter = bInUseScatter;}
	FORCEINLINE float GetDamage() const {return Damage;}
	
};

