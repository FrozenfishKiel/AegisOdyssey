// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemGlobals.h"
#include "AOAbilitySystemGlobals.generated.h"

/**
 * 
 */
UCLASS()
class AEGISODYSSEY_API UAOAbilitySystemGlobals : public UAbilitySystemGlobals
{
	GENERATED_BODY()
public:
	virtual FGameplayEffectContext* AllocGameplayEffectContext() const override;
};
