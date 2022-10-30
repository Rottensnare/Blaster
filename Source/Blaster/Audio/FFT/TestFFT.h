// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
class BLASTER_API TestFFT
{
public:
	TestFFT();
	~TestFFT();
private:

	UPROPERTY(EditDefaultsOnly)
	class UAudioComponent* AudioComponent;
	
};
