#include "STT_PlayHeavyAttack.h"

#include "Abilities/GameplayAbility.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemGlobals.h"
#include "GameplayAbilitySpec.h"
#include "GameplayPrediction.h"
#include "StateTreeExecutionContext.h"
#include "AegisOdyssey/AbilitySystem/AOAbilitySystem.h"
#include "AegisOdyssey/AbilitySystem/Abilities/Attack/Combat/GA_HeavyAttack.h"
#include "AegisOdyssey/Character/AOCharacter.h"
#include "AegisOdyssey/Equipment/AOWeaponManagerComponent.h"
#include "AegisOdyssey/Equipment/Weapons/AOWeaponInstance.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(STT_PlayHeavyAttack)

EStateTreeRunStatus FSTT_PlayHeavyAttack::EnterState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	AAOCharacter* Character = Cast<AAOCharacter>(Context.GetOwner());
	if (!Character)
	{
		return EStateTreeRunStatus::Failed;
	}

	if (UAOWeaponManagerComponent* WeaponManagerComponent = Character->FindComponentByClass<UAOWeaponManagerComponent>())
	{
		InstanceData.OwningWeaponInstance = Cast<UAOWeaponInstance>(WeaponManagerComponent->GetCurrentWeaponInstance());
	}

	InstanceData.AbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Character);
	if (!InstanceData.AbilitySystemComponent || !InstanceData.InputTag.IsValid())
	{
		return EStateTreeRunStatus::Failed;
	}

	for (const FGameplayAbilitySpec& AbilitySpec : InstanceData.AbilitySystemComponent->GetActivatableAbilities())
	{
		if (AbilitySpec.Ability->GetAssetTags().HasTagExact(InstanceData.InputTag) && AbilitySpec.Ability && AbilitySpec.IsActive())
		{
			InstanceData.AbilitySystemComponent->CancelAbilityHandle(AbilitySpec.Handle);
		}
	}

	FHeavyAttackTargetData* TargetData = new FHeavyAttackTargetData();
	TargetData->InputTag = InstanceData.InputTag;
	TargetData->Montage = InstanceData.Montage;
	TargetData->PlayRate = InstanceData.PlayRate;
	TargetData->StartTime = InstanceData.StartTime;
	TargetData->StartSection = InstanceData.StartSection;
	TargetData->DataWeaponInstance = InstanceData.OwningWeaponInstance;

	FGameplayAbilityTargetDataHandle TargetDataHandle;
	TargetDataHandle.Data.Add(TSharedPtr<FGameplayAbilityTargetData>(TargetData));

	FGameplayEventData EventData;
	EventData.EventTag = InstanceData.InputTag;
	EventData.TargetData = TargetDataHandle;

	for (const FGameplayAbilitySpec& AbilitySpec : InstanceData.AbilitySystemComponent->GetActivatableAbilities())
	{
		if (AbilitySpec.Ability->GetAssetTags().HasTagExact(InstanceData.InputTag) && AbilitySpec.Ability && !AbilitySpec.IsActive())
		{
			InstanceData.AbilitySpecHandle = AbilitySpec.Handle;
		}
	}

	if (!InstanceData.AbilitySpecHandle.IsValid())
	{
		return EStateTreeRunStatus::Failed;
	}

	InstanceData.bActivated = InstanceData.AbilitySystemComponent->InternalTryActivateAbility(
		InstanceData.AbilitySpecHandle, FPredictionKey(), nullptr, nullptr, &EventData);

	return EStateTreeRunStatus::Running;
}

void FSTT_PlayHeavyAttack::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
}

void FSTT_PlayHeavyAttack::StateCompleted(FStateTreeExecutionContext& Context, const EStateTreeRunStatus CompletionStatus,
	const FStateTreeActiveStates& CompletedActiveStates) const
{
	FStateTreeTaskCommonBase::StateCompleted(Context, CompletionStatus, CompletedActiveStates);
}

EStateTreeRunStatus FSTT_PlayHeavyAttack::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	FGameplayAbilitySpec* AbilitySpec = InstanceData.AbilitySystemComponent->FindAbilitySpecFromHandle(InstanceData.AbilitySpecHandle);
	if (!AbilitySpec)
	{
		return EStateTreeRunStatus::Failed;
	}

	InstanceData.bActivated = AbilitySpec->IsActive();
	if (!InstanceData.bActivated)
	{
		return EStateTreeRunStatus::Succeeded;
	}

	return EStateTreeRunStatus::Running;
}
