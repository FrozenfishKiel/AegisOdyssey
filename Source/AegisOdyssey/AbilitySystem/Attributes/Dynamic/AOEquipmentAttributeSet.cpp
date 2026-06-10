// Fill out your copyright notice in the Description page of Project Settings.

#include "AOEquipmentAttributeSet.h"

#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOEquipmentAttributeSet)

UAOEquipmentAttributeSet::UAOEquipmentAttributeSet()
{
}

void UAOEquipmentAttributeSet::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UAOEquipmentAttributeSet, EquipmentAttack, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAOEquipmentAttributeSet, EquipmentCritChance, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAOEquipmentAttributeSet, EquipmentDefense, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAOEquipmentAttributeSet, EquipmentResistance, COND_None, REPNOTIFY_Always);
}

void UAOEquipmentAttributeSet::OnRep_EquipmentAttack(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAOEquipmentAttributeSet, EquipmentAttack, OldValue);
}

void UAOEquipmentAttributeSet::OnRep_EquipmentCritChance(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAOEquipmentAttributeSet, EquipmentCritChance, OldValue);
}

void UAOEquipmentAttributeSet::OnRep_EquipmentDefense(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAOEquipmentAttributeSet, EquipmentDefense, OldValue);
}

void UAOEquipmentAttributeSet::OnRep_EquipmentResistance(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAOEquipmentAttributeSet, EquipmentResistance, OldValue);
}
