#include "MVVM_LocalCombatState.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MVVM_LocalCombatState)

UMVVM_LocalCombatState::UMVVM_LocalCombatState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UMVVM_LocalCombatState::SetInCombat(bool bInInCombat)
{
	if (UE_MVVM_SET_PROPERTY_VALUE(bInCombat, bInInCombat))
	{
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(IsInCombat);
	}
}

void UMVVM_LocalCombatState::SetBlocking(bool bInBlocking)
{
	if (UE_MVVM_SET_PROPERTY_VALUE(bBlocking, bInBlocking))
	{
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(IsBlocking);
	}
}

void UMVVM_LocalCombatState::SetBroken(bool bInBroken)
{
	if (UE_MVVM_SET_PROPERTY_VALUE(bBroken, bInBroken))
	{
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(IsBroken);
	}
}

void UMVVM_LocalCombatState::SetParried(bool bInParried)
{
	if (UE_MVVM_SET_PROPERTY_VALUE(bParried, bInParried))
	{
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(IsParried);
	}
}

void UMVVM_LocalCombatState::SetAbilityInputBlocked(bool bInAbilityInputBlocked)
{
	if (UE_MVVM_SET_PROPERTY_VALUE(bAbilityInputBlocked, bInAbilityInputBlocked))
	{
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(IsAbilityInputBlocked);
	}
}

void UMVVM_LocalCombatState::SetLastResultType(EAOCombatResultType InLastResultType)
{
	if (UE_MVVM_SET_PROPERTY_VALUE(LastResultType, InLastResultType))
	{
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetLastResultType);
	}
}
