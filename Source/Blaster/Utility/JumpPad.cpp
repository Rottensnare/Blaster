// Fill out your copyright notice in the Description page of Project Settings.


#include "JumpPad.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

// Sets default values
AJumpPad::AJumpPad()
{
	PrimaryActorTick.bCanEverTick = false;

	JumpPadMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("JumpPadMesh"));
	SetRootComponent(JumpPadMesh);

	OverlapSphere = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	OverlapSphere->SetupAttachment(GetRootComponent());

	JumpLocation = CreateDefaultSubobject<USceneComponent>(TEXT("JumpLocation"));
	JumpLocation->SetupAttachment(GetRootComponent());

}


void AJumpPad::BeginPlay()
{
	Super::BeginPlay();

	OverlapSphere->OnComponentBeginOverlap.AddDynamic(this, &AJumpPad::OnSphereOverlap);
	
}

void AJumpPad::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if(OtherActor == nullptr || JumpLocation == nullptr) return;
	
	FVector OutVelocity{FVector(0.f)};
	
	UGameplayStatics::SuggestProjectileVelocity_CustomArc(this, OutVelocity, OtherActor->GetActorLocation(), JumpLocation->GetComponentLocation(), 0, ArcCurve);
	
	if(ACharacter* TempCharacter = Cast<ACharacter>(OtherActor))
	{
		TempCharacter->LaunchCharacter(OutVelocity * LaunchMultiplier, true, true);
		TempCharacter->GetCharacterMovement()->MovementMode = EMovementMode::MOVE_Flying;
		if(LaunchSound == nullptr || SoundAttenuation == nullptr) return;
		UGameplayStatics::PlaySoundAtLocation(this, LaunchSound, GetActorLocation(), GetActorRotation(), SoundMultiplier, 1, 0, SoundAttenuation);
	}
}

