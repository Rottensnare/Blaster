// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Actor.h"
#include "JumpPad.generated.h"

UCLASS()
class BLASTER_API AJumpPad : public AActor
{
	GENERATED_BODY()
	
public:	
	AJumpPad();
protected:
	
	virtual void BeginPlay() override;
	UFUNCTION()
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:

	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* JumpPadMesh;

	UPROPERTY(EditAnywhere)
	USphereComponent* OverlapSphere;

	UPROPERTY(EditAnywhere)
	USceneComponent* JumpLocation;

	UPROPERTY(EditAnywhere)
	float ArcCurve{0.5f};

	UPROPERTY(EditAnywhere)
	float LaunchMultiplier{1.42f};

	UPROPERTY(EditDefaultsOnly)
	class USoundCue* LaunchSound;

	UPROPERTY(EditDefaultsOnly)
	float SoundMultiplier{.5f};

	UPROPERTY(EditDefaultsOnly)
	USoundAttenuation* SoundAttenuation;
	
};
