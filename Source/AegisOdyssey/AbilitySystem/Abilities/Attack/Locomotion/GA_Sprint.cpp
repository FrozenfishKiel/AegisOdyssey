// Fill out your copyright notice in the Description page of Project Settings.


#include "GA_Sprint.h"

#include "AegisOdyssey/AbilitySystem/Attributes/AOCombatAttributeSet.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GA_Sprint)

UGA_Sprint::UGA_Sprint(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	SprintSpeedBonusAmount = 0.f;
}

void UGA_Sprint::CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                               const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility)
{
	Super::CancelAbility(Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility);
}

bool UGA_Sprint::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags,
	FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!AvatarActor)
	{
		return false;
	}

	return true;
}

void UGA_Sprint::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (ASC)
	{
		UGameplayEffect* NewGE = NewObject<UGameplayEffect>(this);
		NewGE->DurationPolicy = EGameplayEffectDurationType::Infinite;
        
		FGameplayModifierInfo ModifierInfo;
		ModifierInfo.Attribute = UAOCombatAttributeSet::GetSprintSpeedBonusAttribute();
		ModifierInfo.ModifierOp = EGameplayModOp::Additive;
		ModifierInfo.ModifierMagnitude = FScalableFloat(SprintSpeedBonusAmount);
        
		NewGE->Modifiers.Add(ModifierInfo);
        
		FGameplayEffectSpec Spec(NewGE, MakeEffectContext(Handle, ActorInfo));
		SprintSpeedEffectHandle = ASC->ApplyGameplayEffectSpecToSelf(Spec);
	}

	if (!WaitInputReleaseTask)
	{
		WaitInputReleaseTask = UAbilityTask_WaitInputRelease::WaitInputRelease(this);
		WaitInputReleaseTask->OnRelease.AddDynamic(this, &UGA_Sprint::OnInputReleased);
		WaitInputReleaseTask->ReadyForActivation();
	}
}

void UGA_Sprint::OnInputReleased(float TimeHeld)
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
void UGA_Sprint::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (SprintSpeedEffectHandle.IsValid())
	{
		UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
		if (ASC)
		{
			ASC->RemoveActiveGameplayEffect(SprintSpeedEffectHandle, 1);
		}
		SprintSpeedEffectHandle.Invalidate();
	}
	
	if (WaitInputReleaseTask)
	{
		WaitInputReleaseTask->EndTask();
		WaitInputReleaseTask = nullptr;
	}
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
