// Fill out your copyright notice in the Description page of Project Settings.

#include "AOSkillAbility_VolcanoBurst.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AegisOdyssey/AbilitySystem/Tasks/AT_WaitMovementInput.h"
#include "AegisOdyssey/AOSkillEventTags.h"
#include "AegisOdyssey/Equipment/AOEquipmentInstance.h"
#include "AegisOdyssey/Equipment/AOWeaponManagerComponent.h"
#include "AegisOdyssey/SkillSystem/Execution/Definitions/AOSkillExecutionDefinition_AreaSequence.h"
#include "AegisOdyssey/SkillSystem/Execution/Runtime/AOSkillAreaSequenceRuntime_VolcanoBurst.h"
#include "Animation/AnimMontage.h"
#include "AbilitySystemComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOSkillAbility_VolcanoBurst)

UAOSkillAbility_VolcanoBurst::UAOSkillAbility_VolcanoBurst(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	StartSequenceEventTag = AOSkillEventTags::GameplayEvent_Skill_VolcanoBurst_StartSequence;
	RuntimeActorClass = AAOSkillAreaSequenceRuntime_VolcanoBurst::StaticClass();
}

void UAOSkillAbility_VolcanoBurst::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	bWaveSequenceStarted = false;
	HiddenWeaponInstance.Reset();
	ActiveRuntimeSequence.Reset();
	ClearMontageTask();
	ClearSequenceEventTask();
	ClearMovementInputTask();
	ClearWaveTimers();

	UAOSkillAreaSequenceExecutionDefinition* ExecutionDefinition = GetAreaSequenceExecutionDefinition();
	if (ExecutionDefinition == nullptr)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 这里只在施法起手阶段临时隐藏当前武器，避免施法动作穿帮。
	HideCurrentWeaponIfNeeded();

	// 对齐普攻语义：起手阶段也监听移动输入，但只有 runtime 接手后才允许脱手。
	StartMovementInputTaskIfNeeded();

	// Volcano Burst 的喷发启动时机完全交给动画事件。
	const bool bWaitingStartEvent = StartSequenceEventWaitIfNeeded();
	StartMontageTaskIfNeeded();

	if (bWaitingStartEvent)
	{
		return;
	}

	// 没配起手事件时，退化成激活后立刻进入喷发阶段。
	StartWaveSequence();
}

void UAOSkillAbility_VolcanoBurst::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	ClearSequenceEventTask();
	ClearMovementInputTask();
	ClearWaveTimers();
	bWaveSequenceStarted = false;

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	// Montage 任务要跟着 Ability 一起结束，才能复用 PlayMontageAndWait 自带的收口逻辑。
	StartMontageTask = nullptr;

	// 先让 GAS 停掉起手动画，再恢复武器显示，避免先露武器后停动画。
	RestoreHiddenWeaponIfNeeded();
}

