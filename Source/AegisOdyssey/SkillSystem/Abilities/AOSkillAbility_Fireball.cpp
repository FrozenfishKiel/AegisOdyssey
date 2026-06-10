// Fill out your copyright notice in the Description page of Project Settings.

#include "AOSkillAbility_Fireball.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AegisOdyssey/AOLogChannels.h"
#include "AegisOdyssey/AOSkillEventTags.h"
#include "AegisOdyssey/Equipment/AOEquipmentInstance.h"
#include "AegisOdyssey/Equipment/AOWeaponManagerComponent.h"
#include "AegisOdyssey/SkillSystem/Execution/Runtime/AOSkillProjectile_Fireball.h"
#include "Animation/AnimMontage.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOSkillAbility_Fireball)

UAOSkillAbility_Fireball::UAOSkillAbility_Fireball(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SpawnProjectileEventTag = AOSkillEventTags::GameplayEvent_Skill_Fireball_SpawnProjectile;
	RuntimeProjectileClass = AAOSkillProjectile_Fireball::StaticClass();
}

void UAOSkillAbility_Fireball::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	bProjectileSpawned = false;
	HiddenWeaponInstance.Reset();
	ClearMontageTask();
	ClearSpawnProjectileEventTask();

	// 火球术升级后的主链：
	// Ability 先负责施法动画与事件时序，真正发射之后再把飞行/命中交给投射体自己。
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	HideCurrentWeaponIfNeeded();

	const bool bWaitingSpawnEvent = StartSpawnProjectileEventWaitIfNeeded();
	StartMontageTaskIfNeeded();

	UE_LOG(
		LogAegisOdysseyAbilitySystem,
		Log,
		TEXT("UAOSkillAbility_Fireball::ActivateAbility: WaitingSpawnEvent=%s, StartMontage=%s, EventTag=%s, Avatar=%s"),
		bWaitingSpawnEvent ? TEXT("true") : TEXT("false"),
		*GetNameSafe(StartMontage),
		*SpawnProjectileEventTag.ToString(),
		*GetNameSafe(GetAvatarActorFromActorInfo()));

	// 如果没配置事件标签，就退化成立即发射。
	if (!bWaitingSpawnEvent)
	{
		SpawnProjectileFromAnimationEvent();
	}

	// 如果连起手蒙太奇都没配置，火球脱手后就不应该再继续占着 Ability。
	if (StartMontage == nullptr && bProjectileSpawned)
	{
		FinishAbilityAfterProjectileHandoff();
	}
}

void UAOSkillAbility_Fireball::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	ClearSpawnProjectileEventTask();
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
	StartMontageTask = nullptr;
	RestoreHiddenWeaponIfNeeded();
}

TSubclassOf<AActor> UAOSkillAbility_Fireball::GetProjectileActorClassToSpawn() const
{
	return RuntimeProjectileClass;
}

void UAOSkillAbility_Fireball::OnConfiguredProjectileSpawned(AActor* SpawnedProjectile)
{
	if (AAOSkillProjectile_Fireball* FireballProjectile = Cast<AAOSkillProjectile_Fireball>(SpawnedProjectile))
	{
		// 火球术自己的投射体运行时上下文在这里补齐。
		FireballProjectile->InitializeFromSkillAbility(this);
	}
}

void UAOSkillAbility_Fireball::StartMontageTaskIfNeeded()
{
	if (!StartMontage)
	{
		return;
	}

	ClearMontageTask();

	StartMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		FName(TEXT("Fireball_StartMontage")),
		StartMontage,
		1.0f,
		NAME_None,
		true,
		1.0f,
		0.0f,
		false);

	if (StartMontageTask == nullptr)
	{
		UE_LOG(
			LogAegisOdysseyAbilitySystem,
			Warning,
			TEXT("UAOSkillAbility_Fireball::StartMontageTaskIfNeeded: Failed to create montage task. Montage=%s"),
			*GetNameSafe(StartMontage));
		return;
	}

	StartMontageTask->OnBlendOut.AddDynamic(this, &ThisClass::HandleStartMontageBlendedOut);
	StartMontageTask->OnCompleted.AddDynamic(this, &ThisClass::HandleStartMontageCompleted);
	StartMontageTask->OnInterrupted.AddDynamic(this, &ThisClass::HandleStartMontageInterrupted);
	StartMontageTask->OnCancelled.AddDynamic(this, &ThisClass::HandleStartMontageCancelled);
	StartMontageTask->ReadyForActivation();
}

