// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponType.h"
#include "Weapon.generated.h"

//Used for broadcasting to the Weapon Spawn Point class to start the weapon spawn timer.
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPickedUp);

//Useful for indicating what state the weapon is in.
UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	EWS_Initial UMETA(DisplayName = "Initial State"), // Initial State is reserved for weapons that are spawned and haven't been picked up.
	EWS_Equipped UMETA(DisplayName = "Equipped State"),
	EWS_Dropped UMETA(DisplayName = "Dropped State"),
	EWS_UnEquipped UMETA(DisplayName = "UnEquipped State"), //Unequipped is for weapons that are carried by the player but not equipped.
	
	EWS_MAX UMETA(DisplayName = "DefaultMax"),
};

//Used for determining which fire function to call, as well as some other variables.
UENUM(BlueprintType)
enum class EFireType : uint8
{
	EFT_Hitscan UMETA(DisplayName = "Hitscan Weapon"),
	EFT_Projectile UMETA(DisplayName = "Projectile Weapon"),
	EFT_Shotgun UMETA(DisplayName = "Shotgun Weapon"),

	EFT_MAX UMETA(DisplayName = "DefaultMax")
};

UCLASS()
class BLASTER_API AWeapon : public AActor
{
	GENERATED_BODY()
	
public:	

	AWeapon();

	//If someone forgets to add crosshairs, this will prevent a crash of type "Array out of bounds".
	virtual void OnConstruction(const FTransform& Transform) override;
	
	//Currently not used
	virtual void Tick(float DeltaTime) override;
	void ShowPickupWidget(bool bShowWidget); //Sets pickup widget visibility to bShowWidget
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	//Plays fire animation, ejects a casing if any, then calls SpendRound. Specific functionality implemented in the derived classes.
	virtual void Fire(const FVector& HitTarget);
	void Dropped(); //Detaches the weapon and sets state to dropped.
	virtual void OnRep_Owner() override; //Calls hud update functions when owner is not nullptr
	void SetHUDAmmo(); //Calls the characters UpdateHUDAmmo function
	void SetHUDMagAmmo(); //Calls the characters SetHUDMagText function
	void SetHUDWeaponType(); //Calls the characters SetHUDWeaponType function
	void AddAmmo(int32 AmmoToAdd); //Updates Ammo variable, calls SetHUDAmmo and calls the Client version of this function

	void EnableCustomDepth(bool bEnable); //Sets whether or not Custom depth is enabled
	
	bool bDestroyWeapon = false; // True for default weapons

	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	bool bUseScatter{false};
	
	//Delay between shots. Lower is better.
	UPROPERTY(EditAnywhere, Category = Combat)
	float FireDelay{.15f};
	//Returns a random vector based on the muzzle locations. Currently scatter amount is fixed, needs to be made dynamic.
	FVector TraceEndWithScatter(const FVector& HitTarget); 
	
	FOnPickedUp OnPickedUpDelegate; //Used for broadcasting to the WeaponSpawnPoint class to start the weapon spawn timer.

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	//Sets characters overlapping weapon to this
	UFUNCTION()
	void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);
	UFUNCTION()
	void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	//If ping is too high, switch Server-Side Rewind Off
	UFUNCTION()
	void OnPingTooHigh(bool bPingTooHigh);

	
	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	float DistanceToSphere{800.f}; //For scatter calculations, currently a fixed value
	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	float SphereRadius{75.f}; //Another variable for scatter calculations, currently a fixed value

	UPROPERTY(EditAnywhere)
	float Damage{10.f}; //Base damage, overridden by weapon blueprints

	UPROPERTY(EditAnywhere)
	float HeadshotMultiplier{1.5f}; // Headshot damage = Damage * HeadShotMultiplier

	UPROPERTY(EditAnywhere, Replicated)
	bool bUseServerSideRewind{false}; //Whether or not the gun should use SSR. True for most guns.

	UPROPERTY(EditAnywhere)
	bool bCanUseServerSideRewind{true}; //Currently not used

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
	EFireType FireType; //Automatic or Semi-automatic. No burst fire currently, or other rarer types.
	
	//For clients to set the WeaponState. More detailed documentation in the .cpp file
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

	//FOV when aiming
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

	//Updates ammo values, lag compensation taken into consideration
	UFUNCTION(Client, Reliable)
	void ClientUpdateAmmo(int32 ServerAmmo);

	//Updates ammo for clients
	UFUNCTION(Client, Reliable)
	void ClientAddAmmo(int32 AmmoToAdd);
	
	//Decreases ammo by 1, then calls SetHUDAmmo
	void SpendRound();

	UPROPERTY(EditAnywhere)
	int32 MagCapacity;

	//Check EWeaponType enum for types
	UPROPERTY(EditAnywhere)
	EWeaponType WeaponType;

	UPROPERTY(EditAnywhere)
	class USoundCue* ReloadSound;

	UPROPERTY(EditAnywhere)
	USoundCue* EquipSound;

	//Value from 247 to 252. Red, Green, Orange, Purple, Blue, Tan
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	int32 CustomDepthValue{CUSTOM_DEPTH_TAN};

	//Currently not used
	UPROPERTY(EditAnywhere)
	float BaseTurnRate{25.f};
	

	
public:

	void SetWeaponState(EWeaponState State); //For server, more detailed explanation in the .cpp file
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
	FORCEINLINE float GetHeadshotMultiplier() const {return HeadshotMultiplier;}
	
};

