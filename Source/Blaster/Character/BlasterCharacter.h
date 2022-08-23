// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blaster/TurningInPlace.h"
#include "GameFramework/Character.h"
#include "BlasterCharacter.generated.h"

UCLASS()
class BLASTER_API ABlasterCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ABlasterCharacter();
	
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostInitializeComponents() override;

	void PlayFireMontage(bool bAiming);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void MoveForward(float Value);
	void MoveRight(float Value);
	void Turn(float Value);
	void LookUp(float Value);
	void TurnAtRate(float Value);
	void LookUpAtRate(float Value);
	void EquipButtonPressed();
	void CrouchButtonPressed();
	void AimButtonPressed();
	void AimButtonReleased();

	void AimOffset(float DeltaTime);
	void TurnInPlace(float DeltaTime);
	
	virtual void Jump() override;
	
	UFUNCTION(BlueprintCallable)
	EPhysicalSurface GetSurfaceType();

	void FireButtonPressed();
	void FireButtonReleased();

private:

	UPROPERTY(VisibleAnywhere, Category = Camera)
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = Camera)
	class UCameraComponent* CameraComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* OverheadWidget;

	//Turn rate for controller
	UPROPERTY(EditAnywhere, Category = Camera)
	float BaseTurnRate;
	UPROPERTY(EditAnywhere, Category = Camera)
	float BaseLookUpRate;

	UPROPERTY(EditAnywhere, Category = Camera, meta = (ClapMin = "0.05,", ClampMax = "5.0", UIMin = "0.05", UIMax = "5.0"))
	float MouseXSensitivity;
	
	UPROPERTY(EditAnywhere, Category = Camera, meta = (ClapMin = "0.05,", ClampMax = "5.0", UIMin = "0.05", UIMax = "5.0"))
	float MouseYSensitivity;

	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	class AWeapon* OverlappingWeapon;

	UPROPERTY(VisibleAnywhere)
	class UCombatComponent* CombatComponent;

	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon);

	UFUNCTION(Server, Reliable)
	void ServerEquipButtonPressed();
	
	float AO_Yaw; //For calculating aim offsets
	float AO_Pitch;
	float InterpAO_Yaw;
	FRotator StartingAimRotation;

	ETurningInPlace TurningInPlace;

	UPROPERTY(EditAnywhere, Category = Combat)
	class UAnimMontage* FireWeaponMontage;

public:

	void SetOverlappingWeapon(AWeapon* Weapon);

	bool IsWeaponEquipped();
	bool IsAiming();

	FORCEINLINE float GetAOYaw() const {return AO_Yaw;}
	FORCEINLINE float GetAOPitch() const {return AO_Pitch;}
	FORCEINLINE ETurningInPlace GetTurnInPlace() const {return TurningInPlace;}
	AWeapon* GetEquippedWeapon();
	FVector GetHitTarget() const;
	
};
