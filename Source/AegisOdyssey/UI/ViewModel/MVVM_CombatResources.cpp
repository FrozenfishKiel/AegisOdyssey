#include "MVVM_CombatResources.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MVVM_CombatResources)

UMVVM_CombatResources::UMVVM_CombatResources(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UMVVM_CombatResources::SetHealth(float InHealth)
{
	if (UE_MVVM_SET_PROPERTY_VALUE(Health, InHealth))
	{
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetHealth);
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetHealthPercent);
	}
}

void UMVVM_CombatResources::SetMaxHealth(float InMaxHealth)
{
	if (UE_MVVM_SET_PROPERTY_VALUE(MaxHealth, InMaxHealth))
	{
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetMaxHealth);
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetHealthPercent);
	}
}

void UMVVM_CombatResources::SetVigor(float InVigor)
{
	if (UE_MVVM_SET_PROPERTY_VALUE(Vigor, InVigor))
	{
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetVigor);
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetVigorPercent);
	}
}

void UMVVM_CombatResources::SetMaxVigor(float InMaxVigor)
{
	if (UE_MVVM_SET_PROPERTY_VALUE(MaxVigor, InMaxVigor))
	{
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetMaxVigor);
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetVigorPercent);
	}
}

void UMVVM_CombatResources::SetStamina(float InStamina)
{
	if (UE_MVVM_SET_PROPERTY_VALUE(Stamina, InStamina))
	{
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetStamina);
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetStaminaPercent);
	}
}

void UMVVM_CombatResources::SetMaxStamina(float InMaxStamina)
{
	if (UE_MVVM_SET_PROPERTY_VALUE(MaxStamina, InMaxStamina))
	{
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetMaxStamina);
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetStaminaPercent);
	}
}

float UMVVM_CombatResources::GetHealthPercent() const
{
	return MaxHealth > 0.0f ? Health / MaxHealth : 0.0f;
}

float UMVVM_CombatResources::GetVigorPercent() const
{
	return MaxVigor > 0.0f ? Vigor / MaxVigor : 0.0f;
}

float UMVVM_CombatResources::GetStaminaPercent() const
{
	return MaxStamina > 0.0f ? Stamina / MaxStamina : 0.0f;
}
