// Fill out your copyright notice in the Description page of Project Settings.


#include "AOActivatableWidget.h"

#include "ToolMenusEditor.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(AOActivatableWidget)

UAOActivatableWidget::UAOActivatableWidget(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	
}

TOptional<FUIInputConfig> UAOActivatableWidget::GetDesiredInputConfig() const
{
	switch (InputConfig)
	{
	case EAOWidgetInputMode::GameAndMenu:
		return FUIInputConfig(ECommonInputMode::All , GameMouseCaptureMode);
	case EAOWidgetInputMode::Game:
		return FUIInputConfig(ECommonInputMode::Game , GameMouseCaptureMode);
	case EAOWidgetInputMode::Menu:
		return FUIInputConfig(ECommonInputMode::Menu , EMouseCaptureMode::NoCapture);
	case EAOWidgetInputMode::Default:
	default:
		return TOptional<FUIInputConfig>();
	}
}
