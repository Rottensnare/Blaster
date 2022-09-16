// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blaster/TurningInPlace.h"
#include "Blaster/BlasterComponents/CombatState.h"
#include "Blaster/GameMode/Teams.h"
#include "GameFramework/Character.h"
#include "Blaster/Interfaces/InteractWithCrosshairsInterface.h"
#include "Components/TimelineComponent.h"
#include "BlasterCharacter.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLeftGame);

UCLASS()
class BLASTER_API ABlasterCharacter : public ACharacter, public IInteractWithCrosshairsInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ABlasterCharacter();
	
	virtual void Tick(float DeltaTime) override; //Calls RotateInPlace upon other less important things
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	//Sets some initial values for components
	virtual void PostInitializeComponents() override;

	void PlayFireMontage(bool bAiming); //Just plays the fire montage based on aiming state
	void PlayReloadMontage(); //Plays Reload animation. Only 1 reload animation currently
	//Calls SimProxiesTurn and sets TimeSinceLastMovementReplication to 0.f
	virtual void OnRep_ReplicatedMovement() override;
	void UpdateHUDHealth(); //Calls PlayerController version with health values
	void UpdateHUDShields(); //Calls PlayerController version with shield values
	void UpdateHUDAmmo(); //Calls PlayerController version with ammo values

	//Handles functionality concerning getting eliminated, more in the .cpp file
	UFUNCTION(NetMulticast, Reliable)
	void MulticastElim(bool bPlayerLeftGame);
	//Called by the game mode. Drops weapons and calls the multicast version
	void Elim(bool bPlayerLeftGame);
	void PlayElimMontage(); //Plays death animation
	virtual void Destroyed() override; //Destroys elim bot effect for all players and destroys the default weapon

	UFUNCTION(BlueprintImplementableEvent)
	void ShowSniperScopeWidget(bool bShowScope); //Shows or hides sniper scope widget
			
	//Setting character invisible if camera gets too close
	void HideCharacter();

	void SpawnDefaultWeapon(); //Spawns weapon when player spawns

	UPROPERTY(Replicated)
	bool bDisableGameplay{false};

	TMap<FName, class UBoxComponent*> HitCollisionBoxes; //Hit boxes for Server-Side Rewind

	UFUNCTION(Server, Reliable)
	void ServerLeaveGame(); //Calls GameMode->PlayerLeftGame passing in the player state

	FOnLeftGame OnLeftGame; //Used by the ReturnToMainMenu user widget class

	UFUNCTION(NetMulticast, Reliable)
	void MulticastGainedTheLead(); //Spawns a crown effect above the character
	UFUNCTION(NetMulticast, Reliable)
	void MultiCastLostTheLead(); //Despawns the crown effect
	
	//BUG: callstack suggest that the first line crashes the game, but not likely
	//Sets characters material based on the team. 
	void SetTeamColor(ETeams Team); 

	UFUNCTION(Server, Reliable)
	void ServerAttachOrb(class AOrb* Orb);//Sets walk speed and calls the multicast version

	UFUNCTION(NetMulticast, Reliable)
	void MulticastAttachOrb(AOrb* Orb); //Calls Orb->PickedUp and attaches orb above players head

	UFUNCTION(NetMulticast, Reliable)
	void MulticastDropTheOrb(); //sets HeldOrb reference to nullptr and resets movement speed back to normal

	UPROPERTY(ReplicatedUsing = OnRep_HoldingTheOrb)
	bool bHoldingTheOrb = false;

