// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterCharacter.h"

#include "Blaster/Blaster.h"
#include "Blaster/BlasterPlayerController.h"
#include "Blaster/BlasterPlayerState.h"
#include "Blaster/BlasterComponents/CombatComponent.h"
#include "Blaster/GameMode/BlasterGameMode.h"
#include "Blaster/Weapon/Weapon.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Particles/ParticleSystemComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Sound/SoundCue.h"


// Sets default values
ABlasterCharacter::ABlasterCharacter():
BaseTurnRate(75.f),
BaseLookUpRate(75.f),
MouseXSensitivity(1.f),
MouseYSensitivity(1.f),
TurningInPlace(ETurningInPlace::ETIP_NotTurning)
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh());
	CameraBoom->TargetArmLength = 500.f;
	CameraBoom->bUsePawnControlRotation = true;
	
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	CameraComponent->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	CameraComponent->bUsePawnControlRotation = false;
	
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(RootComponent);

	CombatComponent = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	CombatComponent->SetIsReplicated(true);

	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	
	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;

	DissolveTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("DissolveTimelineComponent"));
}

void ABlasterCharacter::OnRep_ReplicatedMovement()
{
	Super::OnRep_ReplicatedMovement();
	
	SimProxiesTurn();
	TimeSinceLastMovementReplication = 0.f;
}

// Called when the game starts or when spawned
void ABlasterCharacter::BeginPlay()
{
	Super::BeginPlay();

	Health = MaxHealth;
	
	UpdateHUDHealth();
	
	if(HasAuthority())
	{
		OnTakeAnyDamage.AddDynamic(this, &ABlasterCharacter::ReceiveDamage);
	}
}

void ABlasterCharacter::RotateInPlace(float DeltaTime)
{
	if(bDisableGameplay)
	{
		bUseControllerRotationYaw = false;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		return;
	}
	if (GetLocalRole() > ENetRole::ROLE_SimulatedProxy && IsLocallyControlled())
	{
		AimOffset(DeltaTime);
	}
	else
	{
		TimeSinceLastMovementReplication += DeltaTime;
		if (TimeSinceLastMovementReplication > 0.25f)
		{
			OnRep_ReplicatedMovement();
		}
		CalculateAO_Pitch();
	}
}

void ABlasterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	RotateInPlace(DeltaTime);

	//Checks if character is too close to the camera, and if so, hides the character. TODO: Use interpolation and dither effect for a more refined hide effect
	HideCharacter();

	PollInit();
}

void ABlasterCharacter::MoveForward(float Value)
{
	if(bDisableGameplay) return;
	if(Controller != nullptr && Value != 0.f)
	{
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X));
		AddMovementInput(Direction, Value);
	}
}

void ABlasterCharacter::MoveRight(float Value)
{
	if(bDisableGameplay) return;
	if(Controller != nullptr && Value != 0.f)
	{
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y));
		AddMovementInput(Direction, Value);
	}
}

void ABlasterCharacter::Turn(float Value)
{
	AddControllerYawInput(Value * MouseXSensitivity);
}

void ABlasterCharacter::LookUp(float Value)
{
	AddControllerPitchInput(Value * MouseYSensitivity);
}

