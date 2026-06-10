// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "GE_PersistentStateTags.generated.h"

UCLASS()
class AEGISODYSSEY_API UGE_PersistentStateTags : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UGE_PersistentStateTags(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
};