protected:
	// Starts HUDInitTimer, Spawns default Weapon and binds ReceiveDamage to OnTakeAnyDamage
	virtual void BeginPlay() override;
	//Handles turning in place differently depending if the character is locally controlled
	void RotateInPlace(float DeltaTime); 
	
	void MoveForward(float Value); //Handles movement forwards and backwards
	void MoveRight(float Value); //Handles movement right and left 
	void Turn(float Value); //Horizontal turning with mouse
	void LookUp(float Value); //Vertical camera turn with mouse
	void TurnAtRate(float Value); //Horizontal turning with controller
	void LookUpAtRate(float Value); //Vertical camera turn with controller
	void EquipButtonPressed(); //Calls EquipWeapon passing in the OverlappingWeapon and sets crosshairs for client
	void SwapButtonPressed(); //Calls the server version
	void CrouchButtonPressed(); //Handles crouching
	void AimButtonPressed(); //Calls CombatComponent->SetAiming with true
	void AimButtonReleased(); //Calls CombatComponent->SetAiming with false
	void ReloadButtonPressed(); //Calls CombatComponent->Reload
	float CalculateSpeed(); //Gets current character velocity
	void CalculateAO_Pitch(); //Calculates aim offset pitch value

	void AimOffset(float DeltaTime); //Handles the aim offset, calls TurnInPlace and CalculateAO_Pitch
	void SimProxiesTurn(); //Used by simulated proxies for turning determining the state of TurningInPlace enum
	void TurnInPlace(float DeltaTime); //Updates variables used by the turn in place animations
	
	virtual void Jump() override; //Crouch
	//Returns physical surface type the character is standing on
	//Needed for determining which footstep sound should be played
	UFUNCTION(BlueprintCallable)
	EPhysicalSurface GetSurfaceType(); 

	void FireButtonPressed(); //Calls CombatComponent->FireButtonPressed with true
	void FireButtonReleased(); //Calls CombatComponent->FireButtonPressed with false

	void PlayHitReactMontage(); //Plays hit react animation. Currently only 1 animation

	//Updates health and shield values, calls necessary "cosmetic" functions and checks if the player's health is 0.f
	//Calls CurrentGameMode->PlayerEliminated if player's health was 0.f
	UFUNCTION()
	void ReceiveDamage(AActor* DamagedActor ,float Damage, const UDamageType* DamageType, class AController* InstigatorController, AActor* DamageCauser);

	void SetSpawnPoint(); //Gets all spawn points and teleports you to your teams own spawn points
	void OnPlayerStateInitialized(); //Handles initialization of some necessary values

	//Initialize HUD if relevant info is missing
	void PollInit();

	//Server-Side Rewind HitBoxes
	UPROPERTY(EditAnywhere)
	class UBoxComponent* HeadBox;
	UPROPERTY(EditAnywhere)
	UBoxComponent* PelvisBox;
	UPROPERTY(EditAnywhere)
	UBoxComponent* Spine02Box;
	UPROPERTY(EditAnywhere)
	UBoxComponent* Spine03Box;
	UPROPERTY(EditAnywhere)
	UBoxComponent* UpperArmLBox;
	UPROPERTY(EditAnywhere)
	UBoxComponent* UpperArmRBox;
	UPROPERTY(EditAnywhere)
	UBoxComponent* LowerArmLBox;
	UPROPERTY(EditAnywhere)
	UBoxComponent* LowerArmRBox;
	UPROPERTY(EditAnywhere)
	UBoxComponent* HandLBox;
	UPROPERTY(EditAnywhere)
	UBoxComponent* HandRBox;
	UPROPERTY(EditAnywhere)
	UBoxComponent* BackpackBox;
	UPROPERTY(EditAnywhere)
	UBoxComponent* BlanketBox;
	UPROPERTY(EditAnywhere)
	UBoxComponent* ThighLBox;
	UPROPERTY(EditAnywhere)
	UBoxComponent* ThighRBox;
	UPROPERTY(EditAnywhere)
	UBoxComponent* CalfLBox;
	UPROPERTY(EditAnywhere)
	UBoxComponent* CalfRBox;
	UPROPERTY(EditAnywhere)
	UBoxComponent* FootLBox;
	UPROPERTY(EditAnywhere)
	UBoxComponent* FootRBox;
	
	
	
	

