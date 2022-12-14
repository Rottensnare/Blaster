// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterCharacter.h"

#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Blaster/Blaster.h"
#include "Blaster/BlasterPlayerController.h"
#include "Blaster/BlasterPlayerState.h"
#include "Blaster/TeamPlayerStart.h"
#include "Blaster/BlasterComponents/BuffComponent.h"
#include "Blaster/BlasterComponents/CombatComponent.h"
#include "Blaster/BlasterComponents/LagCompensationComponent.h"
#include "Blaster/GameMode/BlasterGameMode.h"
#include "Blaster/GameState/BlasterGameState.h"
#include "Blaster/Items/Orb.h"
#include "Blaster/Weapon/Weapon.h"
#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
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

	OrbAttachLocation = CreateDefaultSubobject<USceneComponent>(TEXT("OrbAttachLocation"));
	OrbAttachLocation->SetupAttachment(RootComponent);
	
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(RootComponent);

	CombatComponent = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	CombatComponent->SetIsReplicated(true);

	BuffComponent = CreateDefaultSubobject<UBuffComponent>(TEXT("BuffComponent"));
	BuffComponent->SetIsReplicated(true);

	LagCompensationComponent = CreateDefaultSubobject<ULagCompensationComponent>(TEXT("LagCompensationComponent"));
	

	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	
	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;

	DissolveTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("DissolveTimelineComponent"));

	//Server-Side Rewind

	HeadBox = CreateDefaultSubobject<UBoxComponent>(TEXT("HeadBox"));
	HeadBox->SetupAttachment(GetMesh(), FName("head"));
	HitCollisionBoxes.Add(FName("head"),HeadBox);

	PelvisBox = CreateDefaultSubobject<UBoxComponent>(TEXT("PelvisBox"));
	PelvisBox->SetupAttachment(GetMesh(), FName("Pelvis"));
	HitCollisionBoxes.Add(FName("Pelvis"),PelvisBox);

	Spine02Box = CreateDefaultSubobject<UBoxComponent>(TEXT("Spine02Box"));
	Spine02Box->SetupAttachment(GetMesh(), FName("spine_02"));
	HitCollisionBoxes.Add(FName("spine_02"),Spine02Box);

	Spine03Box = CreateDefaultSubobject<UBoxComponent>(TEXT("Spine03Box"));
	Spine03Box->SetupAttachment(GetMesh(), FName("spine_03"));
	HitCollisionBoxes.Add(FName("spine_03"),Spine03Box);
	
	UpperArmLBox = CreateDefaultSubobject<UBoxComponent>(TEXT("UpperArmLBox"));
	UpperArmLBox->SetupAttachment(GetMesh(), FName("UpperArm_L"));
	HitCollisionBoxes.Add(FName("UpperArm_L"),UpperArmLBox);

	UpperArmRBox = CreateDefaultSubobject<UBoxComponent>(TEXT("UpperArmRBox"));
	UpperArmRBox->SetupAttachment(GetMesh(), FName("UpperArm_R"));
	HitCollisionBoxes.Add(FName("UpperArm_R"),UpperArmRBox);

	LowerArmLBox = CreateDefaultSubobject<UBoxComponent>(TEXT("LowerArmLBox"));
	LowerArmLBox->SetupAttachment(GetMesh(), FName("LowerArm_L"));
	HitCollisionBoxes.Add(FName("LowerArm_L"),LowerArmLBox);

	LowerArmRBox = CreateDefaultSubobject<UBoxComponent>(TEXT("LowerArmRBox"));
	LowerArmRBox->SetupAttachment(GetMesh(), FName("LowerArm_R"));
	HitCollisionBoxes.Add(FName("LowerArm_R"),LowerArmRBox);

	HandLBox = CreateDefaultSubobject<UBoxComponent>(TEXT("HandLBox"));
	HandLBox->SetupAttachment(GetMesh(), FName("Hand_L"));
	HitCollisionBoxes.Add(FName("Hand_L"),HandLBox);

	HandRBox = CreateDefaultSubobject<UBoxComponent>(TEXT("HandRBox"));
	HandRBox->SetupAttachment(GetMesh(), FName("Hand_R"));
	HitCollisionBoxes.Add(FName("Hand_R"),HandRBox);

	BackpackBox = CreateDefaultSubobject<UBoxComponent>(TEXT("BackpackBox"));
	BackpackBox->SetupAttachment(GetMesh(), FName("backpack"));
	HitCollisionBoxes.Add(FName("backpack"),BackpackBox);

	BlanketBox = CreateDefaultSubobject<UBoxComponent>(TEXT("BlanketBox"));
	BlanketBox->SetupAttachment(GetMesh(), FName("blanket_l"));
	HitCollisionBoxes.Add(FName("blanket_l"),BlanketBox);

	ThighLBox = CreateDefaultSubobject<UBoxComponent>(TEXT("ThighLBox"));
	ThighLBox->SetupAttachment(GetMesh(), FName("Thigh_L"));
	HitCollisionBoxes.Add(FName("Thigh_L"),ThighLBox);

	ThighRBox = CreateDefaultSubobject<UBoxComponent>(TEXT("ThighRBox"));
	ThighRBox->SetupAttachment(GetMesh(), FName("Thigh_R"));
	HitCollisionBoxes.Add(FName("Thigh_R"),ThighRBox);

	CalfLBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CalfLBox"));
	CalfLBox->SetupAttachment(GetMesh(), FName("Calf_L"));
	HitCollisionBoxes.Add(FName("Calf_L"),CalfLBox);

	CalfRBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CalfRBox"));
	CalfRBox->SetupAttachment(GetMesh(), FName("Calf_R"));
	HitCollisionBoxes.Add(FName("Calf_R"),CalfRBox);

	FootLBox = CreateDefaultSubobject<UBoxComponent>(TEXT("FootLBox"));
	FootLBox->SetupAttachment(GetMesh(), FName("Foot_L"));
	HitCollisionBoxes.Add(FName("Foot_L"),FootLBox);

	FootRBox = CreateDefaultSubobject<UBoxComponent>(TEXT("FootRBox"));
	FootRBox->SetupAttachment(GetMesh(), FName("Foot_R"));
	HitCollisionBoxes.Add(FName("Foot_R"),FootRBox);

	for(auto Box : HitCollisionBoxes)
	{
		if(Box.Value)
		{
			Box.Value->SetCollisionObjectType(ECC_HitBox);
			Box.Value->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			Box.Value->SetCollisionResponseToChannel(ECC_HitBox, ECR_Block);
			Box.Value->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
}

void ABlasterCharacter::BeginPlay()
{
	Super::BeginPlay();

	Health = MaxHealth;
	GetWorldTimerManager().SetTimer(HUDInitTimer, this, &ABlasterCharacter::HUDInitTimerFinished, HUDInitDelay);
	SpawnDefaultWeapon();
	
	if(HasAuthority())
	{
		OnTakeAnyDamage.AddDynamic(this, &ABlasterCharacter::ReceiveDamage);
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

void ABlasterCharacter::SetTeamColor(ETeams Team)
{
	if(GetMesh() == nullptr || OriginalMaterial == nullptr || BlueMaterial == nullptr ||RedMaterial == nullptr) return;
	switch(Team)
	{
	case ETeams::ET_NoTeam:
		GetMesh()->SetMaterial(0, OriginalMaterial);
		DissolveMaterialInstance = BlueDissolveMatInst;
		break;
	case ETeams::ET_BlueTeam:
		GetMesh()->SetMaterial(0, BlueMaterial);
		DissolveMaterialInstance = BlueDissolveMatInst;
		break;
	case ETeams::ET_RedTeam:
		GetMesh()->SetMaterial(0, RedMaterial);
		DissolveMaterialInstance = RedDissolveMatInst;
		break;
	default:
		break;
	}
}

void ABlasterCharacter::ServerAttachOrb_Implementation(AOrb* Orb)
{
	bHoldingTheOrb = true;
	if(CombatComponent && GetCharacterMovement())
	{
		GetCharacterMovement()->MaxWalkSpeed = CombatComponent->AimWalkSpeed;
	}
	MulticastAttachOrb(Orb);
	
}

void ABlasterCharacter::MulticastAttachOrb_Implementation(AOrb* Orb)
{
	UE_LOG(LogTemp, Warning, TEXT("ABlasterCharacter::MulticastAttachOrb_Implementation"))
	if(Orb == nullptr) return;
	HeldOrb = Orb;
	Orb->SetOwningCharacter(this);
	
	Orb->PickedUp();
	
	Orb->AttachToComponent(OrbAttachLocation, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	
}

void ABlasterCharacter::MulticastDropTheOrb_Implementation()
{
	if(HeldOrb)
	{
		HeldOrb = nullptr;
	}
	UE_LOG(LogTemp, Warning, TEXT("ABlasterCharacter::MulticastDropTheOrb_Implementation"))
	bHoldingTheOrb = false;
	if(GetCharacterMovement() && CombatComponent) GetCharacterMovement()->MaxWalkSpeed = CombatComponent->BaseWalkSpeed;
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
		if(OverlappingWeapon)
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
}

void ABlasterCharacter::SwapButtonPressed()
{
	if(bDisableGameplay) return;
	ServerSwapButtonPressed();
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

void ABlasterCharacter::ServerSwapButtonPressed_Implementation()
{
	if(CombatComponent)
	{
		CombatComponent->SwapWeapons();
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

void ABlasterCharacter::SpawnDefaultWeapon()
{
	ABlasterGameMode* BlasterGameMode = Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this));
	UWorld* World = GetWorld();
	if(BlasterGameMode && World && !bEliminated && DefaultWeaponClass)
	{
		AWeapon* StartingWeapon =  World->SpawnActor<AWeapon>(DefaultWeaponClass);
		if(StartingWeapon)
		{
			StartingWeapon->bDestroyWeapon = true;
			StartingWeapon->SetOwner(this);
			DefaultWeapon = StartingWeapon;
		}
		if(CombatComponent)
		{
			CombatComponent->EquipPrimaryWeapon(StartingWeapon);
		}
	}
}

void ABlasterCharacter::OnRep_Health(float LastHealth)
{
	if(Health < LastHealth)
	{
		PlayHitReactMontage();
	}
	UpdateHUDHealth();
}

void ABlasterCharacter::OnRep_Shields(float LastShields)
{
	if(Shields < LastShields)
	{
		PlayHitReactMontage();
	}
	UpdateHUDShields();
}

void ABlasterCharacter::OnRep_ReplicatedMovement()
{
	Super::OnRep_ReplicatedMovement();
	
	SimProxiesTurn();
	TimeSinceLastMovementReplication = 0.f;
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

void ABlasterCharacter::HUDInitTimerFinished()
{
	EquipButtonPressed();
	UpdateHUDAmmo();
	UpdateHUDHealth();
	UpdateHUDShields();
	
	
	if(BlasterPlayerController && CombatComponent && CombatComponent->EquippedWeapon)
	{
		FText WeaponText = UEnum::GetDisplayValueAsText(CombatComponent->EquippedWeapon->GetWeaponType());
		FString TempWeaponString = WeaponText.ToString();
		
//In a packaged game, GetDisplayValueAsText returns the complete name of the Enum, not just the display name, so need to create a substring to get rid of the EWT_
#if WITH_EDITOR
		
#else
		TempWeaponString = TempWeaponString.RightChop(4);
#endif
		
		BlasterPlayerController->SetHUDWeaponType(TempWeaponString);
	}
}

void ABlasterCharacter::OnRep_HoldingTheOrb()
{
	if(CombatComponent == nullptr) return;
	if(bHoldingTheOrb)
	{
		UE_LOG(LogTemp, Warning, TEXT("Holding The Orb"))
		GetCharacterMovement()->MaxWalkSpeed = CombatComponent->AimWalkSpeed;
		
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("NOT Holding The Orb"))
		//HeldOrb->Dropped(GetActorLocation());
		//HeldOrb = nullptr;
		GetCharacterMovement()->MaxWalkSpeed = CombatComponent->BaseWalkSpeed;
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


bool ABlasterCharacter::IsLocallyReloading() const
{
	if(CombatComponent) return CombatComponent->bLocallyReloading;
	else return false;
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
	PlayerInputComponent->BindAction("SwapWeapons", IE_Pressed, this, &ABlasterCharacter::SwapButtonPressed);
}

void ABlasterCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if(CombatComponent)
	{
		CombatComponent->Character = this;
	}
	if(BuffComponent)
	{
		BuffComponent->Character = this;
		BuffComponent->SetInitialSpeeds(GetCharacterMovement()->MaxWalkSpeed, GetCharacterMovement()->MaxWalkSpeedCrouched);
		BuffComponent->SetInitialJumpVelocity(GetCharacterMovement()->JumpZVelocity);
	}
	if(LagCompensationComponent)
	{
		LagCompensationComponent->BlasterCharacter = this;
		if(Controller)
		{
			LagCompensationComponent->BlasterController = Cast<ABlasterPlayerController>(Controller);
		}
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
			//AnimInstance->Montage_JumpToSection(SectionName); //BUG: If there is only one section, the animation can fail
			break;
		case EWeaponType::EWT_SubmachineGun:
			break;
		case EWeaponType::EWT_Pistol:
			break;
		case EWeaponType::EWT_RocketLauncher:
			break;
		case EWeaponType::EWT_Shotgun:
			break;
		case EWeaponType::EWT_SniperRifle:
			break;
		case EWeaponType::EWT_GrenadeLauncher:
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
	
	CurrentGameMode = CurrentGameMode == nullptr ? Cast<ABlasterGameMode>(GetWorld()->GetAuthGameMode()) : CurrentGameMode;
	if(bEliminated || CurrentGameMode == nullptr) return;

	Damage = CurrentGameMode->CalculateDamage(Controller, InstigatorController, Damage);
	if(Damage == 0.f) return;
	
	//BUG: Possibly will cause a bug in the future. Reference Lecture 166 "Updating the Shield"
	float DamageToHealth; 
	if(Shields - Damage < 0.f)
	{
		DamageToHealth = Damage - Shields;
		Shields = 0.f;
	}else
	{
		Shields = FMath::Clamp(Shields - Damage, 0.f, MaxShields);
		DamageToHealth = 0.f;
	}
	
	
	Health = FMath::Clamp(Health - DamageToHealth, 0.f, MaxHealth);
	UpdateHUDShields();
	UpdateHUDHealth();
	PlayHitReactMontage();

	if(Health == 0.f)
	{
		if(CurrentGameMode)
		{
			BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;
			ABlasterPlayerController* AttackerController = Cast<ABlasterPlayerController>(InstigatorController);
			CurrentGameMode->PlayerEliminated(this, BlasterPlayerController, AttackerController);
		}
	}
}

void ABlasterCharacter::SetSpawnPoint()
{
	if(HasAuthority() && BlasterPlayerState->GetTeam() != ETeams::ET_NoTeam)
	{
	
		TArray<AActor*> PlayerStarts;
		UGameplayStatics::GetAllActorsOfClass(this, ATeamPlayerStart::StaticClass(), PlayerStarts);
		TArray<ATeamPlayerStart*> TeamPlayerStarts;
		for(auto Start : PlayerStarts)
		{
			ATeamPlayerStart* TeamPlayerStart = Cast<ATeamPlayerStart>(Start);
			if(TeamPlayerStart && TeamPlayerStart->Team == BlasterPlayerState->GetTeam())
			{
				TeamPlayerStarts.Add(TeamPlayerStart);
			}
		}
		if(TeamPlayerStarts.Num() > 0)
		{
			ATeamPlayerStart* ChosenPlayerStart = TeamPlayerStarts[FMath::RandRange(0, TeamPlayerStarts.Num()-1)];
			SetActorLocationAndRotation(ChosenPlayerStart->GetActorLocation(), ChosenPlayerStart->GetActorRotation());
		}
	}
}

void ABlasterCharacter::OnPlayerStateInitialized()
{
	if(BlasterPlayerState == nullptr) return;
	BlasterPlayerState->AddToScore(0.f);
	BlasterPlayerState->AddToElims(0);
	SetTeamColor(BlasterPlayerState->GetTeam());
	SetSpawnPoint();
}

void ABlasterCharacter::PollInit()
{
	if(BlasterPlayerState == nullptr)
	{
		BlasterPlayerState = GetPlayerState<ABlasterPlayerState>();
		if(BlasterPlayerState)
		{
			OnPlayerStateInitialized();
			
			ABlasterGameState* BlasterGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));
			if(BlasterGameState && BlasterGameState->TopScoringPlayers.Contains(BlasterPlayerState))
			{
				MulticastGainedTheLead();
			}
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

UBuffComponent* ABlasterCharacter::GetBuffComponent() const
{
	return BuffComponent;
}

void ABlasterCharacter::UpdateHUDHealth()
{
	BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;
	if(BlasterPlayerController)
	{
		BlasterPlayerController->SetHUDHealth(Health, MaxHealth);
	}
}

void ABlasterCharacter::UpdateHUDShields()
{
	BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;
	if(BlasterPlayerController)
	{
		BlasterPlayerController->SetHUDShields(Shields, MaxShields);
	}
}

void ABlasterCharacter::UpdateHUDAmmo()
{
	BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;
	if (BlasterPlayerController && CombatComponent && CombatComponent->EquippedWeapon)
	{
		BlasterPlayerController->SetHUDTotalAmmo(CombatComponent->TotalAmmo);
		BlasterPlayerController->SetHUDAmmo(CombatComponent->EquippedWeapon->GetAmmo());
		BlasterPlayerController->SetHUDMagText(CombatComponent->EquippedWeapon->GetMagCapacity());
	}
}

void ABlasterCharacter::MulticastElim_Implementation(bool bPlayerLeftGame)
{
	//When player leaves some elimination functionality is used, but not all
	//This boolean makes sure only the necessary parts of the code are run
	bLeftGame =  bPlayerLeftGame; 
	
	if(BlasterPlayerController)
	{
		BlasterPlayerController->SetHUDAmmo(0);
		BlasterPlayerController->SetHUDMagText(0);
		BlasterPlayerController->SetHUDTotalAmmo(0);
	}
	
	bEliminated = true;
	PlayElimMontage();

	if(DissolveMaterialInstance) //Creating and setting values for the dissolve effect material
	{
		DynamicDissolveMaterialInstance = UMaterialInstanceDynamic::Create(DissolveMaterialInstance, this);

		GetMesh()->SetMaterial(0, DynamicDissolveMaterialInstance);
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), 0.55f);
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("GlowAmount"), 150.f);
	}
	//Starts the dissolve effect using a timeline
	StartDissolve(); 

	//Stopping movement for dead character
	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->StopMovementImmediately();

	bDisableGameplay = true;
	
	if(CombatComponent)//Making sure character cant aim or keep firing when dead
	{
		CombatComponent->FireButtonPressed(false);
		CombatComponent->SetAiming(false);
	}

	//Disabling collision so that the corpse wont hinder movement of other players
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	//Location for the death animation bot effect
	FVector BotSpawnPoint{GetActorLocation() + FVector(0.f, 0.f, 200.f)}; 
	if(ElimBotEffect)
	{
		ElimBotComponent = UGameplayStatics::SpawnEmitterAtLocation(this, ElimBotEffect, BotSpawnPoint, FRotator::ZeroRotator);
	}
	if(BotSoundCue)
	{
		UGameplayStatics::SpawnSoundAtLocation(this, BotSoundCue, BotSpawnPoint);
	}
	if(CrownComponent) //Destroy the crown effect when dead
	{
		CrownComponent->DestroyComponent();
	}
	
	GetWorldTimerManager().SetTimer(ElimTimer, this, &ABlasterCharacter::ElimTimerFinished, ElimDelay);
}


void ABlasterCharacter::Elim(bool bPlayerLeftGame)
{
	//bHoldingTheOrb = false;
	//if(HeldOrb) HeldOrb->Dropped(GetActorLocation());
	//HeldOrb = nullptr;
	
	if(CombatComponent && CombatComponent->EquippedWeapon)
	{
		if(CombatComponent->EquippedWeapon->bDestroyWeapon)
		{
			CombatComponent->EquippedWeapon->Destroy();
		}
		else
		{
			CombatComponent->EquippedWeapon->Dropped();
		}
		
		if(CombatComponent->SecondaryWeapon)
		{
			if(CombatComponent->SecondaryWeapon->bDestroyWeapon)
			{
				CombatComponent->SecondaryWeapon->Destroy();
			}
			else
			{
				CombatComponent->SecondaryWeapon->Dropped();
			}
		}
	}
	if(GetCharacterMovement() && CombatComponent)
	{
		GetCharacterMovement()->MaxWalkSpeed = CombatComponent->BaseWalkSpeed;
	}
	
	MulticastElim(bPlayerLeftGame);
	
	
}

void ABlasterCharacter::ServerLeaveGame_Implementation()
{
	ABlasterGameMode* GameMode = Cast<ABlasterGameMode>(GetWorld()->GetAuthGameMode());
	BlasterPlayerState = BlasterPlayerState == nullptr ? GetPlayerState<ABlasterPlayerState>() : BlasterPlayerState;
	if(GameMode && BlasterPlayerState)
	{
		GameMode->PlayerLeftGame(BlasterPlayerState);
	}
}

void ABlasterCharacter::ElimTimerFinished()
{
	ABlasterGameMode* GameMode = Cast<ABlasterGameMode>(GetWorld()->GetAuthGameMode());
	if(GameMode && !bLeftGame)
	{
		GameMode->RequestRespawn(this, Controller);
	}
	if(bLeftGame && IsLocallyControlled())
	{
		OnLeftGame.Broadcast();
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

void ABlasterCharacter::MulticastGainedTheLead_Implementation()
{
	if(CrownSystem == nullptr) return;
	if(CrownComponent == nullptr)
	{
		CrownComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(CrownSystem, GetMesh(), FName("head"), GetActorLocation() + FVector(0.f, 0.f, 120.f), GetActorRotation(), EAttachLocation::KeepWorldPosition, false);
	}
	if(CrownComponent)
	{
		CrownComponent->Activate();
	}
}

void ABlasterCharacter::MultiCastLostTheLead_Implementation()
{
	if(CrownComponent)
	{
		CrownComponent->DestroyComponent();
	}
}

void ABlasterCharacter::Destroyed()
{
	if(ElimBotComponent)
	{
		ElimBotComponent->DestroyComponent();
	}
	if(DefaultWeapon)
	{
		DefaultWeapon->Destroy(); //TODO: Will destroy weapon even if someone is using it. Will fix later with an overhaul
	}
	
	Super::Destroyed();
}

void ABlasterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ABlasterCharacter, OverlappingWeapon, COND_OwnerOnly);
	DOREPLIFETIME(ABlasterCharacter, Health);
	DOREPLIFETIME(ABlasterCharacter, Shields);
	DOREPLIFETIME(ABlasterCharacter, bDisableGameplay);
	DOREPLIFETIME(ABlasterCharacter, bHoldingTheOrb);
}



