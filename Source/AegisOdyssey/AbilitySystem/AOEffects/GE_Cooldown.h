// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "GE_Cooldown.generated.h"

/**
 * 
 */
UCLASS()
class AEGISODYSSEY_API UGE_Cooldown : public UGameplayEffect
{
	GENERATED_BODY()
public:
	UGE_Cooldown(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
};
