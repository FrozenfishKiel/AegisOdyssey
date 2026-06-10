#include "AegisOdyssey/Harvest/StateTree/STT_PlayHarvest.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayAbilitySpec.h"
#include "GameplayPrediction.h"
#include "StateTreeExecutionContext.h"
#include "AegisOdyssey/Equipment/AOEquipmentInstance.h"
#include "AegisOdyssey/Equipment/AOWeaponManagerComponent.h"
#include "AegisOdyssey/Harvest/Definition/AOHarvestToolDefinition.h"
#include "AegisOdyssey/Harvest/Fragments/AOHarvestToolFragment.h"
#include "AegisOdyssey/Harvest/Items/AOHarvestToolInstance.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(STT_PlayHarvest)

EStateTreeRunStatus FSTT_PlayHarvest::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	InstanceData.bActivated = false;
	InstanceData.AbilitySystemComponent = nullptr;
	InstanceData.AbilitySpecHandle = FGameplayAbilitySpecHandle();
	InstanceData.HarvestToolDefinition = nullptr;
	InstanceData.HarvestToolInstance = nullptr;
	InstanceData.HarvestToolFragment = nullptr;

	if (!InstanceData.InputTag.IsValid())
	{
		UE_LOG(LogStateTree, Warning, TEXT("FSTT_PlayHarvest::EnterState: InputTag is invalid."));
		return EStateTreeRunStatus::Failed;
	}

	if (InstanceData.Montage == nullptr)
	{
		UE_LOG(LogStateTree, Warning, TEXT("FSTT_PlayHarvest::EnterState: Montage is not configured."));
		return EStateTreeRunStatus::Failed;
	}

	AActor* OwnerActor = ResolveOwningActor(Context);
	if (OwnerActor == nullptr)
	{
		UE_LOG(LogStateTree, Warning, TEXT("FSTT_PlayHarvest::EnterState: Failed to resolve owner actor."));
		return EStateTreeRunStatus::Failed;
	}

	if (!ResolveCurrentHarvestTool(InstanceData, OwnerActor))
	{
		UE_LOG(LogStateTree, Warning, TEXT("FSTT_PlayHarvest::EnterState: Current equipped item is not a valid harvest tool."));
		return EStateTreeRunStatus::Failed;
	}

	InstanceData.AbilitySystemComponent = ResolveAbilitySystemComponent(OwnerActor);
	if (InstanceData.AbilitySystemComponent == nullptr)
	{
		UE_LOG(LogStateTree, Warning, TEXT("FSTT_PlayHarvest::EnterState: Failed to resolve AbilitySystemComponent."));
		return EStateTreeRunStatus::Failed;
	}

	if (!GatherMatchingAbilitySpecHandle(*InstanceData.AbilitySystemComponent, InstanceData.InputTag, InstanceData.AbilitySpecHandle))
	{
		UE_LOG(LogStateTree, Warning, TEXT("FSTT_PlayHarvest::EnterState: No harvest ability matched input tag %s."),
			*InstanceData.InputTag.ToString());
		return EStateTreeRunStatus::Failed;
	}

	FAOHarvestTargetData* TargetData = new FAOHarvestTargetData();
	TargetData->InputTag = InstanceData.InputTag;
	TargetData->Montage = InstanceData.Montage;
	TargetData->PlayRate = InstanceData.PlayRate;
	TargetData->StartSection = InstanceData.StartSection;
	TargetData->StartTime = InstanceData.StartTime;
	TargetData->RuntimeContext.ToolDefinition = InstanceData.HarvestToolDefinition;
	TargetData->RuntimeContext.ToolInstance = InstanceData.HarvestToolInstance;

	FGameplayAbilityTargetDataHandle TargetDataHandle;
	TargetDataHandle.Data.Add(TSharedPtr<FGameplayAbilityTargetData>(TargetData));

	FGameplayEventData EventData;
	EventData.EventTag = InstanceData.InputTag;
	EventData.TargetData = TargetDataHandle;

	// StateTree 在这里既负责驱动采集 Ability 发起，也负责像战斗任务那样
	// 把这次动作要播的蒙太奇参数一起下发。
	// 这里故意不下发正式采集目标。
	// StateTree 只负责“角色现在发起一次采集挥击”和“这次挥击使用哪把工具、播哪段动作”，
	// 真正打到谁，要等命中窗口里按工具参数做命中检测后再决定。
	InstanceData.bActivated = InstanceData.AbilitySystemComponent->InternalTryActivateAbility(
		InstanceData.AbilitySpecHandle,
		FPredictionKey(),
		nullptr,
		nullptr,
		&EventData);

	return InstanceData.bActivated ? EStateTreeRunStatus::Running : EStateTreeRunStatus::Failed;
}