void ABlasterCharacter::TurnAtRate(float Value)
{
	AddControllerYawInput(Value * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void ABlasterCharacter::LookUpAtRate(float Value)
{
	AddControllerPitchInput(Value * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void ABlasterCharacter::EquipButtonPressed()
{
	if(bDisableGameplay) return;
	if(CombatComponent)
	{
		if(HasAuthority())
		{
			CombatComponent->EquipWeapon(OverlappingWeapon);
		}
		else
		{
			ServerEquipButtonPressed();
			CombatComponent->SetCrosshairs();
		}
	}
}

void ABlasterCharacter::CrouchButtonPressed()
{
	if(bDisableGameplay) return;
	if(bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Crouch();
	}
	
}

void ABlasterCharacter::AimButtonPressed()
{
	if(bDisableGameplay) return;
	if(CombatComponent)
	{
		CombatComponent->SetAiming(true);
	}
}


void ABlasterCharacter::AimButtonReleased()
{
	if(CombatComponent)
	{
		CombatComponent->SetAiming(false);
	}
}

void ABlasterCharacter::ReloadButtonPressed()
{
	if(bDisableGameplay) return;
	if(CombatComponent)
	{
		CombatComponent->Reload();
	}
}


float ABlasterCharacter::CalculateSpeed()
{
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.f;
	return Velocity.Size();
}

void ABlasterCharacter::AimOffset(float DeltaTime)
{
	
	if(CombatComponent && CombatComponent->EquippedWeapon == nullptr) return;
	
	float Speed = CalculateSpeed();
	bool bIsInAir = GetCharacterMovement()->IsFalling();

	if(Speed == 0.f && !bIsInAir)
	{
		bRotateRootBone = true;
		FRotator CurrentAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		FRotator DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation, StartingAimRotation);
		AO_Yaw = DeltaAimRotation.Yaw;
		if(TurningInPlace == ETurningInPlace::ETIP_NotTurning)
		{
			InterpAO_Yaw = AO_Yaw;
		}
		bUseControllerRotationYaw = true;
		TurnInPlace(DeltaTime);
	}
	if(Speed > 0.f || bIsInAir)
	{
		bRotateRootBone = false;
		StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		AO_Yaw = 0.f;
		bUseControllerRotationYaw = true;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	}

	CalculateAO_Pitch();
}

void ABlasterCharacter::CalculateAO_Pitch()
{
	AO_Pitch = GetBaseAimRotation().Pitch;
	//Need to map AO_Pitch because Unreal compresses rotation data so that the values are unsigned meaning 0 - 360 degrees
	if(AO_Pitch > 90.f && !IsLocallyControlled()) 
	{
		//Map pitch from [270, 360) to [-90, 0)
		FVector2D InRange(270.f, 360.f);
		FVector2D OutRange(-90.f, 0.f);
		AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);
		
	}
}

void ABlasterCharacter::SimProxiesTurn()
{
	if(CombatComponent == nullptr ||CombatComponent->EquippedWeapon == nullptr) return;

	bRotateRootBone = false;
	float Speed = CalculateSpeed();
	if(Speed > 0.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		return;
	}
	
	
	ProxyRotationLastFrame = ProxyRotation;
	ProxyRotation = GetActorRotation();
	ProxyYaw = UKismetMathLibrary::NormalizedDeltaRotator(ProxyRotation, ProxyRotationLastFrame).Yaw;
	
	if (FMath::Abs(ProxyYaw) > TurnThreshold)
	{
		if (ProxyYaw > TurnThreshold)
		{
			TurningInPlace = ETurningInPlace::ETIP_Right;
		}
		else if (ProxyYaw < -TurnThreshold)
		{
			TurningInPlace = ETurningInPlace::ETIP_Left;
		}
		else
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		}
		return;
	}
	TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	
}

void ABlasterCharacter::TurnInPlace(float DeltaTime)
{
	if(AO_Yaw > 90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Right;
	}
	else if(AO_Yaw < -90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Left;
	}else
	{
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	}
	
	if(TurningInPlace != ETurningInPlace::ETIP_NotTurning)
	{
		InterpAO_Yaw = FMath::FInterpTo(InterpAO_Yaw, 0.f, DeltaTime, 3.f);
		AO_Yaw = InterpAO_Yaw;
		if(FMath::Abs(AO_Yaw)< 15.f)
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
			StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		}
	}
}

void ABlasterCharacter::Jump()
{
	if(bDisableGameplay) return;
	if(bIsCrouched)
	{
		UnCrouch();
		Super::Jump();
	}
	else
	{
		Super::Jump();
	}
}

EPhysicalSurface ABlasterCharacter::GetSurfaceType()
{
	FHitResult HitResult;
	const FVector Start{GetActorLocation()};
	const FVector End{Start+FVector(0.f, 0.f, -400.f)};
	FCollisionQueryParams QueryParams;
	QueryParams.bReturnPhysicalMaterial = true;
	QueryParams.AddIgnoredActor(this);
	GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECollisionChannel::ECC_Visibility, QueryParams);
	
	return UPhysicalMaterial::DetermineSurfaceType(HitResult.PhysMaterial.Get());
	
}

void ABlasterCharacter::FireButtonPressed()
{
	if(bDisableGameplay) return;
	if(CombatComponent)
	{
		CombatComponent->FireButtonPressed(true);
	}
}

void ABlasterCharacter::FireButtonReleased()
{
	if(CombatComponent)
	{
		CombatComponent->FireButtonPressed(false);
	}
}

void ABlasterCharacter::OnRep_OverlappingWeapon(AWeapon* LastWeapon)
{
	
	if(OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(true);
	}
	if(LastWeapon)
	{
		LastWeapon->ShowPickupWidget(false);
	}
	
}

void ABlasterCharacter::ServerEquipButtonPressed_Implementation()
{
	if(CombatComponent)
	{
		CombatComponent->EquipWeapon(OverlappingWeapon);
	}
}


