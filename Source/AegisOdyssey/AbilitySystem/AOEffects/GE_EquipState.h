// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "GE_EquipState.generated.h"

/**
 * 
 */
UCLASS()
class AEGISODYSSEY_API UGE_EquipState : public UGameplayEffect
{
	GENERATED_BODY()
public:
	UGE_EquipState(const FObjectInitializer& ObjectInitializer);
public:
};
UCLASS(BlueprintType)
class AEGISODYSSEY_API UCharacterWeaponStateOption : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	static void SetDynamicGrantedTags(AActor* TargetActor, const FGameplayTag& GrantedTags);
};