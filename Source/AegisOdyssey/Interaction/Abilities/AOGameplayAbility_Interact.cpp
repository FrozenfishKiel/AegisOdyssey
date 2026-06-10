// Fill out your copyright notice in the Description page of Project Settings.

#include "AOGameplayAbility_Interact.h"

#include "Abilities/Tasks/AbilityTask_WaitInputPress.h"
#include "AbilitySystemComponent.h"
#include "AegisOdyssey/Interaction/InteractionStatics.h"
#include "AegisOdyssey/Interaction/Task/AT_WaitForInteractable_LineTrace.h"
#include "AegisOdyssey/UI/AOHUD.h"
#include "AegisOdyssey/UI/AOHUDViewModelComponent.h"
#include "AegisOdyssey/UI/ViewModel/MVVM_HUD.h"
#include "NativeGameplayTags.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOGameplayAbility_Interact)

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Ability_Interaction_Activate, "Ability.Interaction.Activate");

UAOGameplayAbility_Interact::UAOGameplayAbility_Interact(const FObjectInitializer& ObjectInitializer)
{
	ActivationPolicy = EAOAbilityActivationPolicy::OnSpawn;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

void UAOGameplayAbility_Interact::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// 当前项目中的 GA_BP_Interact 蓝图子类已经实现了“扫描 + 等待输入 + 触发交互”的完整流程。
	// 这里不再在 C++ 里重复挂一套相同逻辑，避免一次按键重复执行。
}

void UAOGameplayAbility_Interact::UpdateInteractOptions(const TArray<FInteractionOption>& InteractionOptions)
{
	CurrentOptions = InteractionOptions;
	PushInteractionOptionsToHUDViewModel();
}

void UAOGameplayAbility_Interact::TriggerInteraction()
{
	TriggerInteractionByIndex(0);
}

void UAOGameplayAbility_Interact::TriggerInteractionByIndex(int32 OptionIndex)
{
	if (!CurrentOptions.IsValidIndex(OptionIndex))
	{
		return;
	}

	UAbilitySystemComponent* AbilitySystem = GetAbilitySystemComponentFromActorInfo();
	if (!AbilitySystem)
	{
		return;
	}

	const FInteractionOption& InteractionOption = CurrentOptions[OptionIndex];
	AActor* Instigator = GetAvatarActorFromActorInfo();
	AActor* InteractableTargetActor = UInteractionStatics::GetActorFromInteractableTarget(InteractionOption.InteractableTarget);
	if (!Instigator || !InteractableTargetActor)
	{
		return;
	}

	// 统一交互能力只负责组装事件数据并分发给对象侧，
	// 对象到底是“拾取”、“打开 UI”还是别的行为，由对象自己决定。
	FGameplayEventData Payload;
	Payload.EventTag = TAG_Ability_Interaction_Activate;
	Payload.Instigator = Instigator;
	Payload.Target = InteractableTargetActor;
	Payload.EventMagnitude = static_cast<float>(OptionIndex);

	// 优先走对象侧统一交互执行。
	if (UInteractionStatics::TryExecuteInteraction(InteractionOption.InteractableTarget, TAG_Ability_Interaction_Activate, Payload))
	{
		return;
	}

	// 如果对象侧没有直接接住，则回退到旧的能力事件链。
	if (!InteractionOption.TargetAbilitySystem || !InteractionOption.TargetInteractionAbilityHandle.IsValid())
	{
		return;
	}

	FGameplayAbilityActorInfo TargetActorInfo;
	TargetActorInfo.InitFromActor(InteractableTargetActor, Instigator, InteractionOption.TargetAbilitySystem);

	InteractionOption.TargetAbilitySystem->TriggerAbilityFromGameplayEvent(
		InteractionOption.TargetInteractionAbilityHandle,
		&TargetActorInfo,
		TAG_Ability_Interaction_Activate,
		&Payload,
		*InteractionOption.TargetAbilitySystem);
}

void UAOGameplayAbility_Interact::BeginWaitForInteractionInput()
{
	if (WaitInputPressTask)
	{
		WaitInputPressTask->EndTask();
		WaitInputPressTask = nullptr;
	}

	WaitInputPressTask = UAbilityTask_WaitInputPress::WaitInputPress(this, false);
	if (!WaitInputPressTask)
	{
		return;
	}

	WaitInputPressTask->OnPress.AddDynamic(this, &ThisClass::HandleInteractionInputPressed);
	WaitInputPressTask->ReadyForActivation();
}

void UAOGameplayAbility_Interact::HandleInteractionInputPressed(float TimeWaited)
{
	TriggerInteraction();

	// WaitInputPress 只消费一次按下事件，因此每次触发后立刻重新挂起。
	BeginWaitForInteractionInput();
}

void UAOGameplayAbility_Interact::PushInteractionOptionsToHUDViewModel() const
{
	const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo();
	if (!ActorInfo || !ActorInfo->PlayerController.IsValid())
	{
		return;
	}

	APlayerController* PlayerController = ActorInfo->PlayerController.Get();
	if (!PlayerController || !PlayerController->IsLocalController())
	{
		return;
	}

	if (UAOHUDViewModelComponent* HUDViewModelComponent = AAOHUD::FindHUDOwnedComponent<UAOHUDViewModelComponent>(PlayerController))
	{
		if (UMVVM_HUD* HUDViewModel = HUDViewModelComponent->GetHUDMVVM())
		{
			HUDViewModel->SetInteractionOptions(CurrentOptions);
		}
	}
}

void UAOGameplayAbility_Interact::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	if (ActiveTraceTask)
	{
		ActiveTraceTask->InteractableObjectsChanged.RemoveDynamic(this, &ThisClass::UpdateInteractOptions);
		ActiveTraceTask = nullptr;
	}

	if (WaitInputPressTask)
	{
		WaitInputPressTask->EndTask();
		WaitInputPressTask = nullptr;
	}

	CurrentOptions.Reset();
	PushInteractionOptionsToHUDViewModel();

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