private:

	UPROPERTY(VisibleAnywhere, Category = Camera)
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = Camera)
	class UCameraComponent* CameraComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* OverheadWidget; //Was used to show player net role

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
	class UCombatComponent* CombatComponent; //Component that handles combat related stuff

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UBuffComponent* BuffComponent; //Component that handles buff related stuff

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class ULagCompensationComponent* LagCompensationComponent; //Component that handles lag related stuff

	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon); //Handles showing pickup widget visibility

	UFUNCTION(Server, Reliable)
	void ServerEquipButtonPressed(); //Calls CombatComponent->EquipWeapon

	UFUNCTION(Server, Reliable)
	void ServerSwapButtonPressed(); //Calls CombatComponent->SwapWeapons
	
	float AO_Yaw; //For calculating aim offsets
	float AO_Pitch; //For aim offset pitch
	float InterpAO_Yaw; //Used for smoothing the aim offset with interpolation
	FRotator StartingAimRotation; //Used in aim offset calculations when normalizing delta rotators

	ETurningInPlace TurningInPlace; //Turn in Place state, I.E. Left, right, NoTurning

	UPROPERTY(EditAnywhere, Category = Combat)
	class UAnimMontage* FireWeaponMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* HitReactMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* EliminationMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* ReloadMontage;

	UPROPERTY(EditAnywhere)
	TSubclassOf<AWeapon> DefaultWeaponClass; //Weapon class spawned when player spawns

	UPROPERTY()
	AWeapon* DefaultWeapon; //For keeping track of the default weapon, so that it can be destroyed later
	
	//The distance to the camera at which the character should be set invisible.
	UPROPERTY(EditDefaultsOnly)
	float CharacterHideThreshold;

	bool bRotateRootBone; //If rootbone should be turned
	float TurnThreshold{0.5f};
	FRotator ProxyRotationLastFrame; //Used by simulated proxies when normalizing delta rotators (B)
	FRotator ProxyRotation; //Same as above (A)
	float ProxyYaw;
	float TimeSinceLastMovementReplication;


	UPROPERTY(EditAnywhere, Category = "Player Status")
	float MaxHealth{100.f};

	UPROPERTY(ReplicatedUsing = OnRep_Health, VisibleAnywhere, Category = "Player Status")
	float Health;

	UFUNCTION()
	void OnRep_Health(float LastHealth); //Calls PlayHitReactMontage and UpdateHUDHealth for clients
	
	UPROPERTY(EditAnywhere, Category = "Player Status")
	float MaxShields{50.f};

	UPROPERTY(ReplicatedUsing = OnRep_Shields, EditAnywhere, Category = "Player Status")
	float Shields{0.f};

	UFUNCTION()
	void OnRep_Shields(float LastShields); //Calls PlayHitReactMontage and UpdateHUDShields for clients

	UPROPERTY()
	class ABlasterPlayerController* BlasterPlayerController;

	bool bEliminated{false};
	
	FTimerHandle ElimTimer; //Basically a respawn timer
	UPROPERTY(EditDefaultsOnly);
	float ElimDelay{3.f}; //Respawn delay
	void ElimTimerFinished(); //Respawns player or broadcasts with OnLeftGame delegate if player left

	bool bLeftGame{false};

	FOnTimelineFloat DissolveTrack; //Timeline for playing the dissolve effect
	UPROPERTY(VisibleAnywhere)
	UTimelineComponent* DissolveTimeline;

	UFUNCTION()
	void UpdateDissolveMaterial(float DissolveValue); //Sets Dissolve value for the dissolve material
	//Starts dissolve timeline when player gets eliminated and binds UpdateDissolveMaterial to the track
	void StartDissolve(); 

	UPROPERTY(EditAnywhere)
	UCurveFloat* DissolveCurve; //Curve for the dissolve value

	//Changed at runtime, based on the DissolveMaterialInstance
	UPROPERTY(VisibleAnywhere, Category = Elim)
	UMaterialInstanceDynamic* DynamicDissolveMaterialInstance;

	//Set on the blueprint used by the dynamic version
	UPROPERTY(VisibleAnywhere, Category = Elim)
	UMaterialInstance* DissolveMaterialInstance;

	//Different materials based on team
	UPROPERTY(EditAnywhere, Category = Elim)
	UMaterialInstance* RedDissolveMatInst;

	UPROPERTY(EditAnywhere, Category = Elim)
	UMaterialInstance* BlueDissolveMatInst;

	UPROPERTY(EditAnywhere, Category = Elim)
	UMaterialInstance* RedMaterial;

	UPROPERTY(EditAnywhere, Category = Elim)
	UMaterialInstance* BlueMaterial;

	UPROPERTY(EditAnywhere, Category = Elim)
	UMaterialInstance* OriginalMaterial;

	//Elim bot effect when player gets eliminated
	UPROPERTY(EditAnywhere)
	UParticleSystem* ElimBotEffect;

	UPROPERTY(VisibleAnywhere)
	UParticleSystemComponent* ElimBotComponent;

	UPROPERTY(EditAnywhere)
	class USoundCue* BotSoundCue;

	UPROPERTY()
	class ABlasterPlayerState* BlasterPlayerState;

	FTimerHandle HUDInitTimer; //Making sure HUD gets initialized when necessary values are set
	void HUDInitTimerFinished();
	float HUDInitDelay{0.2f};

	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* CrownSystem; //Crown effect for players in the lead

	UPROPERTY()
	class UNiagaraComponent* CrownComponent;

	UPROPERTY()
	class ABlasterGameMode* CurrentGameMode;

	UPROPERTY(EditAnywhere)
	USceneComponent* OrbAttachLocation; //For CTF. Location to attach the orb

	UPROPERTY()
	class AOrb* HeldOrb;

	UFUNCTION()
	void OnRep_HoldingTheOrb(); //Sets movement speed if holding orb or not, for clients
	
