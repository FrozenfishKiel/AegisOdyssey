// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "StateTreeTaskBase.h"
#include "STT_DebugPrintOnScreen.generated.h"

class UObject;

USTRUCT(BlueprintType)
struct FAOStateTreeDebugPrintValue
{
	GENERATED_BODY()

	// 打印时显示在前面的名字，例如 "SelectedIntentTag"。
	UPROPERTY(EditAnywhere, Category = "Config")
	FString Label;

	// 文本值。
	// 如果这里绑定了非空字符串，会优先打印它。
	UPROPERTY(EditAnywhere, Category = "Value")
	FString TextValue;

	// 标签值。
	// 如果文本为空，且这里绑定了有效标签，就打印标签名。
	UPROPERTY(EditAnywhere, Category = "Value")
	FGameplayTag TagValue;

	// 名字值。
	UPROPERTY(EditAnywhere, Category = "Value")
	FName NameValue = NAME_None;

	// 对象值。
	// 如果前面的值都没有，就打印对象名。
	UPROPERTY(EditAnywhere, Category = "Value")
	TObjectPtr<const UObject> ObjectValue = nullptr;

	// 浮点值。
	// 因为 0 也是合法值，所以是否打印它由 bUseFloatValue 决定。
	UPROPERTY(EditAnywhere, Category = "Value")
	float FloatValue = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Config")
	bool bUseFloatValue = false;

	// 整数值。
	UPROPERTY(EditAnywhere, Category = "Value")
	int32 IntValue = 0;

	UPROPERTY(EditAnywhere, Category = "Config")
	bool bUseIntValue = false;

	// 布尔值。
	UPROPERTY(EditAnywhere, Category = "Value")
	bool bBoolValue = false;

	UPROPERTY(EditAnywhere, Category = "Config")
	bool bUseBoolValue = false;

	// 浮点保留的小数位数。
	UPROPERTY(EditAnywhere, Category = "Config", meta = (ClampMin = "0", UIMin = "0"))
	int32 FloatFractionalDigits = 2;
};

USTRUCT()
struct FSTT_DebugPrintOnScreenInstanceData
{
	GENERATED_BODY()

	// 要打印的值。
	// 一个 Task 只负责一条调试信息；如果你要同时看多个值，就在 Global Task 里放多个这个 Task。
	UPROPERTY(EditAnywhere, Category = "Config")
	FAOStateTreeDebugPrintValue DebugValue;

	// 进入状态时是否立刻打印一次。
	UPROPERTY(EditAnywhere, Category = "Config")
	bool bPrintOnEnter = true;

	// 之后是否按间隔持续打印。
	UPROPERTY(EditAnywhere, Category = "Config")
	bool bPrintOnTick = true;

	// 打印间隔，单位秒。
	UPROPERTY(EditAnywhere, Category = "Config", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float PrintInterval = 0.1f;

	// 屏幕显示时长，单位秒。
	UPROPERTY(EditAnywhere, Category = "Config", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float Duration = 0.15f;

	// 屏幕文字颜色。
	UPROPERTY(EditAnywhere, Category = "Config")
	FColor TextColor = FColor::Green;

	// 是否只在文本变化时才重新打印。
	UPROPERTY(EditAnywhere, Category = "Config")
	bool bOnlyPrintWhenChanged = false;

	// 是否同时写到日志。
	UPROPERTY(EditAnywhere, Category = "Config")
	bool bPrintToLog = false;

	// 屏幕消息 Key。
	// INDEX_NONE 表示自动根据 Label 生成；如果你想手动控制覆盖哪一行，可以自己填。
	UPROPERTY(EditAnywhere, Category = "Config")
	int32 ScreenMessageKey = INDEX_NONE;

	// 输出：最近一次真正打印到屏幕上的完整文本。
	UPROPERTY(VisibleAnywhere, Category = "Output")
	FString LastPrintedMessage;

	// 输出：本次解析出来的完整文本。
	UPROPERTY(VisibleAnywhere, Category = "Output")
	FString ResolvedMessage;

	// 输出：本次是否成功解析出可打印的内容。
	UPROPERTY(VisibleAnywhere, Category = "Output")
	bool bResolvedValue = false;

	// 运行时节流计时。
	UPROPERTY(Transient)
	float TimeUntilNextPrint = 0.0f;
};

USTRUCT(DisplayName = "Debug Print On Screen", Category = "AegisOdyssey|Debug")
struct AEGISODYSSEY_API FSTT_DebugPrintOnScreen : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FSTT_DebugPrintOnScreenInstanceData;

	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;

private:
	static bool ResolveDebugValueText(const FAOStateTreeDebugPrintValue& DebugValue, FString& OutValueText);
	static FString BuildMessageText(const FAOStateTreeDebugPrintValue& DebugValue, const FString& ValueText);
	static int32 ResolveScreenMessageKey(const FAOStateTreeDebugPrintValue& DebugValue, int32 ConfiguredKey);
	static void TryPrint(FStateTreeExecutionContext& Context, FInstanceDataType& InstanceData);
};
