// Fill out your copyright notice in the Description page of Project Settings.


#include "AOInputConfig.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(AOInputConfig)
const UInputAction* UAOInputConfig::FindNativeInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound) const
{
	for (const FAOInputAction& Action : NativeInputActions)
	{
		if (Action.InputAction && (Action.InputTag == InputTag))
		{
			return Action.InputAction;
		}
	}

	if (bLogNotFound)
	{
		
	}

	return nullptr;
}

const UInputAction* UAOInputConfig::FindAbilityInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound) const
{
	for (const FAOInputAction& Action : AbilityPressedInputActions)
	{
		if (Action.InputAction && (Action.InputTag == InputTag))
		{
			return Action.InputAction;
		}
	}

	// 技能槽键位显示不能只看 Pressed 表。
	// 项目里某些输入可能只配置在 Start / Released 表里，
	// 如果这里不一起查，UI 就会把明明存在的技能输入错误显示成 None。
	for (const FAOInputAction& Action : AbilityStartInputActions)
	{
		if (Action.InputAction && (Action.InputTag == InputTag))
		{
			return Action.InputAction;
		}
	}

	for (const FAOInputAction& Action : AbilityReleasedInputActions)
	{
		if (Action.InputAction && (Action.InputTag == InputTag))
		{
			return Action.InputAction;
		}
	}

	if (bLogNotFound)
	{
		
	}

	return nullptr;
}

const UInputAction* UAOInputConfig::FindInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound) const
{
	// 先按 Ability 输入查。
	// 技能槽当前属于这一路，因此它们的 InputTag 最先应该在这里命中。
	if (const UInputAction* AbilityInputAction = FindAbilityInputActionForTag(InputTag, false))
	{
		return AbilityInputAction;
	}

	// 再兜底查 Native 输入。
	// 这样别的 UI 如果未来也想把某些 Native 输入标签直接显示出来，
	// 也可以复用这一个统一查询入口。
	if (const UInputAction* NativeInputAction = FindNativeInputActionForTag(InputTag, false))
	{
		return NativeInputAction;
	}

	if (bLogNotFound)
	{
	}

	return nullptr;
}
