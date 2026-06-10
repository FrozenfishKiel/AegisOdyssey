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
// AbilitySet 中授予给角色的能力配置。
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
// AbilitySet 中直接施加到角色身上的 GameplayEffect 配置。
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
// AbilitySet 中的次级 GameplayEffect 配置，通常用于补充额外效果。
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
// AbilitySet 中的元 GameplayEffect 配置，通常用于伤害计算等临时结算链路。
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

	// 返回这批授予结果里第一条有效的 AbilitySpecHandle，供“单能力授予”为主的调用方快捷取句柄。
	// 这样外层系统不需要直接访问内部数组。
	// 边界：这里只保证“第一条有效句柄”，不保证它在多能力授予场景下具有业务上的“主能力”语义；
	// 若一批里有多个能力，调用方仍应自行区分。
	FGameplayAbilitySpecHandle GetPrimaryAbilitySpecHandle() const;

	void TakeFromAbilitySystem(UAOAbilitySystem* AOASC);

	// stack-aware 版本只解决“某些 GameplayEffect 不能整条删，而应该按层回收”的场景。
	// 当前先给正式装备系统使用，不改动武器和技能等其他 AbilitySet 使用方的默认回收语义。
	void TakeFromAbilitySystemStackAware(UAOAbilitySystem* AOASC);

protected:

	// Handles to the granted abilities.
	UPROPERTY()
	TArray<FGameplayAbilitySpecHandle> AbilitySpecHandles;

	// Handles to the granted gameplay effects.
	UPROPERTY()
	TArray<FActiveGameplayEffectHandle> GameplayEffectHandles;
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

	// AttributeSets 代表“这包授予依赖哪些属性集也要一起到位”。
	// 这里的职责只有“确保属性集在当前 ASC 上可用”，
	// 不负责把共享 AttributeSet 实例绑定进某次授予句柄的对称回收语义里。
	UPROPERTY(EditDefaultsOnly,Category = "Gameplay Sets")
	TArray<FAOAbilitySet_AttributeSet> AttributeSets;
};
