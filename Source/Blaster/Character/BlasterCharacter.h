// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
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

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void MoveForward(float Value);
	void MoveRight(float Value);
	void Turn(float Value);
	void LookUp(float Value);
	void TurnAtRate(float Value);
	void LookUpAtRate(float Value);

private:

	UPROPERTY(VisibleAnywhere, Category = Camera)
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = Camera)
	class UCameraComponent* CameraComponent;

	//Turn rate for controller
	UPROPERTY(EditAnywhere, Category = Camera)
	float BaseTurnRate;
	UPROPERTY(EditAnywhere, Category = Camera)
	float BaseLookUpRate;

	UPROPERTY(EditAnywhere, Category = Camera, meta = (ClapMin = "0.05,", ClampMax = "5.0", UIMin = "0.05", UIMax = "5.0"))
	float MouseXSensitivity;
	
	UPROPERTY(EditAnywhere, Category = Camera, meta = (ClapMin = "0.05,", ClampMax = "5.0", UIMin = "0.05", UIMax = "5.0"))
	float MouseYSensitivity;

	
};
