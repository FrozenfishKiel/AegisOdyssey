// Fill out your copyright notice in the Description page of Project Settings.


#include "GA_LightAttack.h"
#include "AegisOdyssey/StateTree/AOStateTreeComponentBase.h"
#include "AegisOdyssey/StateTree/CombatStateTree/AOCombatStateTree.h"
#include "AegisOdyssey/Character/AOCharacter.h"
#include "AegisOdyssey/Animation/AOAnimInstance.h"
#include "AbilitySystemGlobals.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GA_LightAttack)

UGA_LightAttack::UGA_LightAttack(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	Params = nullptr;

	FAbilityTriggerData TriggerData;
	TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	//TriggerData.TriggerTag = FGameplayTag::RequestGameplayTag(FName("InputTag.LightAttack"), true);
	AbilityTriggers.Add(TriggerData);
}

bool UGA_LightAttack::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                         const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags,
                                         const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
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

void UGA_LightAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                      const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                      const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (TriggerEventData && TriggerEventData->OptionalObject)
	{
		Params = Cast<ULightAttackParams>(const_cast<UObject*>(TriggerEventData->OptionalObject.Get()));
	}
	else if (TriggerEventData && TriggerEventData->OptionalObject2)
	{
		Params = Cast<ULightAttackParams>(const_cast<UObject*>(TriggerEventData->OptionalObject2.Get()));
	}
	else
	{
		EndAbility(CurrentSpecHandle,CurrentActorInfo,CurrentActivationInfo,true,true);
	}

	PlayMontageAnimation();
}

void UGA_LightAttack::PlayMontageAnimation()
{

	UAnimMontage* MontageToPlay = Params ? Params->Montage : nullptr;
	float PlayRateValue = Params ? Params->PlayRate : 1.0f;
	FName StartSectionValue = Params ? Params->StartSection : NAME_None;
	float StartTimeValue = Params ? Params->StartTime : 0.0f;

	MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		FName("PlayMontageAndWait"),
		MontageToPlay,
		PlayRateValue,
		StartSectionValue,
		true,
		1.0f,
		StartTimeValue,
		false
	);

	if (MontageTask)
	{
		MontageTask->OnBlendOut.AddDynamic(this, &UGA_LightAttack::OnMontageBlendedOut);
		MontageTask->OnCompleted.AddDynamic(this, &UGA_LightAttack::OnMontageCompleted);
		MontageTask->OnInterrupted.AddDynamic(this, &UGA_LightAttack::OnMontageInterrupted);
		MontageTask->OnCancelled.AddDynamic(this, &UGA_LightAttack::OnMontageCancelled);
		MontageTask->ReadyForActivation();
	}
}

void UGA_LightAttack::OnMontageCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_LightAttack::OnMontageBlendedOut()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_LightAttack::OnMontageInterrupted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UGA_LightAttack::OnMontageCancelled()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}