void UAOSkillAbility_VolcanoBurst::StartWaveSequence()
{
	if (bWaveSequenceStarted)
	{
		return;
	}

	bWaveSequenceStarted = true;
	ClearSequenceEventTask();
	ClearWaveTimers();

	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	const bool bIsAuthority = AvatarActor != nullptr && AvatarActor->HasAuthority();

	// 长生命周期 runtime 只允许服务端生成，避免客户端本地再结算一套。
	if (!bIsAuthority)
	{
		if (StartMontage == nullptr)
		{
			FinishAbilityAfterRuntimeHandoff();
		}
		return;
	}

	// 从这里开始，长生命周期从 Ability 转移给独立 runtime actor。
	const bool bSpawnedRuntime = SpawnAndStartRuntimeSequence();
	if (!bSpawnedRuntime)
	{
		ClearMovementInputTask();
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	// runtime 接手后，如果没有起手蒙太奇，就不再继续占住 Ability 主链。
	if (StartMontage == nullptr)
	{
		FinishAbilityAfterRuntimeHandoff();
	}
}

bool UAOSkillAbility_VolcanoBurst::SpawnAndStartRuntimeSequence()
{
	UAOSkillAreaSequenceExecutionDefinition* ExecutionDefinition = GetAreaSequenceExecutionDefinition();
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (ExecutionDefinition == nullptr || AvatarActor == nullptr || GetAbilitySystemComponentFromActorInfo() == nullptr)
	{
		return false;
	}

	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return false;
	}

	UClass* RuntimeClassToSpawn = RuntimeActorClass != nullptr
		? RuntimeActorClass.Get()
		: AAOSkillAreaSequenceRuntime_VolcanoBurst::StaticClass();

	AAOSkillAreaSequenceRuntime_VolcanoBurst* RuntimeActor = World->SpawnActorDeferred<AAOSkillAreaSequenceRuntime_VolcanoBurst>(
		RuntimeClassToSpawn,
		FTransform(AvatarActor->GetActorRotation(), AvatarActor->GetActorLocation()),
		AvatarActor,
		Cast<APawn>(AvatarActor),
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	if (RuntimeActor == nullptr)
	{
		return false;
	}

	// runtime 只保留推进波次和采集命中所需的最小上下文。
	// 真实命中结算继续回到 SkillAbility 的统一收口链。
	RuntimeActor->InitializeRuntime(
		this,
		AvatarActor,
		ExecutionDefinition,
		GetAbilityLevel());

	RuntimeActor->FinishSpawning(FTransform(AvatarActor->GetActorRotation(), AvatarActor->GetActorLocation()));
	RuntimeActor->StartRuntimeSequence();
	ActiveRuntimeSequence = RuntimeActor;
	return true;
}

void UAOSkillAbility_VolcanoBurst::StartMontageTaskIfNeeded()
{
	if (!StartMontage)
	{
		return;
	}

	ClearMontageTask();

	StartMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		FName(TEXT("VolcanoBurst_StartMontage")),
		StartMontage,
		1.0f,
		NAME_None,
		true,
		1.0f,
		0.0f,
		false);

	if (StartMontageTask == nullptr)
	{
		return;
	}

	StartMontageTask->OnBlendOut.AddDynamic(this, &ThisClass::HandleStartMontageBlendedOut);
	StartMontageTask->OnCompleted.AddDynamic(this, &ThisClass::HandleStartMontageCompleted);
	StartMontageTask->OnInterrupted.AddDynamic(this, &ThisClass::HandleStartMontageInterrupted);
	StartMontageTask->OnCancelled.AddDynamic(this, &ThisClass::HandleStartMontageCancelled);
	StartMontageTask->ReadyForActivation();
}

bool UAOSkillAbility_VolcanoBurst::StartSequenceEventWaitIfNeeded()
{
	if (!StartSequenceEventTag.IsValid())
	{
		return false;
	}

	ClearSequenceEventTask();

	StartSequenceEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		StartSequenceEventTag,
		nullptr,
		false,
		false);

	if (StartSequenceEventTask == nullptr)
	{
		return false;
	}

	StartSequenceEventTask->EventReceived.AddDynamic(this, &ThisClass::HandleStartSequenceEventReceived);
	StartSequenceEventTask->ReadyForActivation();
	return true;
}

void UAOSkillAbility_VolcanoBurst::ClearMontageTask()
{
	if (StartMontageTask)
	{
		StartMontageTask->EndTask();
		StartMontageTask = nullptr;
	}
}

void UAOSkillAbility_VolcanoBurst::ClearSequenceEventTask()
{
	if (StartSequenceEventTask)
	{
		StartSequenceEventTask->EndTask();
		StartSequenceEventTask = nullptr;
	}
}

void UAOSkillAbility_VolcanoBurst::StartMovementInputTaskIfNeeded()
{
	if (MovementInputTask != nullptr)
	{
		return;
	}

	MovementInputTask = UAT_WaitMovementInput::WaitMovementInput(this);
	if (MovementInputTask == nullptr)
	{
		return;
	}

	MovementInputTask->OnMovementInputDetected.AddDynamic(this, &ThisClass::HandleMovementInputDetected);
	MovementInputTask->ReadyForActivation();
}

