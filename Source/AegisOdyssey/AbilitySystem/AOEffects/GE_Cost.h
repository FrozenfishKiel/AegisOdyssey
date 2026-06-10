// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "GE_Cost.generated.h"

/**
 * 
 */
UCLASS()
class AEGISODYSSEY_API UGE_Cost : public UGameplayEffect
{
	GENERATED_BODY()
public:
	UGE_Cost(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
};