void ABlasterCharacter::HideCharacter()
{
	if(!IsLocallyControlled()) return;
	if((CameraComponent->GetComponentLocation() - GetActorLocation()).Size() < CharacterHideThreshold)
	{
		GetMesh()->SetVisibility(false);
		if(CombatComponent && CombatComponent->EquippedWeapon && CombatComponent->EquippedWeapon->GetWeaponMesh())
		{
			CombatComponent->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = true;
		}
	}
	else
	{
		GetMesh()->SetVisibility(true);
		if(CombatComponent && CombatComponent->EquippedWeapon && CombatComponent->EquippedWeapon->GetWeaponMesh())
		{
			CombatComponent->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = false;
		}
	}
}

void ABlasterCharacter::OnRep_Health()
{
	PlayHitReactMontage();
	UpdateHUDHealth();
}

void ABlasterCharacter::UpdateDissolveMaterial(float DissolveValue)
{
	if(DynamicDissolveMaterialInstance)
	{
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), DissolveValue);
	}
}

void ABlasterCharacter::StartDissolve()
{
	DissolveTrack.BindDynamic(this, &ABlasterCharacter::UpdateDissolveMaterial);
	if(DissolveCurve && DissolveTimeline)
	{
		DissolveTimeline->AddInterpFloat(DissolveCurve, DissolveTrack);
		DissolveTimeline->Play();
	}
}

void ABlasterCharacter::SetOverlappingWeapon(AWeapon* Weapon)
{
	if(IsLocallyControlled()) 
	{
		if(OverlappingWeapon)
		{
			OverlappingWeapon->ShowPickupWidget(false);
		}
	}
	// We are first setting the pickup widgets visibility to false. We assume that we are ending the overlap event
	// and want to disable the visibility. If the Overlapping weapon was set to a valid object pointer, then we set
	// the visibility to true.

	
	OverlappingWeapon = Weapon;
	
	if(IsLocallyControlled())
	{
		if(OverlappingWeapon)
		{
			OverlappingWeapon->ShowPickupWidget(true);
		}
	}
	
}

bool ABlasterCharacter::IsWeaponEquipped()
{
	return (CombatComponent && CombatComponent->EquippedWeapon);
}

bool ABlasterCharacter::IsAiming()
{
	return (CombatComponent && CombatComponent->bAiming);
}


AWeapon* ABlasterCharacter::GetEquippedWeapon()
{
	if(CombatComponent == nullptr) return nullptr;

	return CombatComponent->EquippedWeapon;
}

void ABlasterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	check(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &ABlasterCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ABlasterCharacter::MoveRight);
	PlayerInputComponent->BindAxis("TurnRate", this, &ABlasterCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUpRate", this, &ABlasterCharacter::LookUpAtRate);
	PlayerInputComponent->BindAxis("Turn", this, &ABlasterCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &ABlasterCharacter::LookUp);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ABlasterCharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	PlayerInputComponent->BindAction("Equip", IE_Pressed, this, &ABlasterCharacter::EquipButtonPressed);
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ABlasterCharacter::CrouchButtonPressed);
	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &ABlasterCharacter::AimButtonPressed);
	PlayerInputComponent->BindAction("Aim", IE_Released, this, &ABlasterCharacter::AimButtonReleased);
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ABlasterCharacter::FireButtonPressed);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &ABlasterCharacter::FireButtonReleased);
	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &ABlasterCharacter::ReloadButtonPressed);

}

void ABlasterCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if(CombatComponent)
	{
		CombatComponent->Character = this;
	}
}