EStateTreeRunStatus FSTT_PlayHarvest::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	if (InstanceData.AbilitySystemComponent == nullptr || !InstanceData.AbilitySpecHandle.IsValid())
	{
		return EStateTreeRunStatus::Failed;
	}

	FGameplayAbilitySpec* AbilitySpec = InstanceData.AbilitySystemComponent->FindAbilitySpecFromHandle(InstanceData.AbilitySpecHandle);
	if (AbilitySpec == nullptr)
	{
		return EStateTreeRunStatus::Failed;
	}

	InstanceData.bActivated = AbilitySpec->IsActive();
	return InstanceData.bActivated ? EStateTreeRunStatus::Running : EStateTreeRunStatus::Succeeded;
}

void FSTT_PlayHarvest::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
}

AActor* FSTT_PlayHarvest::ResolveOwningActor(const FStateTreeExecutionContext& Context) const
{
	AActor* OwnerActor = Cast<AActor>(Context.GetOwner());
	if (OwnerActor == nullptr)
	{
		return nullptr;
	}

	if (AController* OwnerController = Cast<AController>(OwnerActor))
	{
		return OwnerController->GetPawn();
	}

	return OwnerActor;
}

UAbilitySystemComponent* FSTT_PlayHarvest::ResolveAbilitySystemComponent(AActor* OwnerActor) const
{
	return OwnerActor ? UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OwnerActor) : nullptr;
}

bool FSTT_PlayHarvest::ResolveCurrentHarvestTool(FInstanceDataType& InstanceData, AActor* OwnerActor) const
{
	if (OwnerActor == nullptr)
	{
		return false;
	}

	UAOWeaponManagerComponent* WeaponManagerComponent = OwnerActor->FindComponentByClass<UAOWeaponManagerComponent>();
	if (WeaponManagerComponent == nullptr)
	{
		return false;
	}

	const UAOEquipmentInstance* CurrentEquipmentInstance = WeaponManagerComponent->GetCurrentWeaponInstance();
	const UAOHarvestToolInstance* HarvestToolInstance = Cast<UAOHarvestToolInstance>(CurrentEquipmentInstance);
	const UAOHarvestToolDefinition* HarvestToolDefinition =
		HarvestToolInstance
			? HarvestToolInstance->GetHarvestToolDefinition()
			: Cast<UAOHarvestToolDefinition>(CurrentEquipmentInstance ? CurrentEquipmentInstance->GetItemCDO() : nullptr);
	if (HarvestToolDefinition == nullptr)
	{
		return false;
	}

	const UAOHarvestToolFragment* HarvestToolFragment =
		HarvestToolInstance
			? HarvestToolInstance->GetHarvestToolFragment()
			: HarvestToolDefinition->FindHarvestToolFragment();
	if (HarvestToolFragment == nullptr)
	{
		return false;
	}

	InstanceData.HarvestToolDefinition = HarvestToolDefinition;
	InstanceData.HarvestToolInstance = HarvestToolInstance;
	InstanceData.HarvestToolFragment = HarvestToolFragment;
	return true;
}

bool FSTT_PlayHarvest::GatherMatchingAbilitySpecHandle(const UAbilitySystemComponent& AbilitySystemComponent, const FGameplayTag& InputTag, FGameplayAbilitySpecHandle& OutHandle) const
{
	OutHandle = FGameplayAbilitySpecHandle();

	for (const FGameplayAbilitySpec& AbilitySpec : AbilitySystemComponent.GetActivatableAbilities())
	{
		if (AbilitySpec.Ability == nullptr)
		{
			continue;
		}

		if (!AbilitySpec.Ability->GetAssetTags().HasTagExact(InputTag))
		{
			continue;
		}

		OutHandle = AbilitySpec.Handle;
		return true;
	}

	return false;
}
