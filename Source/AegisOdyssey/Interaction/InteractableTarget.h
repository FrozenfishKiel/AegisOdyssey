// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "InteractionOption.h"
#include "InteractableTarget.generated.h"

// 交互选项构建器。
// 目标对象在 GatherInteractionOptions 中通过它向外填充当前可用的交互选项。
class FInteractionOptionBuilder
{
public:
	FInteractionOptionBuilder(TScriptInterface<IInteractableTarget> InterfaceTargetScope, TArray<FInteractionOption>& InteractOptions)
		: Scope(InterfaceTargetScope)
		, Options(InteractOptions)
	{
	}

	// 将一个交互选项加入结果集，并自动回填当前所属的交互目标。
	void AddInteractionOption(const FInteractionOption& Option)
	{
		FInteractionOption& OptionEntry = Options.Add_GetRef(Option);
		OptionEntry.InteractableTarget = Scope;
	}

private:
	// 当前正在生成交互选项的目标对象。
	TScriptInterface<IInteractableTarget> Scope;

	// 外部传入的交互选项结果数组。
	TArray<FInteractionOption>& Options;
};

// 可交互目标接口。
// 交互能力只负责发现与分发，具体行为由实现该接口的对象自己决定。
UINTERFACE(MinimalAPI, meta = (CannotImplementInterfaceInBlueprint))
class UInteractableTarget : public UInterface
{
	GENERATED_BODY()
};

class AEGISODYSSEY_API IInteractableTarget
{
	GENERATED_BODY()

public:
	// 收集当前目标对外暴露的交互选项。
	virtual void GatherInteractionOptions(FInteractionOptionBuilder& OptionBuilder) = 0;

	// 允许目标在真正执行交互前，对事件数据做最后补充或修正。
	virtual void CustomizeInteractionEventData(const FGameplayTag& InteractionEventTag, FGameplayEventData& InOutEventData) { }

	// 对象侧执行前的校验入口。
	// 例如距离、状态、占用关系等都可以在这里拦截。
	virtual bool CanExecuteInteraction(const FGameplayTag& InteractionEventTag, const FGameplayEventData& EventData) const { return true; }

	// 对象侧真正的交互执行入口。
	// 返回 true 表示对象已经完成处理，不再走旧的能力事件回退链。
	virtual bool ExecuteInteraction(const FGameplayTag& InteractionEventTag, const FGameplayEventData& EventData) { return false; }
};
