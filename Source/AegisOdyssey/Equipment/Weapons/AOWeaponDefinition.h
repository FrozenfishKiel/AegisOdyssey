// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AegisOdyssey/Equipment/AOEquipmentDefinition.h"
#include "AegisOdyssey/Character/AOAnimStateData.h"
#include "GameplayTagContainer.h"
#include "AOWeaponDefinition.generated.h"

class UAOAnimStateData;
class UAOAttackEffectProfile;
/**
 * 
 */
UCLASS()
class AEGISODYSSEY_API UAOWeaponDefinition : public UAOEquipmentDefinition
{
	GENERATED_BODY()
public:
	const virtual UAOAnimStateData* GetWeaponAnimStateData() const {return WeaponAnimStateData;}
	float GetAIAttackRange() const { return AIAttackRange; }
	float GetWeaponAttack() const { return WeaponAttack; }
	float GetWeaponCritChance() const { return WeaponCritChance; }
	float GetWeaponCritDamage() const { return WeaponCritDamage; }
	const FGameplayTag& GetWeaponTag() const { return WeaponTag; }
	const FGameplayTagContainer& GetDamageTypeTags() const { return DamageTypeTags; }
	const UAOAttackEffectProfile* GetDefaultAttackEffectProfile() const { return DefaultAttackEffectProfile; }
protected:
	// 仅供 AI 决策使用的攻击距离，不参与现有普攻/技能功能链。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	float AIAttackRange = 100.0f;

	// 武器在战斗系统中的统一身份标签。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	FGameplayTag WeaponTag;

	// 武器直接提供给总攻击力计算链的攻击值。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	float WeaponAttack = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	float WeaponCritChance = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	float WeaponCritDamage = 0.0f;

	// 武器默认附带的伤害类型标签。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	FGameplayTagContainer DamageTypeTags;

	// 只作为默认攻击表现编排入口，不承载命中/伤害真值。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Presentation")
	TObjectPtr<UAOAttackEffectProfile> DefaultAttackEffectProfile = nullptr;

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly)
	TObjectPtr<UAOAnimStateData> WeaponAnimStateData;  //只读属性
};
