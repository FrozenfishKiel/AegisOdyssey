// Fill out your copyright notice in the Description page of Project Settings.

#include "GE_PersistentStateTags.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GE_PersistentStateTags)

UGE_PersistentStateTags::UGE_PersistentStateTags(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	DurationPolicy = EGameplayEffectDurationType::Infinite;
}
