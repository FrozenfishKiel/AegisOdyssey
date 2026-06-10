#include "MVVM_TargetHealthBar.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MVVM_TargetHealthBar)

UMVVMTargetHealthBar::UMVVMTargetHealthBar(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UMVVMTargetHealthBar::SetTargetActor(AActor* InTargetActor)
{
	if (UE_MVVM_SET_PROPERTY_VALUE(TargetActor, InTargetActor))
	{
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetTargetActor);
	}
}

void UMVVMTargetHealthBar::SetCurrentHealth(float InCurrentHealth)
{
	if (UE_MVVM_SET_PROPERTY_VALUE(CurrentHealth, InCurrentHealth))
	{
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetCurrentHealth);
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetHealthPercent);
	}
}

void UMVVMTargetHealthBar::SetMaxHealth(float InMaxHealth)
{
	if (UE_MVVM_SET_PROPERTY_VALUE(MaxHealth, InMaxHealth))
	{
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetMaxHealth);
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetHealthPercent);
	}
}

float UMVVMTargetHealthBar::GetHealthPercent() const
{
	return MaxHealth > 0.0f ? CurrentHealth / MaxHealth : 0.0f;
}

void UMVVMTargetHealthBar::SetDead(bool bInDead)
{
	if (UE_MVVM_SET_PROPERTY_VALUE(bDead, bInDead))
	{
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(IsDead);
	}
}

void UMVVMTargetHealthBar::SetVisible(bool bInVisible)
{
	if (UE_MVVM_SET_PROPERTY_VALUE(bVisible, bInVisible))
	{
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(IsVisible);
	}
}
