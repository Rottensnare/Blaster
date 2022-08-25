// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponType.h"
#include "Weapon.generated.h"

UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	EWS_Initial UMETA(DisplayName = "Initial State"),
	EWS_Equipped UMETA(DisplayName = "Equipped State"),
	EWS_Dropped UMETA(DisplayName = "Dropped State"),
	
	EWS_MAX UMETA(DisplayName = "MAX"),
};


UCLASS()
class BLASTER_API AWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWeapon();
	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	void ShowPickupWidget(bool bShowWidget);
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void Fire(const FVector& HitTarget);
	void Dropped();
	virtual void OnRep_Owner() override;
	void SetHUDAmmo();
	void SetHUDMagAmmo();
	void SetTotalAmmo();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);
	UFUNCTION()
	void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	class USkeletalMeshComponent* WeaponMesh;
	
	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	class USphereComponent* AreaSphere;

	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_WeaponState)
	EWeaponState WeaponState;

	UFUNCTION()
	void OnRep_WeaponState();

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	class UWidgetComponent* PickupWidget;

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	class UAnimationAsset* FireAnimation;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class ACasing> CasingClass;

	UPROPERTY(EditAnywhere, Category = Crosshairs)
	TArray<class UTexture2D*> Crosshairs;

	UPROPERTY(EditAnywhere)
	float ZoomedFOV{30.f};

	UPROPERTY(EditAnywhere)
	float ZoomInterpSpeed{20.f};

	UPROPERTY(EditDefaultsOnly)
	float AimAccuracy{0.6f};

	UPROPERTY(EditDefaultsOnly)
	float MaxRecoil{2.f};

	UPROPERTY(EditDefaultsOnly)
	float RecoilPerShot{0.2f};

	UPROPERTY(EditAnywhere, Category = Combat)
	bool bAutomatic{true};

	UPROPERTY(EditAnywhere, Category = Combat)
	float FireDelay{.15f};

	UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_Ammo)
	int32 Ammo;

	UFUNCTION()
	void OnRep_Ammo();

	void SpendRound();

	UPROPERTY(EditAnywhere)
	int32 MagCapacity;
	
	UPROPERTY()
	class ABlasterCharacter* OwnerCharacter;

	UPROPERTY()
	class ABlasterPlayerController* OwnerController;

	UPROPERTY(EditAnywhere)
	EWeaponType WeaponType;
	
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
	FORCEINLINE bool MagazineIsFull() const {return Ammo >= MagCapacity;}
	FORCEINLINE int32 GetAmmo() const {return Ammo;}

	
};

