// Fill out your copyright notice in the Description page of Project Settings.


#include "GA_DurationRecovery.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AegisOdyssey/AOStateTags.h"
#include "GameplayEffectComponents/TargetTagRequirementsGameplayEffectComponent.h"
#include "AegisOdyssey/AbilitySystem/Attributes/Combat/AOCombatAttributeSet.h"
#include "AegisOdyssey/AbilitySystem/ModMagCal/MMC_CalculateVigor.h"
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Ability_DurationRecovery_Value, "Ability.DurationRecovery.Value");

#include UE_INLINE_GENERATED_CPP_BY_NAME(GA_DurationRecovery)

UGA_DurationRecovery::UGA_DurationRecovery(const FObjectInitializer& ObjectInitializer)
{
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;
	RecoverValue = 0.f;
}

void UGA_DurationRecovery::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	ApplyEffectToAvatarActor();  //给角色施加效果
}

void UGA_DurationRecovery::ApplyCost(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
	Super::ApplyCost(Handle, ActorInfo, ActivationInfo);
}

void UGA_DurationRecovery::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	if (RecoveryEffectHandle.IsValid())
	{
		UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
		if (ASC)
		{
			ASC->RemoveActiveGameplayEffect(RecoveryEffectHandle, 1);
		}
		RecoveryEffectHandle.Invalidate();
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_DurationRecovery::ApplyEffectToAvatarActor()
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC && !RecoveryEffectToApplyClass) return;
	
	FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();
	ContextHandle.AddSourceObject(this);
	const FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(RecoveryEffectToApplyClass,GetAbilityLevel(),ContextHandle);
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle,TAG_Ability_DurationRecovery_Value,RecoverValue);
	RecoveryEffectHandle = ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
}