void ABlasterCharacter::PlayFireMontage(bool bAiming)
{
	if(CombatComponent == nullptr || CombatComponent->EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if(AnimInstance && FireWeaponMontage)
	{
		AnimInstance->Montage_Play(FireWeaponMontage);
		FName SectionName;
		SectionName = bAiming ? FName("RifleShoulder") : FName("RifleHip");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ABlasterCharacter::PlayReloadMontage()
{
	if(CombatComponent == nullptr || CombatComponent->EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if(AnimInstance && ReloadMontage)
	{
		AnimInstance->Montage_Play(ReloadMontage);
		FName SectionName;
		switch (CombatComponent->EquippedWeapon->GetWeaponType())
		{
		case EWeaponType::EWT_AssaultRifle:
			//SectionName = FName("ReloadARHip");
			//AnimInstance->Montage_JumpToSection(SectionName);
			break;
		case EWeaponType::EWT_SubmachineGun:
			break;
		default:
			break;
		}
	}
}

void ABlasterCharacter::PlayHitReactMontage()
{
	if(CombatComponent == nullptr || CombatComponent->EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if(AnimInstance && HitReactMontage)
	{
		AnimInstance->Montage_Play(HitReactMontage);
		FName SectionName{FName("HitReactFront")};
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ABlasterCharacter::ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType,
	AController* InstigatorController, AActor* DamageCauser)
{

	Health = FMath::Clamp(Health - Damage, 0.f, MaxHealth);
	UpdateHUDHealth();
	PlayHitReactMontage();

	if(Health == 0.f)
	{
		ABlasterGameMode* GameMode = Cast<ABlasterGameMode>(GetWorld()->GetAuthGameMode());
		if(GameMode)
		{
			BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;
			ABlasterPlayerController* AttackerController = Cast<ABlasterPlayerController>(InstigatorController);
			GameMode->PlayerEliminated(this, BlasterPlayerController, AttackerController);
		}
	}
	
}

void ABlasterCharacter::PollInit()
{
	if(BlasterPlayerState == nullptr)
	{
		BlasterPlayerState = GetPlayerState<ABlasterPlayerState>();
		if(BlasterPlayerState)
		{
			BlasterPlayerState->AddToScore(0.f);
			BlasterPlayerState->AddToElims(0);
		}
	}
}

FVector ABlasterCharacter::GetHitTarget() const
{
	if(CombatComponent == nullptr) return  FVector();
	return CombatComponent->HitTarget;
}

ECombatState ABlasterCharacter::GetCombatState() const
{
	if(CombatComponent == nullptr) return ECombatState::ECS_MAX;
	return CombatComponent->CombatState;
	
}

int32 ABlasterCharacter::GetTotalAmmo() const
{
	if(CombatComponent == nullptr) return 0;
	return CombatComponent->TotalAmmo;
}

UCombatComponent* ABlasterCharacter::GetCombatComponent() const
{
	return CombatComponent;
}

void ABlasterCharacter::UpdateHUDHealth()
{
	BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;
	if(BlasterPlayerController)
	{
		BlasterPlayerController->SetHUDHealth(Health, MaxHealth);
	}
}

void ABlasterCharacter::MulticastElim_Implementation()
{
	if(BlasterPlayerController)
	{
		BlasterPlayerController->SetHUDAmmo(0);
		BlasterPlayerController->SetHUDMagText(0);
		BlasterPlayerController->SetHUDTotalAmmo(0);
	}
	
	bEliminated = true;
	PlayElimMontage();

	if(DissolveMaterialInstance)
	{
		DynamicDissolveMaterialInstance = UMaterialInstanceDynamic::Create(DissolveMaterialInstance, this);

		GetMesh()->SetMaterial(0, DynamicDissolveMaterialInstance);
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), 0.55f);
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("GlowAmount"), 150.f);
	}
	
	StartDissolve();

	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->StopMovementImmediately();

	bDisableGameplay = true;
	
	if(CombatComponent) CombatComponent->FireButtonPressed(false);
	
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	FVector BotSpawnPoint{GetActorLocation() + FVector(0.f, 0.f, 200.f)};
	if(ElimBotEffect)
	{
		ElimBotComponent = UGameplayStatics::SpawnEmitterAtLocation(this, ElimBotEffect, BotSpawnPoint, FRotator::ZeroRotator);
	}
	if(BotSoundCue)
	{
		UGameplayStatics::SpawnSoundAtLocation(this, BotSoundCue, BotSpawnPoint);
	}
}


void ABlasterCharacter::Elim()
{
	if(CombatComponent && CombatComponent->EquippedWeapon)
	{
		CombatComponent->EquippedWeapon->Dropped();
	}
	MulticastElim();
	GetWorldTimerManager().SetTimer(ElimTimer, this, &ABlasterCharacter::ElimTimerFinished, ElimDelay);
	
}

void ABlasterCharacter::ElimTimerFinished()
{
	ABlasterGameMode* GameMode = Cast<ABlasterGameMode>(GetWorld()->GetAuthGameMode());
	if(GameMode)
	{
		GameMode->RequestRespawn(this, Controller);
	}
	
}

void ABlasterCharacter::PlayElimMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if(AnimInstance && EliminationMontage)
	{
		AnimInstance->Montage_Play(EliminationMontage);
	}
}

void ABlasterCharacter::Destroyed()
{
	if(ElimBotComponent)
	{
		ElimBotComponent->DestroyComponent();
	}
	
	Super::Destroyed();
}

void ABlasterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ABlasterCharacter, OverlappingWeapon, COND_OwnerOnly);
	DOREPLIFETIME(ABlasterCharacter, Health);
	DOREPLIFETIME(ABlasterCharacter, bDisableGameplay);
}



