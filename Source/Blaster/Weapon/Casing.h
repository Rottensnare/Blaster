// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Casing.generated.h"

UCLASS()
class BLASTER_API ACasing : public AActor
{
	GENERATED_BODY()
	
public:	

	ACasing();

	//Plays sound at location, but only once per casing
	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& HitResult);

protected:

	//Binds OnHit to OnComponentHit, adds semi-random impulse vector and sets lifespan to 5 seconds
	virtual void BeginPlay() override; 
	
private:

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* CasingMesh;

	UPROPERTY(EditAnywhere)
	float ShellImpulse; //Impulse strength

	UPROPERTY(EditAnywhere)
	class USoundCue* ShellHitCue;

	bool bSoundPlayedAlready{false};

	

};
