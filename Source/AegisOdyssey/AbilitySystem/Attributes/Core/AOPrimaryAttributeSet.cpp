// Fill out your copyright notice in the Description page of Project Settings.


#include "AOPrimaryAttributeSet.h"
#include "AegisOdyssey/Character/AOPawnData.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOPrimaryAttributeSet)

UAOPrimaryAttributeSet::UAOPrimaryAttributeSet()
{
}

bool UAOPrimaryAttributeSet::RefreshPrimaryAttributesFromLevel(const UAOPawnData* PawnData, int32 Level) const
{
	if (PawnData == nullptr || Level < 1)
	{
		return false;
	}

	UAOAbilitySystem* AbilitySystemComponent = GetAOAbilitySystemComponent();
	if (AbilitySystemComponent == nullptr)
	{
		return false;
	}

	auto OverridePrimaryAttribute = [AbilitySystemComponent, PawnData, Level](const FGameplayAttribute& Attribute, const FName AttributeName)
	{
		const float NewValue = static_cast<float>(PawnData->GetAttributeValueFromNameAndLevel(AttributeName, Level));
		AbilitySystemComponent->SetNumericAttributeBase(Attribute, NewValue);
	};

	OverridePrimaryAttribute(GetStrengthAttribute(), TEXT("Strength"));
	OverridePrimaryAttribute(GetIntelligenceAttribute(), TEXT("Intelligence"));
	OverridePrimaryAttribute(GetAgilityAttribute(), TEXT("Agility"));
	OverridePrimaryAttribute(GetConstitutionAttribute(), TEXT("Constitution"));
	OverridePrimaryAttribute(GetDexterityAttribute(), TEXT("Dexterity"));
	OverridePrimaryAttribute(GetHitReactThresholdAttribute(), TEXT("HitReactThreshold"));
	return true;
}

void UAOPrimaryAttributeSet::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION_NOTIFY(UAOPrimaryAttributeSet , Strength , COND_None , REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAOPrimaryAttributeSet , Intelligence , COND_None , REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAOPrimaryAttributeSet , Agility , COND_None , REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAOPrimaryAttributeSet , Constitution , COND_None , REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAOPrimaryAttributeSet , Dexterity , COND_None , REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAOPrimaryAttributeSet , HitReactThreshold , COND_None , REPNOTIFY_Always);

}

bool UAOPrimaryAttributeSet::PreGameplayEffectExecute(struct FGameplayEffectModCallbackData& Data)
{
	return Super::PreGameplayEffectExecute(Data);
	
}

void UAOPrimaryAttributeSet::PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);
}

void UAOPrimaryAttributeSet::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	Super::PreAttributeBaseChange(Attribute, NewValue);
}

void UAOPrimaryAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
	if (Attribute == GetHitReactThresholdAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.0f);
	}
}

void UAOPrimaryAttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);
}

void UAOPrimaryAttributeSet::OnRep_Strength(FGameplayAttributeData const& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAOPrimaryAttributeSet, Strength, OldValue);
	OnStrengthChanged.Broadcast(nullptr,nullptr,nullptr,GetStrength() - OldValue.GetCurrentValue(),OldValue.GetCurrentValue(),GetStrength());

}

void UAOPrimaryAttributeSet::OnRep_Intelligence(FGameplayAttributeData const& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAOPrimaryAttributeSet, Intelligence, OldValue);
	OnIntelligenceChanged.Broadcast(nullptr,nullptr,nullptr,GetIntelligence() - OldValue.GetCurrentValue(),OldValue.GetCurrentValue(),GetIntelligence());

}

void UAOPrimaryAttributeSet::OnRep_Agility(FGameplayAttributeData const& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAOPrimaryAttributeSet, Agility, OldValue);
	OnAgilityChanged.Broadcast(nullptr,nullptr,nullptr,GetAgility() - OldValue.GetCurrentValue(),OldValue.GetCurrentValue(),GetAgility());

}

void UAOPrimaryAttributeSet::OnRep_Constitution(FGameplayAttributeData const& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAOPrimaryAttributeSet, Constitution, OldValue);
	OnConstitutionChanged.Broadcast(nullptr,nullptr,nullptr,GetConstitution() - OldValue.GetCurrentValue(),OldValue.GetCurrentValue(),GetConstitution());
}

void UAOPrimaryAttributeSet::OnRep_Dexterity(FGameplayAttributeData const& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAOPrimaryAttributeSet, Dexterity, OldValue);
	OnDexterityChanged.Broadcast(nullptr,nullptr,nullptr,GetDexterity() - OldValue.GetCurrentValue(),OldValue.GetCurrentValue(),GetDexterity());
}

void UAOPrimaryAttributeSet::OnRep_HitReactThreshold(FGameplayAttributeData const& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAOPrimaryAttributeSet, HitReactThreshold, OldValue);
}
