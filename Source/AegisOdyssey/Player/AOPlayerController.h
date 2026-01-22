// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonPlayerController.h"
#include "ModularPlayerController.h"
#include "AOPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class AEGISODYSSEY_API AAOPlayerController : public ACommonPlayerController
{
	GENERATED_BODY()
public:
	virtual void PostProcessInput(const float DeltaTime, const bool bGamePaused) override;
};