public:

	void SetOverlappingWeapon(AWeapon* Weapon); //Sets pickup widget visibility and sets OverlappingWeapon to Weapon

	bool IsWeaponEquipped();
	bool IsAiming();

	FORCEINLINE float GetAOYaw() const {return AO_Yaw;}
	FORCEINLINE float GetAOPitch() const {return AO_Pitch;}
	FORCEINLINE ETurningInPlace GetTurnInPlace() const {return TurningInPlace;}
	FORCEINLINE UCameraComponent* GetCameraComponent() const {return CameraComponent;}
	FORCEINLINE bool ShouldRotateRootBone() const {return bRotateRootBone;}
	FORCEINLINE bool IsEliminated() const {return bEliminated;}
	FORCEINLINE float GetHealth() const {return Health;}
	FORCEINLINE void SetHealth(const float InAmount) {Health = InAmount;}
	FORCEINLINE float GetShields() const {return Shields;}
	FORCEINLINE void SetShields(const float InAmount) {Shields = InAmount;}
	FORCEINLINE float GetMaxHealth() const {return MaxHealth;}
	FORCEINLINE float GetMaxShields() const {return MaxShields;}
	FORCEINLINE bool GetDisableGameplay() const {return bDisableGameplay;}
	FORCEINLINE ULagCompensationComponent* GetLagCompensationComponent() const {return LagCompensationComponent;}
	FORCEINLINE AOrb* GetHeldOrb() const {return HeldOrb;}
	bool IsLocallyReloading() const;
	AWeapon* GetEquippedWeapon();
	FVector GetHitTarget() const;
	ECombatState GetCombatState() const;
	int32 GetTotalAmmo() const;
	UCombatComponent* GetCombatComponent() const;
	UBuffComponent* GetBuffComponent() const;
	
	
};
