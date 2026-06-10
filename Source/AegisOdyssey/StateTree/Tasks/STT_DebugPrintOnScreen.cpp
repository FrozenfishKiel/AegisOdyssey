// Fill out your copyright notice in the Description page of Project Settings.

#include "STT_DebugPrintOnScreen.h"

#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "Misc/Crc.h"
#include "StateTreeExecutionContext.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(STT_DebugPrintOnScreen)

namespace
{
FString FormatFloatValue(const float InValue, const int32 FractionalDigits)
{
	FNumberFormattingOptions FormatOptions;
	FormatOptions.SetMinimumFractionalDigits(FMath::Max(0, FractionalDigits));
	FormatOptions.SetMaximumFractionalDigits(FMath::Max(0, FractionalDigits));
	return FText::AsNumber(InValue, &FormatOptions).ToString();
}
}

EStateTreeRunStatus FSTT_DebugPrintOnScreen::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	InstanceData.TimeUntilNextPrint = 0.0f;
	InstanceData.LastPrintedMessage.Reset();
	InstanceData.ResolvedMessage.Reset();
	InstanceData.bResolvedValue = false;

	if (InstanceData.bPrintOnEnter)
	{
		TryPrint(Context, InstanceData);
	}

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FSTT_DebugPrintOnScreen::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	if (!InstanceData.bPrintOnTick)
	{
		return EStateTreeRunStatus::Running;
	}

	InstanceData.TimeUntilNextPrint -= DeltaTime;
	if (InstanceData.TimeUntilNextPrint > 0.0f)
	{
		return EStateTreeRunStatus::Running;
	}

	TryPrint(Context, InstanceData);
	return EStateTreeRunStatus::Running;
}

bool FSTT_DebugPrintOnScreen::ResolveDebugValueText(const FAOStateTreeDebugPrintValue& DebugValue, FString& OutValueText)
{
	if (!DebugValue.TextValue.IsEmpty())
	{
		OutValueText = DebugValue.TextValue;
		return true;
	}

	if (DebugValue.TagValue.IsValid())
	{
		OutValueText = DebugValue.TagValue.ToString();
		return true;
	}

	if (DebugValue.NameValue != NAME_None)
	{
		OutValueText = DebugValue.NameValue.ToString();
		return true;
	}

	if (DebugValue.ObjectValue != nullptr)
	{
		OutValueText = GetNameSafe(DebugValue.ObjectValue.Get());
		return true;
	}

	if (DebugValue.bUseFloatValue)
	{
		OutValueText = FormatFloatValue(DebugValue.FloatValue, DebugValue.FloatFractionalDigits);
		return true;
	}

	if (DebugValue.bUseIntValue)
	{
		OutValueText = FString::FromInt(DebugValue.IntValue);
		return true;
	}

	if (DebugValue.bUseBoolValue)
	{
		OutValueText = DebugValue.bBoolValue ? TEXT("True") : TEXT("False");
		return true;
	}

	OutValueText.Reset();
	return false;
}

FString FSTT_DebugPrintOnScreen::BuildMessageText(const FAOStateTreeDebugPrintValue& DebugValue, const FString& ValueText)
{
	if (DebugValue.Label.IsEmpty())
	{
		return ValueText;
	}

	return FString::Printf(TEXT("%s: %s"), *DebugValue.Label, *ValueText);
}

int32 FSTT_DebugPrintOnScreen::ResolveScreenMessageKey(const FAOStateTreeDebugPrintValue& DebugValue, const int32 ConfiguredKey)
{
	if (ConfiguredKey != INDEX_NONE)
	{
		return ConfiguredKey;
	}

	const FString KeySource = DebugValue.Label.IsEmpty() ? TEXT("StateTreeDebugPrint") : DebugValue.Label;
	return static_cast<int32>(FCrc::StrCrc32(*KeySource));
}

void FSTT_DebugPrintOnScreen::TryPrint(FStateTreeExecutionContext& Context, FInstanceDataType& InstanceData)
{
	InstanceData.TimeUntilNextPrint = FMath::Max(0.0f, InstanceData.PrintInterval);
	InstanceData.ResolvedMessage.Reset();
	InstanceData.bResolvedValue = false;

	FString ValueText;
	if (!ResolveDebugValueText(InstanceData.DebugValue, ValueText))
	{
		return;
	}

	InstanceData.bResolvedValue = true;
	InstanceData.ResolvedMessage = BuildMessageText(InstanceData.DebugValue, ValueText);

	if (InstanceData.bOnlyPrintWhenChanged && InstanceData.ResolvedMessage.Equals(InstanceData.LastPrintedMessage, ESearchCase::CaseSensitive))
	{
		return;
	}

	const int32 MessageKey = ResolveScreenMessageKey(InstanceData.DebugValue, InstanceData.ScreenMessageKey);
	if (GEngine != nullptr)
	{
		GEngine->AddOnScreenDebugMessage(MessageKey, InstanceData.Duration, InstanceData.TextColor, InstanceData.ResolvedMessage);
	}

	if (InstanceData.bPrintToLog)
	{
		UE_LOG(LogStateTree, Log, TEXT("%s"), *InstanceData.ResolvedMessage);
	}

	InstanceData.LastPrintedMessage = InstanceData.ResolvedMessage;
}