bool UAOSkillAbility_Fireball::StartSpawnProjectileEventWaitIfNeeded()
{
	if (!SpawnProjectileEventTag.IsValid())
	{
		UE_LOG(LogAegisOdysseyAbilitySystem, Warning, TEXT("UAOSkillAbility_Fireball::StartSpawnProjectileEventWaitIfNeeded: SpawnProjectileEventTag is invalid."));
		return false;
	}

	ClearSpawnProjectileEventTask();

	SpawnProjectileEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		SpawnProjectileEventTag,
		nullptr,
		false,
		false);

	if (SpawnProjectileEventTask == nullptr)
	{
		UE_LOG(
			LogAegisOdysseyAbilitySystem,
			Warning,
			TEXT("UAOSkillAbility_Fireball::StartSpawnProjectileEventWaitIfNeeded: Failed to create WaitGameplayEvent task for tag [%s]."),
			*SpawnProjectileEventTag.ToString());
		return false;
	}

	UE_LOG(
		LogAegisOdysseyAbilitySystem,
		Log,
		TEXT("UAOSkillAbility_Fireball::StartSpawnProjectileEventWaitIfNeeded: Listening for tag [%s]."),
		*SpawnProjectileEventTag.ToString());

	SpawnProjectileEventTask->EventReceived.AddDynamic(this, &ThisClass::HandleSpawnProjectileEventReceived);
	SpawnProjectileEventTask->ReadyForActivation();
	return true;
}

void UAOSkillAbility_Fireball::ClearMontageTask()
{
	if (StartMontageTask)
	{
		StartMontageTask->EndTask();
		StartMontageTask = nullptr;
	}
}

void UAOSkillAbility_Fireball::ClearSpawnProjectileEventTask()
{
	if (SpawnProjectileEventTask)
	{
		SpawnProjectileEventTask->EndTask();
		SpawnProjectileEventTask = nullptr;
	}
}

void UAOSkillAbility_Fireball::HideCurrentWeaponIfNeeded()
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

void UAOSkillAbility_Fireball::RestoreHiddenWeaponIfNeeded()
{
	if (!HiddenWeaponInstance.IsValid())
	{
		return;
	}

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

void UAOSkillAbility_Fireball::SpawnProjectileFromAnimationEvent()
{
	if (bProjectileSpawned)
	{
		return;
	}

	bProjectileSpawned = true;
	ClearSpawnProjectileEventTask();

	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	const bool bIsAuthority = AvatarActor != nullptr && AvatarActor->HasAuthority();

	// 火球运行时 Actor 现在只允许服务端生成。
	// 这样可以避免 LocalPredicted 下客户端先本地生成一颗、随后服务端再复制一颗，
	// 进而出现“客户端刚出手就爆炸，但另一颗又正常飞出去”的双实例问题。
	if (bIsAuthority && !SpawnConfiguredProjectile())
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	// 火球一旦脱手，后续生命周期就属于投射体自己了。
	// 如果此时起手蒙太奇已经不存在，Ability 也应该立刻让出主链。
	if (StartMontage == nullptr)
	{
		FinishAbilityAfterProjectileHandoff();
	}
}

void UAOSkillAbility_Fireball::FinishAbilityAfterMontage(bool bWasCancelled)
{
	// 火球发射成功后，Ability 的职责只剩起手动画本身。
	// 这里不提前手动 EndTask 蒙太奇任务，让 GAS 在 Ability 正式结束时统一收口。
	ClearSpawnProjectileEventTask();
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, bWasCancelled);
}

void UAOSkillAbility_Fireball::FinishAbilityAfterProjectileHandoff()
{
	// 这里收的是“角色施法动作占用”而不是“火球被取消”。
	// 投射体已经独立存在时，提前脱手不应把这次释放判成取消。
	FinishAbilityAfterMontage(false);
}

void UAOSkillAbility_Fireball::HandleSpawnProjectileEventReceived(FGameplayEventData Payload)
{
	UE_LOG(
		LogAegisOdysseyAbilitySystem,
		Log,
		TEXT("UAOSkillAbility_Fireball::HandleSpawnProjectileEventReceived: Received tag [%s]."),
		*Payload.EventTag.ToString());
	SpawnProjectileFromAnimationEvent();
}

void UAOSkillAbility_Fireball::HandleStartMontageCompleted()
{
	if (!bProjectileSpawned)
	{
		UE_LOG(
			LogAegisOdysseyAbilitySystem,
			Warning,
			TEXT("UAOSkillAbility_Fireball::HandleStartMontageCompleted: Montage completed before fireball spawn event was received."));
	}
	FinishAbilityAfterMontage(false);
}

void UAOSkillAbility_Fireball::HandleStartMontageBlendedOut()
{
	if (!bProjectileSpawned)
	{
		UE_LOG(
			LogAegisOdysseyAbilitySystem,
			Warning,
			TEXT("UAOSkillAbility_Fireball::HandleStartMontageBlendedOut: Montage blended out before fireball spawn event was received."));
	}
	FinishAbilityAfterMontage(false);
}

void UAOSkillAbility_Fireball::HandleStartMontageInterrupted()
{
	if (!bProjectileSpawned)
	{
		UE_LOG(
			LogAegisOdysseyAbilitySystem,
			Warning,
			TEXT("UAOSkillAbility_Fireball::HandleStartMontageInterrupted: Montage interrupted before fireball spawn event was received."));
	}
	FinishAbilityAfterMontage(true);
}

void UAOSkillAbility_Fireball::HandleStartMontageCancelled()
{
	if (!bProjectileSpawned)
	{
		UE_LOG(
			LogAegisOdysseyAbilitySystem,
			Warning,
			TEXT("UAOSkillAbility_Fireball::HandleStartMontageCancelled: Montage cancelled before fireball spawn event was received."));
	}
	FinishAbilityAfterMontage(true);
}
