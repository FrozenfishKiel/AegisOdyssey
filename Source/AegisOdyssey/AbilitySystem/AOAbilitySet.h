// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AOAbilitySystem.h"
#include "Engine/DataAsset.h"
#include "AOAbilitySet.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct FAOAbilitySet_GameplayAbility
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UAOAbilitySystem> Ability = nullptr;

	UPROPERTY(EditDefaultsOnly)
	int32 AbilityLevel = 1;

	UPROPERTY(EditDefaultsOnly,Meta = (Categories = "InputTag"))
	FGameplayTag InputTag;
	
};

USTRUCT(BlueprintType)
struct FAOAbilitySet_GameplayEffect
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameplayEffect> GameplayEffect = nullptr;

	UPROPERTY(EditDefaultsOnly)
	float EffectLevel = 1.f;
};

USTRUCT(BlueprintType)
struct FAOAbilitySet_AttributeSet
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UAttributeSet> AttributeSet;
};

UCLASS()
class AEGISODYSSEY_API UAOAbilitySet : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly,Category = "Gameplay Abilities")
	TArray<FAOAbilitySet_GameplayAbility> GrantedGameplayAbilities;

	UPROPERTY(EditDefaultsOnly,Category = "Gameplay Effects")
	TArray<FAOAbilitySet_GameplayEffect> GrantedGameplayEffects;

	UPROPERTY(EditDefaultsOnly,Category = "Gameplay Sets")
	TArray<FAOAbilitySet_AttributeSet> AttributeSets;
};
