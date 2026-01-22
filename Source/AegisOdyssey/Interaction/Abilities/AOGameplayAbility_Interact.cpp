// Fill out your copyright notice in the Description page of Project Settings.


#include "AOGameplayAbility_Interact.h"
#include "NativeGameplayTags.h"
#include "AbilitySystemComponent.h"
#include "AegisOdyssey/Interaction/InteractionStatics.h"
#include "AegisOdyssey/Interaction/Task/AT_WaitForInteractable_LineTrace.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOGameplayAbility_Interact)

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Ability_Interaction_Activate, "Ability.Interaction.Activate");
UAOGameplayAbility_Interact::UAOGameplayAbility_Interact(const FObjectInitializer& ObjectInitializer)
{
	ActivationPolicy = EAOAbilityActivationPolicy::OnSpawn;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

void UAOGameplayAbility_Interact::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	UAbilitySystemComponent* AbilitySystem = GetAbilitySystemComponentFromActorInfo();
	
	// 记录激活信息用于调试
	UE_LOG(LogTemp, Warning, TEXT("=== AOGameplayAbility_Interact::ActivateAbility 开始 ==="));
	UE_LOG(LogTemp, Warning, TEXT("网络角色: %s, 激活模式: %s"), 
		AbilitySystem ? *UEnum::GetValueAsString(AbilitySystem->GetOwnerRole()) : TEXT("NULL"),
		*UEnum::GetValueAsString(ActivationInfo.ActivationMode));
	
	// 支持本地预测：在客户端和服务器上都创建交互检测任务
	// 使用本地预测模式时，客户端需要本地执行以提供流畅的交互体验
	if (AbilitySystem && (AbilitySystem->GetOwnerRole() == ROLE_Authority))
	{
		UAT_WaitForInteractable_LineTrace* Task = UAT_WaitForInteractable_LineTrace::WaitForInteractableTarget_SingleLineTrace(
			this, 
			FCollisionProfileName(), 
			FGameplayAbilityTargetingLocationInfo(),
			InteractionScanRange,
			InteractionScanRate
		);
	}
}


//交互时，更新交互的对象Option
void UAOGameplayAbility_Interact::UpdateInteractOptions(const TArray<FInteractionOption>& InteractionOptions)
{
	CurrentOptions = InteractionOptions;
}

void UAOGameplayAbility_Interact::TriggerInteraction()
{
	if (CurrentOptions.Num() == 0) return;

	UAbilitySystemComponent* AbilitySystem = GetAbilitySystemComponentFromActorInfo();
	if (AbilitySystem)
	{
		const FInteractionOption& InteractionOption = CurrentOptions[0];

		AActor* Instigator = GetAvatarActorFromActorInfo();
		AActor* InteractableTargetActor = UInteractionStatics::GetActorFromInteractableTarget(InteractionOption.InteractableTarget);
		
		FGameplayEventData PayLoad;
		PayLoad.EventTag = TAG_Ability_Interaction_Activate;
		PayLoad.Instigator = Instigator;
		PayLoad.Target = InteractableTargetActor;

		InteractionOption.InteractableTarget->CustomizeInteractionEventData(TAG_Ability_Interaction_Activate, PayLoad);

		AActor* TargetActor = const_cast<AActor*>(ToRawPtr(PayLoad.Instigator));

		FGameplayAbilityActorInfo ActorInfo;
		ActorInfo.InitFromActor(InteractableTargetActor , TargetActor , InteractionOption.TargetAbilitySystem);
		
		// 使用EventTag的方式激活Ability。
		const bool bSuccess = InteractionOption.TargetAbilitySystem->TriggerAbilityFromGameplayEvent(
			InteractionOption.TargetInteractionAbilityHandle,
			&ActorInfo,
			TAG_Ability_Interaction_Activate,
			&PayLoad,
			*InteractionOption.TargetAbilitySystem
		);
	}
}


void UAOGameplayAbility_Interact::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
