// Fill out your copyright notice in the Description page of Project Settings.


#include "TestFFT.h"
#include "Components/AudioComponent.h"

TestFFT::TestFFT():
AudioComponent(nullptr)
{
	TArray<float> FrequenciesToGet;
	TArray<FSoundWaveSpectralData> SpectralData;
	AudioComponent->GetCookedFFTData(FrequenciesToGet, SpectralData);
}

TestFFT::~TestFFT()
{
}
