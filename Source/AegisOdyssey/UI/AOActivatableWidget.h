// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "AOActivatableWidget.generated.h"

/**
 * 
 */
UENUM(BlueprintType)
enum class EAOWidgetInputMode : uint8
{
	Default,
	GameAndMenu,
	Game,
	Menu
};
UCLASS(Abstract,Blueprintable)
class AEGISODYSSEY_API UAOActivatableWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	UAOActivatableWidget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
public:
	virtual TOptional<FUIInputConfig> GetDesiredInputConfig() const override;
protected:
	UPROPERTY(EditDefaultsOnly, Category = Input)
	EAOWidgetInputMode InputConfig = EAOWidgetInputMode::Default;

	UPROPERTY(EditDefaultsOnly, Category = Input)
	EMouseCaptureMode GameMouseCaptureMode = EMouseCaptureMode::CapturePermanently;
};
