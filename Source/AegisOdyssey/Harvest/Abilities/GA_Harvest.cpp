#include "AegisOdyssey/Harvest/Abilities/GA_Harvest.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
#include "AbilitySystemComponent.h"
#include "GameplayCueManager.h"
#include "AegisOdyssey/AOLogChannels.h"
#include "AegisOdyssey/AOStateTags.h"
#include "AegisOdyssey/AOHarvestCueTags.h"
#include "AegisOdyssey/Harvest/Core/AOHarvestableComponent.h"
#include "AegisOdyssey/Harvest/Core/AOHarvestableActor.h"
#include "AegisOdyssey/Harvest/Cue/AOHarvestGameplayCueNotify_Burst.h"
#include "AegisOdyssey/Harvest/Definition/AOHarvestToolDefinition.h"
#include "AegisOdyssey/Harvest/Definition/AOHarvestableDefinition.h"
#include "AegisOdyssey/Harvest/Fragments/AOHarvestToolFragment.h"
#include "AegisOdyssey/Harvest/Core/AOHarvestableTarget.h"
#include "AegisOdyssey/Harvest/Items/AOHarvestToolInstance.h"
#include "AegisOdyssey/Harvest/System/AOHarvestResolver.h"
#include "AegisOdyssey/Inventory/AOInventoryItemDefinition.h"
#include "AegisOdyssey/Inventory/AOInventoryIteminstance.h"
#include "AegisOdyssey/Inventory/AOInventoryStatics.h"
#include "AegisOdyssey/Items/AOItemCatalogTypes.h"
#include "AegisOdyssey/System/AOGameData.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Engine/DataTable.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Components/MeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GA_Harvest)

namespace
{
	const FAOItemCatalogRow* FindItemCatalogRowById(const UDataTable& ItemCatalogDataTable, int32 ItemId)
	{
		if (ItemCatalogDataTable.GetRowStruct() != FAOItemCatalogRow::StaticStruct())
		{
			return nullptr;
		}

		for (const TPair<FName, uint8*>& Pair : ItemCatalogDataTable.GetRowMap())
		{
			const FAOItemCatalogRow* ItemRow = reinterpret_cast<const FAOItemCatalogRow*>(Pair.Value);
			if (ItemRow != nullptr && ItemRow->ItemId == ItemId)
			{
				return ItemRow;
			}
		}

		return nullptr;
	}
}

UGA_Harvest::UGA_Harvest(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ActivationPolicy = EAOAbilityActivationPolicy::WhileInputActive;
	HarvestHitWindowTag = AOStateTags::State_Harvest_HitWindow;
	Montage = nullptr;
	PlayRate = 1.0f;
	StartSection = NAME_None;
	StartTime = 0.0f;
	bEnableHarvestDebugDraw = false;
	HarvestDebugDrawDuration = 2.0f;
}

bool UGA_Harvest::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags,
	FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	return GetAvatarActorFromActorInfo() != nullptr;
}