void UAOSkillAbility_VolcanoBurst::ClearMovementInputTask()
{
	if (MovementInputTask)
	{
		MovementInputTask->EndTask();
		MovementInputTask = nullptr;
	}
}

void UAOSkillAbility_VolcanoBurst::ClearWaveTimers()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(SequenceStartTimerHandle);
	}
}

void UAOSkillAbility_VolcanoBurst::HideCurrentWeaponIfNeeded()
{
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (AvatarActor == nullptr)
	{
		return;
	}

	UAOWeaponManagerComponent* WeaponManagerComponent = AvatarActor->FindComponentByClass<UAOWeaponManagerComponent>();
	if (WeaponManagerComponent == nullptr)
	{
		return;
	}

	UAOEquipmentInstance* CurrentWeaponInstance = WeaponManagerComponent->GetCurrentWeaponInstance();
	if (CurrentWeaponInstance == nullptr)
	{
		return;
	}

	// 只记录这次 Ability 自己隐藏过的武器。
	HiddenWeaponInstance = CurrentWeaponInstance;

	for (AActor* SpawnedActor : CurrentWeaponInstance->GetSpawnedActors())
	{
		if (SpawnedActor != nullptr)
		{
			SpawnedActor->SetActorHiddenInGame(true);
			SpawnedActor->SetActorEnableCollision(false);
		}
	}
}

void UAOSkillAbility_VolcanoBurst::RestoreHiddenWeaponIfNeeded()
{
	if (!HiddenWeaponInstance.IsValid())
	{
		return;
	}

	// Ability 结束时只恢复它自己在起手阶段隐藏过的武器显示。
	for (AActor* SpawnedActor : HiddenWeaponInstance->GetSpawnedActors())
	{
		if (SpawnedActor != nullptr)
		{
			SpawnedActor->SetActorHiddenInGame(false);
			SpawnedActor->SetActorEnableCollision(true);
		}
	}

	HiddenWeaponInstance.Reset();
}

void UAOSkillAbility_VolcanoBurst::HandleStartSequenceEventReceived(FGameplayEventData Payload)
{
	StartWaveSequence();
}

void UAOSkillAbility_VolcanoBurst::HandleStartMontageCompleted()
{
	FinishAbilityAfterMontage(false);
}

void UAOSkillAbility_VolcanoBurst::HandleStartMontageBlendedOut()
{
	FinishAbilityAfterMontage(false);
}

void UAOSkillAbility_VolcanoBurst::HandleStartMontageInterrupted()
{
	FinishAbilityAfterMontage(true);
}

void UAOSkillAbility_VolcanoBurst::HandleStartMontageCancelled()
{
	FinishAbilityAfterMontage(true);
}

void UAOSkillAbility_VolcanoBurst::HandleMovementInputDetected()
{
	if (!bWaveSequenceStarted || !CancelAbilityTag.IsValid())
	{
		return;
	}

	UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo();
	if (AbilitySystemComponent == nullptr)
	{
		return;
	}

	// 只有 runtime 已经接手，且角色进入可取消窗口后，才允许提前脱手。
	if (AbilitySystemComponent->HasMatchingGameplayTag(CancelAbilityTag))
	{
		FinishAbilityAfterRuntimeHandoff();
	}
}

void UAOSkillAbility_VolcanoBurst::FinishAbilityAfterMontage(bool bWasCancelled)
{
	// 不提前 EndTask，让 PlayMontageAndWait 通过 Ability 结束走自己的收口逻辑。
	ClearSequenceEventTask();
	ClearMovementInputTask();
	ClearWaveTimers();
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, bWasCancelled);
}

void UAOSkillAbility_VolcanoBurst::FinishAbilityAfterRuntimeHandoff()
{
	// 这里收的是“角色脱手”，不是“喷发被取消”。
	FinishAbilityAfterMontage(false);
}
