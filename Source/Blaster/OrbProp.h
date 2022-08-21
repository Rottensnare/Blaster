// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "OrbProp.generated.h"

UCLASS()
class BLASTER_API AOrbProp : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AOrbProp();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


private:
	//Testing for Git
	int32 TestInt{10};
	

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
