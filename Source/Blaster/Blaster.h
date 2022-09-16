// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

//Surface type definitions for footsteps
#define EPS_Tile EPhysicalSurface::SurfaceType1
#define EPS_Metal EPhysicalSurface::SurfaceType2
#define EPS_Stone EPhysicalSurface::SurfaceType3
#define EPS_Wood EPhysicalSurface::SurfaceType4
#define EPS_Water EPhysicalSurface::SurfaceType5
#define EPS_Grass EPhysicalSurface::SurfaceType6

//Trace channel definitions for skeletal mesh and SSR hit boxes
#define ECC_SkeletalMesh ECollisionChannel::ECC_GameTraceChannel1
#define ECC_HitBox ECollisionChannel::ECC_GameTraceChannel2

//TODO: Create an array of random vectors to use