void UGA_Harvest::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	bHasValidHarvestContext = ExtractHarvestTargetData(TriggerEventData, CurrentHarvestTargetData);
	if (!bHasValidHarvestContext)
	{
		UE_LOG(LogAegisOdysseyAbilitySystem, Warning, TEXT("UGA_Harvest::ActivateAbility: Missing valid harvest target data."));
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	Montage = CurrentHarvestTargetData.Montage.Get();
	PlayRate = CurrentHarvestTargetData.PlayRate;
	StartSection = CurrentHarvestTargetData.StartSection;
	StartTime = CurrentHarvestTargetData.StartTime;

	if (Montage == nullptr)
	{
		UE_LOG(LogAegisOdysseyAbilitySystem, Warning, TEXT("UGA_Harvest::ActivateAbility: Harvest montage is missing from target data."));
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	if (WaitInputReleaseTask)
	{
		WaitInputReleaseTask->EndTask();
		WaitInputReleaseTask = nullptr;
	}

	WaitInputReleaseTask = UAbilityTask_WaitInputRelease::WaitInputRelease(this);
	if (WaitInputReleaseTask)
	{
		WaitInputReleaseTask->OnRelease.AddDynamic(this, &UGA_Harvest::OnInputReleased);
		WaitInputReleaseTask->ReadyForActivation();
	}

	if (!WaitHarvestHitTask)
	{
		WaitHarvestHitTask = UAT_WaitHarvestHit::WaitHarvestHit(this);
		if (WaitHarvestHitTask)
		{
			WaitHarvestHitTask->ReadyForActivation();
		}
	}

	PlayHarvestMontage();
}

void UGA_Harvest::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (WaitInputReleaseTask)
	{
		WaitInputReleaseTask->EndTask();
		WaitInputReleaseTask = nullptr;
	}

	if (HarvestMontageTask)
	{
		HarvestMontageTask->EndTask();
		HarvestMontageTask = nullptr;
	}

	if (WaitHarvestHitTask)
	{
		WaitHarvestHitTask->EndTask();
		WaitHarvestHitTask = nullptr;
	}

	ClearHarvestWindowTag();

	bHasValidHarvestContext = false;
	CurrentHarvestTargetData = FAOHarvestTargetData();
	Montage = nullptr;
	PlayRate = 1.0f;
	StartSection = NAME_None;
	StartTime = 0.0f;

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_Harvest::OnInputReleased(float TimeHeld)
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_Harvest::OnMontageCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_Harvest::OnMontageBlendedOut()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_Harvest::OnMontageInterrupted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UGA_Harvest::OnMontageCancelled()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

bool UGA_Harvest::ExtractHarvestTargetData(const FGameplayEventData* TriggerEventData, FAOHarvestTargetData& OutTargetData) const
{
	if (TriggerEventData == nullptr)
	{
		return false;
	}

	for (const TSharedPtr<FGameplayAbilityTargetData>& Data : TriggerEventData->TargetData.Data)
	{
		if (!Data.IsValid() || Data->GetScriptStruct() != FAOHarvestTargetData::StaticStruct())
		{
			continue;
		}

		OutTargetData = *static_cast<FAOHarvestTargetData*>(Data.Get());
		return true;
	}

	return false;
}

bool UGA_Harvest::RebuildToolRuntimeContext(FAOHarvestRuntimeContext& InOutRuntimeContext) const
{
	const UAOHarvestToolInstance* ToolInstance = InOutRuntimeContext.ToolInstance.Get();
	const UAOHarvestToolDefinition* ToolDefinition = ToolInstance != nullptr
		? ToolInstance->GetHarvestToolDefinition()
		: InOutRuntimeContext.ToolDefinition.Get();
	const UAOHarvestToolFragment* ToolFragment = ToolInstance != nullptr
		? ToolInstance->GetHarvestToolFragment()
		: (ToolDefinition != nullptr ? ToolDefinition->FindHarvestToolFragment() : nullptr);
	if (ToolDefinition == nullptr || ToolFragment == nullptr)
	{
		return false;
	}

	InOutRuntimeContext.ToolDefinition = ToolDefinition;
	InOutRuntimeContext.ToolFragment = ToolFragment;
	return true;
}

void UGA_Harvest::PlayHarvestMontage()
{
	if (Montage == nullptr)
	{
		UE_LOG(LogAegisOdysseyAbilitySystem, Warning,
			TEXT("UGA_Harvest::PlayHarvestMontage: Montage is not configured in the current harvest request."));
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo();
	UAnimInstance* AnimInstance = ActorInfo ? ActorInfo->GetAnimInstance() : nullptr;
	if (AnimInstance == nullptr)
	{
		UE_LOG(LogAegisOdysseyAbilitySystem, Warning, TEXT("UGA_Harvest::PlayHarvestMontage: AnimInstance is null."));
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	if (HarvestMontageTask)
	{
		HarvestMontageTask->EndTask();
		HarvestMontageTask = nullptr;
	}

	// Ability 只负责维持这次挥击动作生命周期。
	// 真正的采集结算会等蒙太奇里的 UAOHarvestWindow 打开命中窗后，再由等待任务提交一次采集命中。
	HarvestMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		FName("PlayHarvestMontage"),
		Montage,
		PlayRate,
		StartSection,
		true,
		1.0f,
		StartTime,
		false);

	if (HarvestMontageTask == nullptr)
	{
		UE_LOG(LogAegisOdysseyAbilitySystem, Warning,
			TEXT("UGA_Harvest::PlayHarvestMontage: Failed to create montage task."));
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	HarvestMontageTask->OnBlendOut.AddDynamic(this, &UGA_Harvest::OnMontageBlendedOut);
	HarvestMontageTask->OnCompleted.AddDynamic(this, &UGA_Harvest::OnMontageCompleted);
	HarvestMontageTask->OnInterrupted.AddDynamic(this, &UGA_Harvest::OnMontageInterrupted);
	HarvestMontageTask->OnCancelled.AddDynamic(this, &UGA_Harvest::OnMontageCancelled);
	HarvestMontageTask->ReadyForActivation();
}

void UGA_Harvest::ClearHarvestWindowTag()
{
	UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo();
	if (AbilitySystemComponent && HarvestHitWindowTag.IsValid())
	{
		AbilitySystemComponent->RemoveLooseGameplayTag(HarvestHitWindowTag);
	}
}

bool UGA_Harvest::ResolveHarvestTargetFromHitContext(FAOHarvestHitContext& InOutHitContext) const
{
	// 这里做的是“命中窗口里的真实目标发现”。
	// 采集主链不再依赖状态树或交互系统预先给出正式目标，
	// 而是用这次挥击自己的轨迹和工具参数直接找出本次真正打到的对象。
	const UAOHarvestToolFragment* ToolFragment = InOutHitContext.RuntimeContext.ToolFragment.Get();
	AActor* HarvesterActor = InOutHitContext.HarvesterActor.Get();
	UWorld* World = HarvesterActor ? HarvesterActor->GetWorld() : nullptr;
	if (ToolFragment == nullptr || World == nullptr || HarvesterActor == nullptr)
	{
		return false;
	}

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(AOHarvestAcquireTarget), false);
	QueryParams.AddIgnoredActor(HarvesterActor);

	FHitResult HitResult;
	const FAOHarvestHitCheckConfig& HitCheckConfig = ToolFragment->HitCheckConfig;
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(HarvesterActor);
	const EDrawDebugTrace::Type DrawDebugType = bEnableHarvestDebugDraw ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None;
	const bool bHit = UKismetSystemLibrary::SphereTraceSingle(
		World,
		InOutHitContext.TraceStart,
		InOutHitContext.TraceEnd,
		HitCheckConfig.SweepRadius,
		UEngineTypes::ConvertToTraceType(ECC_Visibility),
		false,
		ActorsToIgnore,
		DrawDebugType,
		HitResult,
		true,
		FLinearColor::Yellow,
		FLinearColor::Green,
		HarvestDebugDrawDuration);

	if (!bHit)
	{
		return false;
	}

	AActor* HitActor = HitResult.GetActor();
	if (HitActor == nullptr)
	{
		return false;
	}

	UAOHarvestableComponent* HarvestableComponent = nullptr;
	if (const AAOHarvestableActor* HarvestableActor = Cast<AAOHarvestableActor>(HitActor))
	{
		HarvestableComponent = HarvestableActor->GetOwnedHarvestableComponent();
	}

	if (HarvestableComponent == nullptr && HitActor->GetClass()->ImplementsInterface(UAOHarvestableTarget::StaticClass()))
	{
		HarvestableComponent = IAOHarvestableTarget::Execute_GetHarvestableComponent(HitActor);
	}

	if (HarvestableComponent == nullptr)
	{
		HarvestableComponent = HitActor->FindComponentByClass<UAOHarvestableComponent>();
	}

	if (HarvestableComponent == nullptr)
	{
		return false;
	}

	// 只有当命中的对象真的暴露了 HarvestableComponent，
	// 它才会被认定为这次采集动作的正式目标。
	InOutHitContext.RuntimeContext.TargetActor = HitActor;
	InOutHitContext.RuntimeContext.TargetComponent = HarvestableComponent;
	InOutHitContext.HitLocation = HitResult.ImpactPoint;
	InOutHitContext.HitNormal = HitResult.ImpactNormal;
	InOutHitContext.bHasHitData = true;
	return true;
}

bool UGA_Harvest::TryResolveHarvestTraceFromMeshComponent(const UMeshComponent* MeshComponent, const FAOHarvestHitCheckConfig& HitCheckConfig,
	FVector& OutTraceStart, FVector& OutTraceEnd, FVector& OutFacingDirection) const
{
	if (MeshComponent == nullptr)
	{
		return false;
	}

	if (!MeshComponent->DoesSocketExist(HitCheckConfig.TraceStartSocketName) || !MeshComponent->DoesSocketExist(HitCheckConfig.TraceEndSocketName))
	{
		return false;
	}

	OutTraceStart = MeshComponent->GetSocketLocation(HitCheckConfig.TraceStartSocketName);
	OutTraceEnd = MeshComponent->GetSocketLocation(HitCheckConfig.TraceEndSocketName);
	OutFacingDirection = (OutTraceEnd - OutTraceStart).GetSafeNormal();
	return !OutFacingDirection.IsNearlyZero();
}

bool UGA_Harvest::ResolveHarvestTraceFromTool(FAOHarvestHitContext& InOutHitContext) const
{
	const UAOHarvestToolFragment* ToolFragment = InOutHitContext.RuntimeContext.ToolFragment.Get();
	const UAOHarvestToolInstance* ToolInstance = InOutHitContext.RuntimeContext.ToolInstance.Get();
	if (ToolFragment == nullptr || ToolInstance == nullptr)
	{
		return false;
	}

	const FAOHarvestHitCheckConfig& HitCheckConfig = ToolFragment->HitCheckConfig;
	if (HitCheckConfig.TraceStartSocketName.IsNone() || HitCheckConfig.TraceEndSocketName.IsNone())
	{
		return false;
	}

	for (AActor* SpawnedActor : ToolInstance->GetSpawnedActors())
	{
		if (SpawnedActor == nullptr)
		{
			continue;
		}

		for (const UActorComponent* Component : SpawnedActor->GetComponents())
		{
			const UMeshComponent* MeshComponent = Cast<UMeshComponent>(Component);
			if (MeshComponent == nullptr)
			{
				continue;
			}

			FVector TraceStart = FVector::ZeroVector;
			FVector TraceEnd = FVector::ZeroVector;
			FVector FacingDirection = FVector::ZeroVector;
			if (!TryResolveHarvestTraceFromMeshComponent(MeshComponent, HitCheckConfig, TraceStart, TraceEnd, FacingDirection))
			{
				continue;
			}

			InOutHitContext.TraceStart = TraceStart;
			InOutHitContext.TraceEnd = TraceEnd;
			InOutHitContext.FacingDirection = FacingDirection;
			return true;
		}
	}

	return false;
}

bool UGA_Harvest::BuildHarvestHitContext(FAOHarvestHitContext& OutHitContext) const
{
	if (!bHasValidHarvestContext)
	{
		return false;
	}

	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (AvatarActor == nullptr)
	{
		return false;
	}

	FAOHarvestRuntimeContext RuntimeContext = CurrentHarvestTargetData.RuntimeContext;
	if (!RebuildToolRuntimeContext(RuntimeContext))
	{
		return false;
	}

	// 这里先构造“这次挥击是从哪里打到哪里”的命中快照。
	// 这条轨迹必须来自工具自身 socket，和战斗武器判定保持同一思路。
	OutHitContext = FAOHarvestHitContext();
	OutHitContext.RuntimeContext = RuntimeContext;
	OutHitContext.RuntimeContext.TargetActor = nullptr;
	OutHitContext.RuntimeContext.TargetComponent = nullptr;
	OutHitContext.HarvesterActor = AvatarActor;

	if (!ResolveHarvestTraceFromTool(OutHitContext))
	{
		return false;
	}

	if (!ResolveHarvestTargetFromHitContext(OutHitContext))
	{
		return false;
	}

	// 命中到了对象还不够；目标组件当前还必须确实允许接受采集请求。
	return OutHitContext.RuntimeContext.TargetComponent != nullptr
		&& OutHitContext.RuntimeContext.TargetComponent->CanAcceptHarvestRequest();
}

void UGA_Harvest::DrawHarvestHitDebugPreview(const FAOHarvestHitContext& HitContext) const
{
	const UAOHarvestToolFragment* ToolFragment = HitContext.RuntimeContext.ToolFragment.Get();
	if (ToolFragment == nullptr)
	{
		return;
	}

	UAOHarvestResolver::DrawHarvestDebugPreview(HitContext, ToolFragment->HitCheckConfig, bEnableHarvestDebugDraw, HarvestDebugDrawDuration);
}

void UGA_Harvest::ShowHarvestProgressOnScreen(const FAOHarvestHitContext& HitContext, const FAOHarvestResult& HarvestResult) const
{
	if (!bShowHarvestProgressOnScreen || GEngine == nullptr || HitContext.RuntimeContext.TargetComponent == nullptr)
	{
		return;
	}

	const float TotalProgress = HitContext.RuntimeContext.TargetComponent->GetTotalHarvestProgress();
	const float RemainingProgress = HarvestResult.RemainingProgress;
	const float ProgressPercent = TotalProgress > 0.0f ? ((TotalProgress - RemainingProgress) / TotalProgress) * 100.0f : 0.0f;
	const FString DebugMessage = FString::Printf(
		TEXT("Harvest %s  Progress: %.1f / %.1f  (%.0f%%)"),
		*GetNameSafe(HitContext.RuntimeContext.TargetActor),
		RemainingProgress,
		TotalProgress,
		ProgressPercent);

	GEngine->AddOnScreenDebugMessage(
		reinterpret_cast<uint64>(HitContext.RuntimeContext.TargetActor.Get()),
		1.5f,
		HarvestResult.bDepletedAfterHit ? FColor::Yellow : FColor::Green,
		DebugMessage);
}

FGameplayCueParameters UGA_Harvest::BuildHarvestCueParameters(const FAOHarvestHitContext& HitContext,
	const UAOHarvestableDefinition* HarvestableDefinition) const
{
	FGameplayCueParameters CueParameters;
	CueParameters.Location = HitContext.TraceEnd;
	CueParameters.Normal = HitContext.FacingDirection.GetSafeNormal();
	CueParameters.AbilityLevel = GetAbilityLevel();
	CueParameters.EffectCauser = GetAvatarActorFromActorInfo();
	CueParameters.Instigator = GetAvatarActorFromActorInfo();
	CueParameters.SourceObject = const_cast<UAOHarvestableDefinition*>(HarvestableDefinition);
	return CueParameters;
}

void UGA_Harvest::ExecuteHarvestCue(const FAOHarvestHitContext& HitContext, const FAOHarvestResult& HarvestResult) const
{
	const UAOHarvestableDefinition* HarvestableDefinition = HitContext.RuntimeContext.TargetComponent != nullptr
		? HitContext.RuntimeContext.TargetComponent->GetHarvestableDefinition()
		: nullptr;
	if (HarvestableDefinition == nullptr)
	{
		return;
	}

	const FGameplayTag CueTag = HarvestResult.bDepletedAfterHit
		? AOHarvestCueTags::GameplayCue_Harvest_Depleted
		: AOHarvestCueTags::GameplayCue_Harvest_Hit;
	const FAOHarvestCueVisualSet& VisualSet = HarvestResult.bDepletedAfterHit
		? HarvestableDefinition->HarvestDepletedCueVisuals
		: HarvestableDefinition->HarvestHitCueVisuals;
	const FGameplayCueParameters CueParameters = BuildHarvestCueParameters(HitContext, HarvestableDefinition);
	if (HitContext.RuntimeContext.TargetComponent != nullptr)
	{
		HitContext.RuntimeContext.TargetComponent->MulticastPlayHarvestCue(
			FVector_NetQuantize(HitContext.TraceEnd),
			FVector_NetQuantizeNormal(HitContext.FacingDirection.GetSafeNormal()),
			HarvestResult.bDepletedAfterHit);
	}

	if (UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo())
	{
		AbilitySystemComponent->ExecuteGameplayCue(CueTag, CueParameters);
	}
}

void UGA_Harvest::TryProcessHarvestHitOnAuthority()
{
	// 这个 Ability 仍然是 LocalPredicted。
	// 客户端负责表现与输入生命周期，真正的采集结算只允许权威端在命中窗内处理。
	if (!GetAvatarActorFromActorInfo() || !GetAvatarActorFromActorInfo()->HasAuthority())
	{
		return;
	}

	ExecuteHarvestHit();
}

void UGA_Harvest::ExecuteHarvestHit()
{
	FAOHarvestHitContext HitContext;
	if (!BuildHarvestHitContext(HitContext))
	{
		UE_LOG(LogAegisOdysseyAbilitySystem, Warning, TEXT("UGA_Harvest::ExecuteHarvestHit: Failed to build hit context."));
		return;
	}

	DrawHarvestHitDebugPreview(HitContext);

	FAOHarvestResult HarvestResult;
	if (!UAOHarvestResolver::ResolveHarvestRequest(HitContext, HarvestResult, bEnableHarvestDebugDraw, HarvestDebugDrawDuration))
	{
		// 走到这里说明“这次挥击确实打到了一个可采对象”，
		// 但在服务端最终复核里没有通过距离、朝向、遮挡、对象接收规则等校验。
		UE_LOG(LogAegisOdysseyAbilitySystem, Verbose, TEXT("UGA_Harvest::ExecuteHarvestHit: Request rejected. Reason=%s"),
			*HarvestResult.RejectReason.ToString());
		return;
	}

	// 采集节点自己解析本次请求对应的最终扣减结果。
	// 外部只提供理论请求值，避免别处拿到副本状态再参与进度计算。
	if (!HitContext.RuntimeContext.TargetComponent->ResolveHarvestProgressRequest(HarvestResult.RequestedProgress, HarvestResult))
	{
		UE_LOG(LogAegisOdysseyAbilitySystem, Verbose,
			TEXT("UGA_Harvest::ExecuteHarvestHit: Progress resolve failed. Target=%s RequestedProgress=%.2f"),
			*GetNameSafe(HitContext.RuntimeContext.TargetActor),
			HarvestResult.RequestedProgress);
		return;
	}

	if (!UAOHarvestResolver::FinalizeHarvestRewards(HitContext, HarvestResult))
	{
		UE_LOG(LogAegisOdysseyAbilitySystem, Warning,
			TEXT("UGA_Harvest::ExecuteHarvestHit: Reward finalization failed. Target=%s"),
			*GetNameSafe(HitContext.RuntimeContext.TargetActor));
		return;
	}

	FAOHarvestLifecycleContext LifecycleContext;
	LifecycleContext.HarvestResult = HarvestResult;
	LifecycleContext.HarvesterActor = HitContext.HarvesterActor.Get();
	LifecycleContext.HarvesterForward =
		HitContext.HarvesterActor != nullptr
			? HitContext.HarvesterActor->GetActorForwardVector().GetSafeNormal()
			: HitContext.FacingDirection.GetSafeNormal();
	LifecycleContext.bHasHarvesterForward = !LifecycleContext.HarvesterForward.IsNearlyZero();
	LifecycleContext.HitLocation = HitContext.HitLocation;
	LifecycleContext.HitNormal = HitContext.HitNormal;
	LifecycleContext.HitDirection = (HitContext.TraceEnd - HitContext.TraceStart).GetSafeNormal();
	LifecycleContext.bHasHitData = HitContext.bHasHitData;

	// 先提交奖励，再确认本次采集整体成立。
	// 这样可以保证“背包装不下时，这次采集整体不结算”，不会出现树已经砍空但奖励没进包的问题。
	if (!TryCommitHarvestRewards(HitContext, HarvestResult))
	{
		UE_LOG(LogAegisOdysseyAbilitySystem, Verbose,
			TEXT("UGA_Harvest::ExecuteHarvestHit: Reward commit failed. Target=%s"),
			*GetNameSafe(HitContext.RuntimeContext.TargetActor));
		return;
	}

	if (!HitContext.RuntimeContext.TargetComponent->ApplyHarvestResultWithContext(LifecycleContext))
	{
		UE_LOG(LogAegisOdysseyAbilitySystem, Warning, TEXT("UGA_Harvest::ExecuteHarvestHit: Failed to apply harvest result."));
		return;
	}

	ExecuteHarvestCue(HitContext, HarvestResult);
	ShowHarvestProgressOnScreen(HitContext, HarvestResult);

	UE_LOG(LogAegisOdysseyAbilitySystem, Log,
		TEXT("UGA_Harvest::ExecuteHarvestHit: Success Target=%s AppliedProgress=%.2f RemainingProgress=%.2f RewardCount=%d"),
		*GetNameSafe(HitContext.RuntimeContext.TargetActor),
		HarvestResult.AppliedProgress,
		HarvestResult.RemainingProgress,
		HarvestResult.RewardEntries.Num());

	if (HarvestResult.bDepletedAfterHit)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	}
}

bool UGA_Harvest::BuildRewardItemBatch(const FAOHarvestResult& HarvestResult, TArray<FAOItemCatalogRow>& OutItemRows, TArray<int32>& OutItemCounts) const
{
	OutItemRows.Reset();
	OutItemCounts.Reset();

	const UDataTable* ItemCatalogDataTable = UAOGameData::Get().GetItemCatalogDataTable();
	if (ItemCatalogDataTable == nullptr)
	{
		UE_LOG(LogAegisOdysseyAbilitySystem, Warning,
			TEXT("UGA_Harvest::BuildRewardItemBatch: ItemCatalogDataTable is not cached in AOGameData."));
		return false;
	}

	OutItemRows.Reserve(HarvestResult.RewardEntries.Num());
	OutItemCounts.Reserve(HarvestResult.RewardEntries.Num());

	for (const FAOHarvestRewardEntry& RewardEntry : HarvestResult.RewardEntries)
	{
		if (RewardEntry.ItemId == INDEX_NONE || RewardEntry.Count <= 0)
		{
			UE_LOG(LogAegisOdysseyAbilitySystem, Warning,
				TEXT("UGA_Harvest::BuildRewardItemBatch: Invalid reward entry. ItemId=%d Count=%d"),
				RewardEntry.ItemId,
				RewardEntry.Count);
			return false;
		}

		const FAOItemCatalogRow* ItemRow = FindItemCatalogRowById(*ItemCatalogDataTable, RewardEntry.ItemId);
		if (ItemRow == nullptr || ItemRow->ItemDefinitionClass == nullptr)
		{
			UE_LOG(LogAegisOdysseyAbilitySystem, Warning,
				TEXT("UGA_Harvest::BuildRewardItemBatch: Failed to resolve catalog row for ItemId=%d"),
				RewardEntry.ItemId);
			return false;
		}

		if (UAOInventoryItemDefinition::ResolveItemInstanceClass(ItemRow->ItemDefinitionClass) == nullptr)
		{
			UE_LOG(LogAegisOdysseyAbilitySystem, Warning,
				TEXT("UGA_Harvest::BuildRewardItemBatch: Failed to resolve item instance class for ItemId=%d"),
				RewardEntry.ItemId);
			return false;
		}

		OutItemRows.Add(*ItemRow);
		OutItemCounts.Add(RewardEntry.Count);
	}

	return true;
}

bool UGA_Harvest::BuildRewardReceiveBatch(const FAOHarvestResult& HarvestResult, FAOInventoryReceiveBatch& OutReceiveBatch) const
{
	OutReceiveBatch = FAOInventoryReceiveBatch();

	TArray<FAOItemCatalogRow> ItemRows;
	TArray<int32> ItemCounts;
	if (!BuildRewardItemBatch(HarvestResult, ItemRows, ItemCounts))
	{
		return false;
	}

	for (int32 Index = 0; Index < ItemRows.Num(); ++Index)
	{
		const FAOItemCatalogRow& ItemRow = ItemRows[Index];
		const int32 Count = ItemCounts[Index];

		// 这里把采集结果统一翻译成库存系统已经认识的 DefinitionEntry 批次。
		// Harvest 不绕开正式库存入口，避免再自己拼一套旁路入包逻辑。
		if (Count <= 0 || ItemRow.ItemDefinitionClass == nullptr)
		{
			return false;
		}

		FAOInventoryDefinitionEntry& DefinitionEntry = OutReceiveBatch.DefinitionEntries.AddDefaulted_GetRef();
		DefinitionEntry.Count = Count;
		DefinitionEntry.ItemDefinitionClass = ItemRow.ItemDefinitionClass;
	}

	return true;
}

bool UGA_Harvest::TryCommitHarvestRewards(const FAOHarvestHitContext& HitContext, const FAOHarvestResult& HarvestResult)
{
	if (HarvestResult.RewardEntries.IsEmpty())
	{
		UE_LOG(LogAegisOdysseyAbilitySystem, Verbose,
			TEXT("UGA_Harvest::TryCommitHarvestRewards: RewardEntries is empty. Treating as no-op success. Target=%s"),
			*GetNameSafe(HitContext.RuntimeContext.TargetActor));
		return true;
	}

	FAOInventoryReceiveBatch ReceiveBatch;
	if (!BuildRewardReceiveBatch(HarvestResult, ReceiveBatch))
	{
		UE_LOG(LogAegisOdysseyAbilitySystem, Warning,
			TEXT("UGA_Harvest::TryCommitHarvestRewards: BuildRewardReceiveBatch failed. Target=%s RewardEntryCount=%d"),
			*GetNameSafe(HitContext.RuntimeContext.TargetActor),
			HarvestResult.RewardEntries.Num());
		return false;
	}

	if (!UAOInventoryStatics::CanActorFullyAcceptInventoryBatch(HitContext.HarvesterActor, ReceiveBatch))
	{
		UE_LOG(LogAegisOdysseyAbilitySystem, Warning,
			TEXT("UGA_Harvest::TryCommitHarvestRewards: Harvester cannot fully accept inventory batch. Harvester=%s DefinitionEntryCount=%d"),
			*GetNameSafe(HitContext.HarvesterActor),
			ReceiveBatch.DefinitionEntries.Num());
		return false;
	}

	if (!UAOInventoryStatics::TryAddInventoryBatchToActor(HitContext.HarvesterActor, ReceiveBatch))
	{
		UE_LOG(LogAegisOdysseyAbilitySystem, Warning,
			TEXT("UGA_Harvest::TryCommitHarvestRewards: TryAddInventoryBatchToActor failed after acceptance check. Harvester=%s DefinitionEntryCount=%d"),
			*GetNameSafe(HitContext.HarvesterActor),
			ReceiveBatch.DefinitionEntries.Num());
		return false;
	}

	return true;
}

UAT_WaitHarvestHit::UAT_WaitHarvestHit(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bTickingTask = true;
}

UAT_WaitHarvestHit* UAT_WaitHarvestHit::WaitHarvestHit(UGA_Harvest* OwningAbility)
{
	UAT_WaitHarvestHit* Task = NewAbilityTask<UAT_WaitHarvestHit>(OwningAbility);
	Task->HarvestAbility = OwningAbility;
	return Task;
}

void UAT_WaitHarvestHit::Activate()
{
	Super::Activate();

	if (!HarvestAbility.IsValid())
	{
		EndTask();
	}
}

void UAT_WaitHarvestHit::TickTask(float DeltaTime)
{
	Super::TickTask(DeltaTime);

	if (!HarvestAbility.IsValid() || !AbilitySystemComponent.IsValid())
	{
		EndTask();
		return;
	}

	const FGameplayTag HarvestWindowTag = HarvestAbility->HarvestHitWindowTag;
	const bool bIsHitWindowActive = HarvestWindowTag.IsValid()
		&& AbilitySystemComponent->GetOwnedGameplayTags().HasTagExact(HarvestWindowTag);

	if (!bIsHitWindowActive)
	{
		if (bWasHitWindowActiveLastTick)
		{
			bSubmittedHitInCurrentWindow = false;
		}

		bWasHitWindowActiveLastTick = false;
		return;
	}

	bWasHitWindowActiveLastTick = true;

	if (bSubmittedHitInCurrentWindow)
	{
		return;
	}

	// 每个采集命中窗只允许提交一次采集命中。
	// 如果同一段蒙太奇里配置了多个 HarvestWindow，前一个窗口关闭后，
	// 这里会在下一个窗口重新允许提交。
	HarvestAbility->TryProcessHarvestHitOnAuthority();
	bSubmittedHitInCurrentWindow = true;
}

void UAT_WaitHarvestHit::OnDestroy(bool bInOwnerFinished)
{
	HarvestAbility = nullptr;
	bWasHitWindowActiveLastTick = false;
	bSubmittedHitInCurrentWindow = false;
	Super::OnDestroy(bInOwnerFinished);
}

