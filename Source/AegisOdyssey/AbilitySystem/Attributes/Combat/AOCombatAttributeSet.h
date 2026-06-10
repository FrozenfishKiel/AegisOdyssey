// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"
#include "AegisOdyssey/AbilitySystem/Attributes/Base/AOAttributeSet.h"
#include "AOCombatAttributeSet.generated.h"

UCLASS()
class AEGISODYSSEY_API UAOCombatAttributeSet : public UAOAttributeSet
{
	GENERATED_BODY()

public:
	UAOCombatAttributeSet();

	// 这里保留角色常驻战斗属性与最终汇总属性。
	// 武器、正式装备等外部来源属性现在各自放在独立的专用 AttributeSet 上，
	// 最终再通过派生属性链汇总到 Attack / Defense / Resistance / CritChance / CritDamage。
	ATTRIBUTE_ACCESSORS(UAOCombatAttributeSet, Vigor);
	ATTRIBUTE_ACCESSORS(UAOCombatAttributeSet, MaxVigor);
	ATTRIBUTE_ACCESSORS(UAOCombatAttributeSet, Stamina);
	ATTRIBUTE_ACCESSORS(UAOCombatAttributeSet, MaxStamina);

	ATTRIBUTE_ACCESSORS(UAOCombatAttributeSet, Attack);
	ATTRIBUTE_ACCESSORS(UAOCombatAttributeSet, Defense);
	ATTRIBUTE_ACCESSORS(UAOCombatAttributeSet, Resistance);
	ATTRIBUTE_ACCESSORS(UAOCombatAttributeSet, CritChance);
	ATTRIBUTE_ACCESSORS(UAOCombatAttributeSet, CritDamage);
	ATTRIBUTE_ACCESSORS(UAOCombatAttributeSet, HitReactTotalThreshold);

	ATTRIBUTE_ACCESSORS(UAOCombatAttributeSet, MaxSpeed);
	ATTRIBUTE_ACCESSORS(UAOCombatAttributeSet, StaminaBonus);
	ATTRIBUTE_ACCESSORS(UAOCombatAttributeSet, SprintSpeedBonus);
	ATTRIBUTE_ACCESSORS(UAOCombatAttributeSet, VigorBonus);
	ATTRIBUTE_ACCESSORS(UAOCombatAttributeSet, CraftingSpeedBonus);

	mutable FAOAttributeEvent OnVigorChanged;
	mutable FAOAttributeEvent OnMaxVigorChanged;
	mutable FAOAttributeEvent OnStaminaChanged;
	mutable FAOAttributeEvent OnMaxStaminaChanged;
	mutable FAOAttributeEvent OnCritChanceChanged;
	mutable FAOAttributeEvent OnCritDamageChanged;
	mutable FAOAttributeEvent OnAttackChanged;
	mutable FAOAttributeEvent OnDefenseChanged;
	mutable FAOAttributeEvent OnResistanceChanged;
	mutable FAOAttributeEvent OnHitReactTotalThresholdChanged;

protected:
	void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool PreGameplayEffectExecute(struct FGameplayEffectModCallbackData& Data) override;
	virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) override;
	virtual void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;

public:
	UPROPERTY(BlueprintReadOnly, Category = "AO|Stamina", meta = (AllowPrivateAccess = true))
	FGameplayAttributeData StaminaBonus;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Stamina, Category = "AO|Stamina", meta = (AllowPrivateAccess = true))
	FGameplayAttributeData Stamina;
	UFUNCTION()
	void OnRep_Stamina(const FGameplayAttributeData& OldValue);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxStamina, Category = "AO|Stamina", meta = (AllowPrivateAccess = true))
	FGameplayAttributeData MaxStamina;
	UFUNCTION()
	void OnRep_MaxStamina(const FGameplayAttributeData& OldValue);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Vigor, Category = "AO|Vigor", meta = (AllowPrivateAccess = true))
	FGameplayAttributeData Vigor;
	UFUNCTION()
	void OnRep_Vigor(const FGameplayAttributeData& OldValue);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxVigor, Category = "AO|Vigor", meta = (AllowPrivateAccess = true))
	FGameplayAttributeData MaxVigor;
	UFUNCTION()
	void OnRep_MaxVigor(const FGameplayAttributeData& OldValue);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Attack, Category = "AO|Attack", meta = (AllowPrivateAccess = true))
	FGameplayAttributeData Attack;
	UFUNCTION()
	void OnRep_Attack(const FGameplayAttributeData& OldValue);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Defense, Category = "AO|Defense", meta = (AllowPrivateAccess = true))
	FGameplayAttributeData Defense;
	UFUNCTION()
	void OnRep_Defense(const FGameplayAttributeData& OldValue);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Resistance, Category = "AO|Resistance", meta = (AllowPrivateAccess = true))
	FGameplayAttributeData Resistance;
	UFUNCTION()
	void OnRep_Resistance(const FGameplayAttributeData& OldValue);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_CritChance, Category = "AO|CritChance", meta = (AllowPrivateAccess = true))
	FGameplayAttributeData CritChance;
	UFUNCTION()
	void OnRep_CritChance(const FGameplayAttributeData& OldValue);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_CritDamage, Category = "AO|CritDamage", meta = (AllowPrivateAccess = true))
	FGameplayAttributeData CritDamage;
	UFUNCTION()
	void OnRep_CritDamage(const FGameplayAttributeData& OldValue);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_HitReactTotalThreshold, Category = "AO|HitReact", meta = (AllowPrivateAccess = true, ClampMin = "0.0"))
	FGameplayAttributeData HitReactTotalThreshold;
	UFUNCTION()
	void OnRep_HitReactTotalThreshold(const FGameplayAttributeData& OldValue);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxSpeed, Category = "AO|Speed", meta = (AllowPrivateAccess = true))
	FGameplayAttributeData MaxSpeed;
	UFUNCTION()
	void OnRep_MaxSpeed();

	// Bonus 属性只做中间承接，避免把临时加成直接写进本体属性后丢失来源边界。
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_SprintSpeedBonus, Category = "AO|SprintSpeedBonus", meta = (AllowPrivateAccess = true))
	FGameplayAttributeData SprintSpeedBonus;
	UFUNCTION()
	void OnRep_SprintSpeedBonus();

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_VigorBonus, Category = "AO|VigorBonus", meta = (AllowPrivateAccess = true))
	FGameplayAttributeData VigorBonus;
	UFUNCTION()
	void OnRep_VigorBonus();

	// 统一制造总加成。
	// 角色制造先直接读取这个总属性，后续如果工作台、Buff 或装备也要并入，
	// 仍然优先汇总到这里，而不是让制造组件自己散读多个来源。
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_CraftingSpeedBonus, Category = "AO|Crafting", meta = (AllowPrivateAccess = true))
	FGameplayAttributeData CraftingSpeedBonus;
	UFUNCTION()
	void OnRep_CraftingSpeedBonus(const FGameplayAttributeData& OldValue);

};
