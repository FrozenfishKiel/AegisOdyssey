// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/PlayerCameraManager.h"
#include "AOPlayerCameraManager.generated.h"

/**
 * 
 */
#define AEGISODYSSEY_CAMERA_DEFAULT_FOV   (80.0f)
#define AEGISODYSSEY_CAMERA_DEFAULT_PITCH_MIN (180.0f)
#define AEGISODYSSEY_CAMERA_DEFAULT_PITCH_MAX   (90.0f)
UCLASS()
class AEGISODYSSEY_API AAOPlayerCameraManager : public APlayerCameraManager
{
	GENERATED_BODY()

public:
	AAOPlayerCameraManager(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void UpdateViewTarget(FTViewTarget& OutVT, float DeltaTime) override;

	virtual void DisplayDebug(UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplay, float& YL, float& YPos) override;
};
