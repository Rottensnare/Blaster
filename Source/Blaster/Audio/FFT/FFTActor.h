// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FFTActor.generated.h"

UCLASS()
class BLASTER_API AFFTActor : public AActor
{
	GENERATED_BODY()
	
public:	
	AFFTActor();
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

private:

	UPROPERTY(VisibleAnywhere)
	class UAudioComponent* AudioComponent;

	UPROPERTY(EditAnywhere)
	class USoundCue* CustomSoundCue;

	UPROPERTY(VisibleAnywhere)
	TArray<float> FrequenciesToGet;

	UPROPERTY(VisibleAnywhere)
	TArray<FSoundWaveSpectralData> SpectralData;

	TMap<uint32, float> TempMap;

};
