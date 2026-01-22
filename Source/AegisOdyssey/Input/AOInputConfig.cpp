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
	for (const FAOInputAction& Action : AbilityInputActions)
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
