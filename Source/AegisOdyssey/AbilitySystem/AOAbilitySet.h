// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AOAbilitySystem.h"
#include "Abilities/AOGameplayAbility.h"
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
	TSubclassOf<UAOGameplayAbility> Ability = nullptr;

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
struct FAOAbilitySet_SecondaryGameplayEffect
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameplayEffect> GameplayEffect = nullptr;
	UPROPERTY(EditDefaultsOnly)
	float EffectLevel = 1.f;
};

USTRUCT(BlueprintType)
struct FAOAbilitySet_MetaGameplayEffect
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


USTRUCT(BlueprintType)
struct FAOAbilitySet_GrantedHandles
{
	GENERATED_BODY()

public:

	void AddAbilitySpecHandle(const FGameplayAbilitySpecHandle& Handle);
	void AddGameplayEffectHandle(const FActiveGameplayEffectHandle& Handle);
	void AddAttributeSet(UAttributeSet* Set);

	void TakeFromAbilitySystem(UAOAbilitySystem* AOASC);

protected:

	// Handles to the granted abilities.
	UPROPERTY()
	TArray<FGameplayAbilitySpecHandle> AbilitySpecHandles;

	// Handles to the granted gameplay effects.
	UPROPERTY()
	TArray<FActiveGameplayEffectHandle> GameplayEffectHandles;

	// Pointers to the granted attribute sets
	UPROPERTY()
	TArray<TObjectPtr<UAttributeSet>> GrantedAttributeSets;
};
UCLASS()
class AEGISODYSSEY_API UAOAbilitySet : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
	void GiveToAbilitySystem(UAOAbilitySystem* InAOASC, FAOAbilitySet_GrantedHandles* OutGrantedHandles, UObject* SourceObject = nullptr) const;

public:
	UPROPERTY(EditDefaultsOnly,Category = "Gameplay Abilities")
	TArray<FAOAbilitySet_GameplayAbility> GrantedGameplayAbilities;

	UPROPERTY(EditDefaultsOnly,Category = "Gameplay Effects")
	TArray<FAOAbilitySet_GameplayEffect> GrantedGameplayEffects;
	UPROPERTY(EditDefaultsOnly,Category = "Gameplay Effects")
	TArray<FAOAbilitySet_SecondaryGameplayEffect> SecondaryGameplayEffects;
	UPROPERTY(EditDefaultsOnly,Category = "Gameplay Effects")
	TArray<FAOAbilitySet_MetaGameplayEffect> MetaGameplayEffects;

	UPROPERTY(EditDefaultsOnly,Category = "Gameplay Sets")
	TArray<FAOAbilitySet_AttributeSet> AttributeSets;
};
