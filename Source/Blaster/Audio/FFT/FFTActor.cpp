// Fill out your copyright notice in the Description page of Project Settings.


#include "FFTActor.h"

#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"

AFFTActor::AFFTActor()
{
	PrimaryActorTick.bCanEverTick = true;
	AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
	FrequenciesToGet = {100.f, 500.f, 1000.f, 5000.f};

}

void AFFTActor::BeginPlay()
{
	Super::BeginPlay();
	if(AudioComponent == nullptr || CustomSoundCue == nullptr) return;
	/*TempMap.Emplace(0, 1.f);
	AudioComponent->SetSound( TObjectPtr<USoundCue>(CustomSoundCue));
	AudioComponent->Activate(true);
	AudioComponent->Play();
	AudioComponent->SetPlaybackTimes(TempMap);
	UE_LOG(LogTemp, Warning, TEXT("Has Cooked FFT Data: %i"), AudioComponent->HasCookedFFTData());
	//AudioComponent->GetCookedFFTData(FrequenciesToGet, SpectralData);
	*/
	
}

void AFFTActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	bool bGotCookedFFTData = AudioComponent->GetCookedFFTData(FrequenciesToGet, SpectralData);
	//UE_LOG(LogTemp, Warning, TEXT("Cooked FFT Data Fetched: %i"), bGotCookedFFTData);
	//UE_LOG(LogTemp, Warning, TEXT("AudioComponent is playing: %i"), AudioComponent->IsPlaying())
	
}

