// Fill out your copyright notice in the Description page of Project Settings.


#include "AOPlayerCameraManager.h"

#include "Async/TaskGraphInterfaces.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "AOCameraComponent.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(AOPlayerCameraManager)

AAOPlayerCameraManager::AAOPlayerCameraManager(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	DefaultFOV = AEGISODYSSEY_CAMERA_DEFAULT_FOV;
	ViewPitchMin = AEGISODYSSEY_CAMERA_DEFAULT_PITCH_MIN;
	ViewPitchMax = AEGISODYSSEY_CAMERA_DEFAULT_PITCH_MAX;
}

void AAOPlayerCameraManager::UpdateViewTarget(FTViewTarget& OutVT, float DeltaTime)
{
	Super::UpdateViewTarget(OutVT, DeltaTime);
}

void AAOPlayerCameraManager::DisplayDebug(UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplay, float& YL,
	float& YPos)
{
	Super::DisplayDebug(Canvas, DebugDisplay, YL, YPos);
	
	check(Canvas);

	FDisplayDebugManager& DisplayDebugManager = Canvas->DisplayDebugManager;

	DisplayDebugManager.SetFont(GEngine->GetSmallFont());
	DisplayDebugManager.SetDrawColor(FColor::Yellow);
	DisplayDebugManager.DrawString(FString::Printf(TEXT("LyraPlayerCameraManager: %s"), *GetNameSafe(this)));

	Super::DisplayDebug(Canvas, DebugDisplay, YL, YPos);

	const APawn* Pawn = (PCOwner ? PCOwner->GetPawn() : nullptr);

	if (const UAOCameraComponent* CameraComponent = UAOCameraComponent::FindCameraComponent(Pawn))
	{
		CameraComponent->DrawDebug(Canvas);
	}
}
