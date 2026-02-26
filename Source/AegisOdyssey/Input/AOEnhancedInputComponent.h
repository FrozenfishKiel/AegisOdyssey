// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AOInputConfig.h"
#include "EnhancedInputComponent.h"
#include "AOEnhancedInputComponent.generated.h"

class UEnhancedInputLocalPlayerSubsystem;
/**
 * 
 */
UCLASS(Config = Input)
class AEGISODYSSEY_API UAOEnhancedInputComponent : public UEnhancedInputComponent
{
	GENERATED_BODY()
public:
	template<class UserClass, typename FuncType>
	void BindNativeAction(const UAOInputConfig* InputConfig, const FGameplayTag& InputTag, ETriggerEvent TriggerEvent, UserClass* Object, FuncType Func, bool bLogIfNotFound);

	template<class UserClass,typename StartedFuncType ,typename PressedFuncType, typename ReleasedFuncType>
	void BindAbilityActions(const UAOInputConfig* InputConfig, UserClass* Object, StartedFuncType StartedFunc,PressedFuncType PressedFunc, ReleasedFuncType ReleasedFunc, TArray<uint32>& BindHandles);

	void AddInputMappings(const UAOInputConfig* InputConfig, UEnhancedInputLocalPlayerSubsystem* InputSubsystem) const;

	void RemoveInputMappings(const UAOInputConfig* InputConfig, UEnhancedInputLocalPlayerSubsystem* InputSubsystem) const;
};

template <class UserClass, typename FuncType>
void UAOEnhancedInputComponent::BindNativeAction(const UAOInputConfig* InputConfig, const FGameplayTag& InputTag,
	ETriggerEvent TriggerEvent, UserClass* Object, FuncType Func, bool bLogIfNotFound)
{
	//按键输入和Tag绑定，输入等于输出对应的Tag
	check(InputConfig);
	if (const UInputAction* IA = InputConfig->FindNativeInputActionForTag(InputTag,bLogIfNotFound))
	{
		BindAction(IA, TriggerEvent, Object, Func);
	}
}

//输入技能按键绑定
template <class UserClass, typename StartedFuncType, typename PressedFuncType, typename ReleasedFuncType>
void UAOEnhancedInputComponent::BindAbilityActions(const UAOInputConfig* InputConfig, UserClass* Object,
	StartedFuncType StartedFunc,PressedFuncType PressedFunc, ReleasedFuncType ReleasedFunc, TArray<uint32>& BindHandles)
{
	check(InputConfig);

	for (const FAOInputAction& Action : InputConfig->AbilityPressedInputActions)
	{
		if (Action.InputAction && Action.InputTag.IsValid())
		{
			if (PressedFunc)
			{
				BindHandles.Add(BindAction(Action.InputAction, ETriggerEvent::Triggered, Object, PressedFunc, Action.InputTag).GetHandle());
			}
		}
	}
	for (const FAOInputAction& Action : InputConfig->AbilityStartInputActions)
	{
		if (Action.InputAction && Action.InputTag.IsValid())
		{
			if (StartedFunc)
			{
				BindHandles.Add(BindAction(Action.InputAction,ETriggerEvent::Started,Object,StartedFunc, Action.InputTag).GetHandle());
			}
		}
	}
	for (const FAOInputAction& Action : InputConfig->AbilityReleasedInputActions)
	{
		if (Action.InputAction && Action.InputTag.IsValid())
		{
			if (ReleasedFunc)
			{
				BindHandles.Add(BindAction(Action.InputAction, ETriggerEvent::Completed, Object, ReleasedFunc, Action.InputTag).GetHandle());
			}
		}
	}
}
