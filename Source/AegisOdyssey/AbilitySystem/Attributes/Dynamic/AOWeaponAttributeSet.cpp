// Fill out your copyright notice in the Description page of Project Settings.

#include "AOWeaponAttributeSet.h"

#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOWeaponAttributeSet)

UAOWeaponAttributeSet::UAOWeaponAttributeSet()
{
}

void UAOWeaponAttributeSet::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UAOWeaponAttributeSet, WeaponAttack, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAOWeaponAttributeSet, WeaponCritChance, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAOWeaponAttributeSet, WeaponCritDamage, COND_None, REPNOTIFY_Always);
}

void UAOWeaponAttributeSet::OnRep_WeaponAttack(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAOWeaponAttributeSet, WeaponAttack, OldValue);
}

void UAOWeaponAttributeSet::OnRep_WeaponCritChance(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAOWeaponAttributeSet, WeaponCritChance, OldValue);
}

void UAOWeaponAttributeSet::OnRep_WeaponCritDamage(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAOWeaponAttributeSet, WeaponCritDamage, OldValue);
}
