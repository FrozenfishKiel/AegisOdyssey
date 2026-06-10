// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "Blueprint/UserWidget.h"
#include "CommonActivatableWidget.h"
#include "InteractionOption.generated.h"

class IInteractableTarget;
class UAbilitySystemComponent;

/**
 * 交互选项描述。
 * 它表示当前目标对象向外暴露出的一个候选交互动作。
 */
USTRUCT(BlueprintType)
struct FInteractionOption
{
	GENERATED_BODY()

public:
	/** 这个选项所属的交互目标，由构建器自动回填。 */
	UPROPERTY(BlueprintReadWrite, Category = "AO|Interaction")
	TScriptInterface<IInteractableTarget> InteractableTarget;

	/** 选项主标题。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AO|Interaction")
	FText Text;

	/** 选项副标题或补充提示。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AO|Interaction")
	FText SubText;

	/** 旧交互链使用的能力类型。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AO|Interaction")
	TSubclassOf<UGameplayAbility> InteractionAbilityToGrant = nullptr;

	/** 运行时解析出的目标交互能力句柄。 */
	UPROPERTY(BlueprintReadOnly, Category = "AO|Interaction")
	FGameplayAbilitySpecHandle TargetInteractionAbilityHandle;

	/** 运行时解析出的目标能力系统组件。 */
	UPROPERTY(BlueprintReadOnly, Category = "AO|Interaction")
	TObjectPtr<UAbilitySystemComponent> TargetAbilitySystem = nullptr;

	/** 这个选项正式执行后希望打开的 UI 类。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AO|Interaction")
	TSoftClassPtr<UCommonActivatableWidget> InteractionWidgetClass;

public:
	FORCEINLINE bool operator==(const FInteractionOption& Other) const
	{
		return InteractableTarget == Other.InteractableTarget &&
			InteractionAbilityToGrant == Other.InteractionAbilityToGrant &&
			TargetAbilitySystem == Other.TargetAbilitySystem &&
			TargetInteractionAbilityHandle == Other.TargetInteractionAbilityHandle &&
			InteractionWidgetClass == Other.InteractionWidgetClass &&
			Text.IdenticalTo(Other.Text) &&
			SubText.IdenticalTo(Other.SubText);
	}

	FORCEINLINE bool operator!=(const FInteractionOption& Other) const
	{
		return !operator==(Other);
	}

	FORCEINLINE bool operator<(const FInteractionOption& Other) const
	{
		return InteractableTarget.GetInterface() < Other.InteractableTarget.GetInterface();
	}
};
