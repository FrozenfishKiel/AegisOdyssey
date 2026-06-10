// Fill out your copyright notice in the Description page of Project Settings.

#include "AOFragment_Consumable.h"

#include "GameplayEffect.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOFragment_Consumable)

bool UAOFragment_Consumable::HasUsableEffect() const
{
	if (!bAllowUseFromInventory)
	{
		return false;
	}

	for (const TSubclassOf<UGameplayEffect>& EffectClass : EffectsToApply)
	{
		if (EffectClass != nullptr)
		{
			return true;
		}
	}

	return false;
}
