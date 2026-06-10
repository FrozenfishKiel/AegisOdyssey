// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AegisOdyssey/AbilitySystem/Attributes/Base/AOAttributeSet.h"
#include "AOWeaponAttributeSet.generated.h"

// 武器来源属性单独拆到专用 AttributeSet。
// 它作为角色常驻默认子对象存在，只承接“当前激活武器”提供的来源数值，
// 不再把这类来源属性直接堆进角色最终战斗属性集里。
UCLASS()
class AEGISODYSSEY_API UAOWeaponAttributeSet : public UAOAttributeSet
{
	GENERATED_BODY()

public:
	UAOWeaponAttributeSet();

	ATTRIBUTE_ACCESSORS(UAOWeaponAttributeSet, WeaponAttack);
	ATTRIBUTE_ACCESSORS(UAOWeaponAttributeSet, WeaponCritChance);
	ATTRIBUTE_ACCESSORS(UAOWeaponAttributeSet, WeaponCritDamage);

protected:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

public:
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_WeaponAttack, Category = "AO|Weapon", meta = (AllowPrivateAccess = true))
	FGameplayAttributeData WeaponAttack;
	UFUNCTION()
	void OnRep_WeaponAttack(const FGameplayAttributeData& OldValue);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_WeaponCritChance, Category = "AO|Weapon", meta = (AllowPrivateAccess = true))
	FGameplayAttributeData WeaponCritChance;
	UFUNCTION()
	void OnRep_WeaponCritChance(const FGameplayAttributeData& OldValue);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_WeaponCritDamage, Category = "AO|Weapon", meta = (AllowPrivateAccess = true))
	FGameplayAttributeData WeaponCritDamage;
	UFUNCTION()
	void OnRep_WeaponCritDamage(const FGameplayAttributeData& OldValue);
};
