// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blaster/TurningInPlace.h"
#include "Blaster/BlasterComponents/CombatState.h"
#include "GameFramework/Character.h"
#include "Blaster/Interfaces/InteractWithCrosshairsInterface.h"
#include "Components/TimelineComponent.h"
#include "BlasterCharacter.generated.h"

UCLASS()
class BLASTER_API ABlasterCharacter : public ACharacter, public IInteractWithCrosshairsInterface
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
	void PlayReloadMontage();
	
	virtual void OnRep_ReplicatedMovement() override;
	void UpdateHUDHealth();
	
	UFUNCTION(NetMulticast, Reliable)
	void MulticastElim();
	void Elim();
	void PlayElimMontage();
	virtual void Destroyed() override;

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
	void ReloadButtonPressed();
	float CalculateSpeed();
	void CalculateAO_Pitch();

	void AimOffset(float DeltaTime);
	void SimProxiesTurn();
	void TurnInPlace(float DeltaTime);
	
	virtual void Jump() override;
	
	UFUNCTION(BlueprintCallable)
	EPhysicalSurface GetSurfaceType();

	void FireButtonPressed();
	void FireButtonReleased();

	void PlayHitReactMontage();

	UFUNCTION()
	void ReceiveDamage(AActor* DamagedActor ,float Damage, const UDamageType* DamageType, class AController* InstigatorController, AActor* DamageCauser);

	//Initialize HUD if relevant info is missing
	void PollInit();

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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
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

	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* HitReactMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* EliminationMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* ReloadMontage;

	
	//Setting character invisible if camera gets too close
	void HideCharacter();

	//The distance to the camera at which the character should be set invisible.
	UPROPERTY(EditDefaultsOnly)
	float CharacterHideThreshold;

	bool bRotateRootBone;
	float TurnThreshold{0.5f};
	FRotator ProxyRotationLastFrame;
	FRotator ProxyRotation;
	float ProxyYaw;
	float TimeSinceLastMovementReplication;


	UPROPERTY(EditAnywhere, Category = "Player Status")
	float MaxHealth{100.f};

	UPROPERTY(ReplicatedUsing = OnRep_Health, VisibleAnywhere, Category = "Player Status")
	float Health;

	UFUNCTION()
	void OnRep_Health();

	UPROPERTY()
	class ABlasterPlayerController* BlasterPlayerController;

	bool bEliminated{false};

	FTimerHandle ElimTimer;
	UPROPERTY(EditDefaultsOnly);
	float ElimDelay{3.f};
	void ElimTimerFinished();

	FOnTimelineFloat DissolveTrack;
	UPROPERTY(VisibleAnywhere)
	UTimelineComponent* DissolveTimeline;

	UFUNCTION()
	void UpdateDissolveMaterial(float DissolveValue);
	void StartDissolve();

	UPROPERTY(EditAnywhere)
	UCurveFloat* DissolveCurve;

	//Changed at runtime, based on the DissolveMaterialInstance
	UPROPERTY(VisibleAnywhere, Category = Elim)
	UMaterialInstanceDynamic* DynamicDissolveMaterialInstance;

	//Set on the blueprint used by the dynamic version
	UPROPERTY(EditAnywhere, Category = Elim)
	UMaterialInstance* DissolveMaterialInstance;

	UPROPERTY(EditAnywhere)
	UParticleSystem* ElimBotEffect;

	UPROPERTY(VisibleAnywhere)
	UParticleSystemComponent* ElimBotComponent;

	UPROPERTY(EditAnywhere)
	class USoundCue* BotSoundCue;

	UPROPERTY()
	class ABlasterPlayerState* BlasterPlayerState;

	
public:

	void SetOverlappingWeapon(AWeapon* Weapon);

	bool IsWeaponEquipped();
	bool IsAiming();

	FORCEINLINE float GetAOYaw() const {return AO_Yaw;}
	FORCEINLINE float GetAOPitch() const {return AO_Pitch;}
	FORCEINLINE ETurningInPlace GetTurnInPlace() const {return TurningInPlace;}
	FORCEINLINE UCameraComponent* GetCameraComponent() const {return CameraComponent;}
	FORCEINLINE bool ShouldRotateRootBone() const {return bRotateRootBone;}
	FORCEINLINE bool IsEliminated() const {return bEliminated;}
	FORCEINLINE float GetHealth() const {return Health;}
	FORCEINLINE float GetMaxHealth() const {return MaxHealth;}
	AWeapon* GetEquippedWeapon();
	FVector GetHitTarget() const;
	ECombatState GetCombatState() const;
	int32 GetTotalAmmo() const;
	
	
};
