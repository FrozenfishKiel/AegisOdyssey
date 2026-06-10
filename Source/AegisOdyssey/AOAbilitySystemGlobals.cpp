// Fill out your copyright notice in the Description page of Project Settings.


#include "AOAbilitySystemGlobals.h"

#include "AOAbilityTypes.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOAbilitySystemGlobals)

FGameplayEffectContext* UAOAbilitySystemGlobals::AllocGameplayEffectContext() const
{
	return new FAOGameplayEffectContext();  //生成我们新的自定义的Context结构体。
}
