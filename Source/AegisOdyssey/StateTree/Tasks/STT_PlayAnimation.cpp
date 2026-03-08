#include "STT_PlayAnimation.h"
#include "StateTreeExecutionContext.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AegisOdyssey/Character/AOCharacter.h"
#include "AegisOdyssey/AbilitySystem/AOAbilitySystem.h"
#include "AegisOdyssey/AbilitySystem/Abilities/Attack/Combat/GA_LightAttack.h"
#include "AbilitySystemGlobals.h"
#include "GameplayAbilitySpec.h"
#include "Abilities/GameplayAbility.h"
#include "GameplayPrediction.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(STT_PlayAnimation)

EStateTreeRunStatus FSTT_PlayAnimation::EnterState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	
	AAOCharacter* Character = Cast<AAOCharacter>(Context.GetOwner());
	if (!Character) 
	{
		return EStateTreeRunStatus::Failed;
	}
	

	InstanceData.AbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Character);
	if (!InstanceData.AbilitySystemComponent) 
	{
		return EStateTreeRunStatus::Failed;
	}

	if (!InstanceData.InputTag.IsValid()) 
	{
		return EStateTreeRunStatus::Failed;
	}


	UE_LOG(LogStateTree, Warning, TEXT("FSTT_PlayAnimation::EnterState: Canceling current abilities with tag: %s"), *InstanceData.InputTag.ToString());
	for (const FGameplayAbilitySpec& AbilitySpec : InstanceData.AbilitySystemComponent->GetActivatableAbilities())
	{
		if (AbilitySpec.Ability->GetAssetTags().HasTagExact(InstanceData.InputTag))
		{
			if (AbilitySpec.Ability && AbilitySpec.IsActive())
			{
				UE_LOG(LogStateTree, Warning, TEXT("FSTT_PlayAnimation::EnterState: Canceling ability: %s with AbilityTag: %s"), 
					*AbilitySpec.Ability->GetName(), *InstanceData.InputTag.ToString());
				InstanceData.AbilitySystemComponent->CancelAbilityHandle(AbilitySpec.Handle);
			}
		}
	}
	// 创建或重置Helper
	if (!InstanceData.AOAbilityTaskHelper)
	{
		InstanceData.AOAbilityTaskHelper = NewObject<UAOAbilityTaskHelper>(Context.GetOwner());
	}
	/**
	 * 创建轻攻击目标数据（支持网络自动复制到服务器）
	 * FLightAttackTargetData继承自FGameplayAbilityTargetData，会自动复制到服务器
	 */
	FLightAttackTargetData* TargetData = new FLightAttackTargetData();
	TargetData->InputTag = InstanceData.InputTag;
	TargetData->Montage = InstanceData.Montage;
	TargetData->PlayRate = InstanceData.PlayRate;
	TargetData->StartTime = InstanceData.StartTime;
	TargetData->StartSection = InstanceData.StartSection;

	/**
	 * 创建目标数据句柄并添加目标数据
	 */
	FGameplayAbilityTargetDataHandle TargetDataHandle;
	TargetDataHandle.Data.Add(TSharedPtr<FGameplayAbilityTargetData>(TargetData));
	
	/**
	 * 创建事件数据并传递目标数据句柄
	 * TargetData会自动复制到服务器，服务器也能获取到相同的参数
	 */
	FGameplayEventData EventData;
	EventData.EventTag = InstanceData.InputTag;
	EventData.TargetData = TargetDataHandle;

	UE_LOG(LogStateTree, Warning, TEXT("FSTT_PlayAnimation::EnterState: Activating GA_LightAttack with TargetData - InputTag: %s, Montage: %s, PlayRate: %.2f"), 
		*InstanceData.InputTag.ToString(), 
		*GetNameSafe(InstanceData.Montage), 
		InstanceData.PlayRate);

	FGameplayAbilitySpecHandle AbilitySpecHandle;
	UGameplayAbility* TargetAbility = nullptr;
	for (const FGameplayAbilitySpec& AbilitySpec : InstanceData.AbilitySystemComponent->GetActivatableAbilities())
	{
		if (AbilitySpec.Ability->GetAssetTags().HasTagExact(InstanceData.InputTag))
		{
			if (AbilitySpec.Ability && !AbilitySpec.IsActive())
			{
				AbilitySpecHandle = AbilitySpec.Handle;
				TargetAbility = AbilitySpec.Ability;
			}
		}
	}


	/**
	 * 调用HandleGameplayEvent激活能力并传递参数
	 * 参数会通过TargetData自动复制到服务器
	 */

	FOnGameplayAbilityEnded::FDelegate AbilityEndedDelegate;
	AbilityEndedDelegate.BindUObject(InstanceData.AOAbilityTaskHelper, &UAOAbilityTaskHelper::OnAbilityEnded);

	InstanceData.AOAbilityTaskHelper->bAbilityIsActivate = InstanceData.AbilitySystemComponent->InternalTryActivateAbility(AbilitySpecHandle,FPredictionKey(),&TargetAbility,
		&AbilityEndedDelegate, &EventData);

	UE_LOG(LogStateTree, Warning, TEXT("FSTT_PlayAnimation::EnterState: Activation result: %d"), InstanceData.bActivated);

	return EStateTreeRunStatus::Running;

}

void FSTT_PlayAnimation::ExitState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
}


void FSTT_PlayAnimation::StateCompleted(FStateTreeExecutionContext& Context, const EStateTreeRunStatus CompletionStatus,
	const FStateTreeActiveStates& CompletedActiveStates) const
{
	FStateTreeTaskCommonBase::StateCompleted(Context, CompletionStatus, CompletedActiveStates);
}

EStateTreeRunStatus FSTT_PlayAnimation::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	if (!InstanceData.AOAbilityTaskHelper) return EStateTreeRunStatus::Failed;
	if (!InstanceData.AOAbilityTaskHelper->bAbilityIsActivate)
	{
		UE_LOG(LogStateTree, Warning, TEXT("FSTT_PlayAnimation::Finish Task From Current Ability End "));

		return EStateTreeRunStatus::Succeeded;
	}
	return EStateTreeRunStatus::Running;
}

void UAOAbilityTaskHelper::OnAbilityEnded(UGameplayAbility* TargetAbility)
{
	bAbilityIsActivate = false;
}
