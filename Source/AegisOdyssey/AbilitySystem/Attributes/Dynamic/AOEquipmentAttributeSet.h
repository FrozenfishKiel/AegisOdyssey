// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AegisOdyssey/AbilitySystem/Attributes/Base/AOAttributeSet.h"
#include "AOEquipmentAttributeSet.generated.h"

// 正式装备来源属性单独拆到专用 AttributeSet。
// 它同样作为角色常驻默认子对象存在，只承接外部装备来源提供的属性值，
// 最终再由派生属性链汇总到角色最终战斗属性上。
UCLASS()
class AEGISODYSSEY_API UAOEquipmentAttributeSet : public UAOAttributeSet
{
	GENERATED_BODY()

public:
	UAOEquipmentAttributeSet();

	ATTRIBUTE_ACCESSORS(UAOEquipmentAttributeSet, EquipmentAttack);
	ATTRIBUTE_ACCESSORS(UAOEquipmentAttributeSet, EquipmentCritChance);
	ATTRIBUTE_ACCESSORS(UAOEquipmentAttributeSet, EquipmentDefense);
	ATTRIBUTE_ACCESSORS(UAOEquipmentAttributeSet, EquipmentResistance);

protected:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

public:
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_EquipmentAttack, Category = "AO|Equipment", meta = (AllowPrivateAccess = true))
	FGameplayAttributeData EquipmentAttack;
	UFUNCTION()
	void OnRep_EquipmentAttack(const FGameplayAttributeData& OldValue);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_EquipmentCritChance, Category = "AO|Equipment", meta = (AllowPrivateAccess = true))
	FGameplayAttributeData EquipmentCritChance;
	UFUNCTION()
	void OnRep_EquipmentCritChance(const FGameplayAttributeData& OldValue);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_EquipmentDefense, Category = "AO|Equipment", meta = (AllowPrivateAccess = true))
	FGameplayAttributeData EquipmentDefense;
	UFUNCTION()
	void OnRep_EquipmentDefense(const FGameplayAttributeData& OldValue);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_EquipmentResistance, Category = "AO|Equipment", meta = (AllowPrivateAccess = true))
	FGameplayAttributeData EquipmentResistance;
	UFUNCTION()
	void OnRep_EquipmentResistance(const FGameplayAttributeData& OldValue);
};
