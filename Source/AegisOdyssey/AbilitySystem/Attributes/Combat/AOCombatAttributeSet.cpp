// Fill out your copyright notice in the Description page of Project Settings.


#include "AOCombatAttributeSet.h"

#include "AbilitySystemComponent.h"
#include "GameplayEffectExtension.h"
#include "AegisOdyssey/Character/AOCharacter.h"
#include "AegisOdyssey/Character/AOCharacterCombatManagerComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "AegisOdyssey/AOLogChannels.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOCombatAttributeSet)

UAOCombatAttributeSet::UAOCombatAttributeSet()
{
	
}

void UAOCombatAttributeSet::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UAOCombatAttributeSet , Attack , COND_None , REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAOCombatAttributeSet , Defense , COND_None , REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAOCombatAttributeSet , Resistance , COND_None , REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAOCombatAttributeSet , MaxSpeed , COND_None , REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAOCombatAttributeSet , MaxVigor , COND_None , REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAOCombatAttributeSet , Vigor , COND_None , REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAOCombatAttributeSet , MaxStamina , COND_None , REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAOCombatAttributeSet , Stamina , COND_None , REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAOCombatAttributeSet , CritChance , COND_None , REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAOCombatAttributeSet , CritDamage , COND_None , REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAOCombatAttributeSet , HitReactTotalThreshold , COND_None , REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAOCombatAttributeSet , CraftingSpeedBonus , COND_None , REPNOTIFY_Always);

}

bool UAOCombatAttributeSet::PreGameplayEffectExecute(struct FGameplayEffectModCallbackData& Data)
{
	if (Data.EvaluatedData.Attribute == GetVigorBonusAttribute())
	{
		const float OldVigor = GetVigor();
		SetVigor(FMath::Clamp(OldVigor + Data.EvaluatedData.Magnitude , 0.f, GetMaxVigor()));
	}
	if (Data.EvaluatedData.Attribute == GetStaminaBonusAttribute())
	{
		const float OldStamina = GetStamina();
		SetStamina(FMath::Clamp(OldStamina + Data.EvaluatedData.Magnitude , 0.f, GetMaxStamina()));
	}
	return Super::PreGameplayEffectExecute(Data);
}

void UAOCombatAttributeSet::PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);
	FEffectProperties Properties;
	SetEffectContext(Data, Properties);
	const FGameplayEffectContextHandle& EffectContext = Data.EffectSpec.GetContext();
	AActor* Instigator = EffectContext.GetOriginalInstigator();
	AActor* Causer = EffectContext.GetEffectCauser();
	if (Data.EvaluatedData.Attribute == GetStaminaAttribute() && GetStamina() <= KINDA_SMALL_NUMBER)
	{
		if (AAOCharacter* OwnerCharacter = Cast<AAOCharacter>(GetOwningActor()))
		{
			if (UAbilitySystemComponent* OwnerASC = OwnerCharacter->GetAbilitySystemComponent())
			{
				if (!OwnerASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(TEXT("State.Combat.Broken"))))
				{
					if (UAOCharacterCombatManagerComponent* CombatManager = OwnerCharacter->FindComponentByClass<UAOCharacterCombatManagerComponent>())
					{
						CombatManager->HandleBrokenState(
							Instigator,
							Causer,
							EffectContext.GetSourceObject());
					}
				}
			}
		}
	}
	(void)Properties;
}

void UAOCombatAttributeSet::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	Super::PreAttributeBaseChange(Attribute, NewValue);
}

void UAOCombatAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
	if (Attribute == GetVigorAttribute())
	{
		///UE_LOG(LogAegisOdysseyAttributeSet, Warning, TEXT("PreAttributeChange - Vigor: OldValue=%f, NewValue=%f, MaxVigor=%f"), GetVigor(), NewValue, GetMaxVigor());
		NewValue = FMath::Clamp(NewValue , 0.f , GetMaxVigor());
	}
	else if (Attribute == GetMaxVigorAttribute())
	{
		NewValue = FMath::Max(NewValue , 1.0f);
	}
	else if (Attribute == GetMaxStaminaAttribute())
	{
		NewValue = FMath::Max(NewValue , 1.0f);
	}
	else if (Attribute == GetStaminaAttribute())
	{
		NewValue = FMath::Clamp(NewValue , 0.f  , GetMaxStamina());
	}
	else if (Attribute == GetHitReactTotalThresholdAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.0f);
	}
}

void UAOCombatAttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);
	
	
	if (Attribute == GetMaxSpeedAttribute() || Attribute == GetSprintSpeedBonusAttribute())
	{
		if (AAOCharacter* AOCharacter = Cast<AAOCharacter>( GetOwningAbilitySystemComponent()->GetAvatarActor()))
		{
			UCharacterMovementComponent* CharacterMovementComponent = AOCharacter->GetCharacterMovement();
			if (CharacterMovementComponent)
			{
				float TotalSpeed = GetMaxSpeed();
				CharacterMovementComponent->MaxWalkSpeed = TotalSpeed;
				float NewMaxWalkSpeed = CharacterMovementComponent->MaxWalkSpeed;
				UE_LOG(LogAegisOdysseyAttributeSet, Log, TEXT("UAOCombatAttributeSet::PostAttributeChange: MaxWalkSpeed  %.2f "), 
					NewMaxWalkSpeed);
			}
		}
	}
}

void UAOCombatAttributeSet::OnRep_Stamina(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAOCombatAttributeSet, Stamina, OldValue);
	OnStaminaChanged.Broadcast(nullptr,nullptr,nullptr,GetStamina() - OldValue.GetCurrentValue(),OldValue.GetCurrentValue(),GetStamina());

}

void UAOCombatAttributeSet::OnRep_MaxStamina(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAOCombatAttributeSet, MaxStamina, OldValue);
	OnMaxStaminaChanged.Broadcast(nullptr,nullptr,nullptr,GetMaxStamina() - OldValue.GetCurrentValue(),OldValue.GetCurrentValue() , GetMaxStamina());

}

void UAOCombatAttributeSet::OnRep_Vigor(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAOCombatAttributeSet, Vigor, OldValue);
	OnVigorChanged.Broadcast(nullptr,nullptr,nullptr,GetVigor() - OldValue.GetCurrentValue(),OldValue.GetCurrentValue(),GetVigor());
}

void UAOCombatAttributeSet::OnRep_MaxVigor(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAOCombatAttributeSet, MaxVigor, OldValue);
	OnMaxVigorChanged.Broadcast(nullptr,nullptr,nullptr,GetMaxVigor() - OldValue.GetCurrentValue() , OldValue.GetCurrentValue() , GetMaxVigor());
}

void UAOCombatAttributeSet::OnRep_VigorBonus()
{
	
}

void UAOCombatAttributeSet::OnRep_CritChance(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAOCombatAttributeSet, CritChance, OldValue);
	OnCritChanceChanged.Broadcast(nullptr,nullptr,nullptr,GetCritChance() - OldValue.GetCurrentValue() , OldValue.GetCurrentValue() , GetCritChance());
}

void UAOCombatAttributeSet::OnRep_CritDamage(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAOCombatAttributeSet, CritDamage, OldValue);
	OnCritDamageChanged.Broadcast(nullptr,nullptr,nullptr,GetCritDamage() - OldValue.GetCurrentValue() , OldValue.GetCurrentValue() , GetCritDamage());
}

void UAOCombatAttributeSet::OnRep_HitReactTotalThreshold(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAOCombatAttributeSet, HitReactTotalThreshold, OldValue);
	OnHitReactTotalThresholdChanged.Broadcast(
		nullptr,
		nullptr,
		nullptr,
		GetHitReactTotalThreshold() - OldValue.GetCurrentValue(),
		OldValue.GetCurrentValue(),
		GetHitReactTotalThreshold());
}

void UAOCombatAttributeSet::OnRep_Attack(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAOCombatAttributeSet, Attack, OldValue);
	OnAttackChanged.Broadcast(nullptr,nullptr,nullptr,GetAttack() - OldValue.GetCurrentValue() , OldValue.GetCurrentValue() , GetAttack());

}

void UAOCombatAttributeSet::OnRep_Defense(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAOCombatAttributeSet, Defense, OldValue);
	OnDefenseChanged.Broadcast(nullptr, nullptr, nullptr, GetDefense() - OldValue.GetCurrentValue(), OldValue.GetCurrentValue(), GetDefense());
}

void UAOCombatAttributeSet::OnRep_Resistance(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAOCombatAttributeSet, Resistance, OldValue);
	OnResistanceChanged.Broadcast(nullptr, nullptr, nullptr, GetResistance() - OldValue.GetCurrentValue(), OldValue.GetCurrentValue(), GetResistance());
}

void UAOCombatAttributeSet::OnRep_MaxSpeed()
{
	UE_LOG(LogAegisOdysseyAttributeSet, Log, TEXT("UAOCombatAttributeSet::OnRep_MaxSpeed: MaxSpeed replicated to %.2f"), GetMaxSpeed());
}

void UAOCombatAttributeSet::OnRep_SprintSpeedBonus()
{

}

void UAOCombatAttributeSet::OnRep_CraftingSpeedBonus(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAOCombatAttributeSet, CraftingSpeedBonus, OldValue);
}

