// Fill out your copyright notice in the Description page of Project Settings.

#include "AegisOdyssey/Character/Enemies/AI/AOAIInventoryRuntimeUseLibrary.h"

#include "AegisOdyssey/Equipment/AOQuickBarComponent.h"
#include "AegisOdyssey/Inventory/AOInventoryComponent.h"
#include "AegisOdyssey/Inventory/AOInventoryItemDefinition.h"
#include "AegisOdyssey/Inventory/AOInventoryItemInstance.h"
#include "GameFramework/Pawn.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOAIInventoryRuntimeUseLibrary)

bool UAOAIInventoryRuntimeUseLibrary::TryResolveUseCommand(APawn* UserPawn, const FAOAIInventoryUseCommand& Command, FAOAIResolvedInventoryUseTarget& OutResolvedTarget)
{
	OutResolvedTarget = FAOAIResolvedInventoryUseTarget();

	if (UserPawn == nullptr)
	{
		return false;
	}

	switch (Command.CommandType)
	{
	case EAOAIInventoryUseCommandType::InventorySearch:
		return TryResolveInventorySearchCommand(UserPawn, Command, OutResolvedTarget);

	case EAOAIInventoryUseCommandType::QuickBarSlot:
		return TryResolveQuickBarCommand(UserPawn, Command, OutResolvedTarget);

	default:
		return false;
	}
}

bool UAOAIInventoryRuntimeUseLibrary::TryExecuteUseCommand(APawn* UserPawn, const FAOAIInventoryUseCommand& Command, FAOAIInventoryUseExecutionResult& OutExecutionResult)
{
	OutExecutionResult = FAOAIInventoryUseExecutionResult();

	if (!TryResolveUseCommand(UserPawn, Command, OutExecutionResult.ResolvedTarget))
	{
		return false;
	}

	return TryExecuteResolvedTarget(UserPawn, OutExecutionResult.ResolvedTarget, OutExecutionResult);
}

bool UAOAIInventoryRuntimeUseLibrary::TryExecuteResolvedTarget(
	APawn* UserPawn,
	const FAOAIResolvedInventoryUseTarget& ResolvedTarget,
	FAOAIInventoryUseExecutionResult& OutExecutionResult)
{
	OutExecutionResult = FAOAIInventoryUseExecutionResult();
	OutExecutionResult.ResolvedTarget = ResolvedTarget;

	if (UserPawn == nullptr || ResolvedTarget.InventoryComponent == nullptr)
	{
		return false;
	}

	if (ResolvedTarget.bUsedQuickBarSlot)
	{
		if (UAOQuickBarComponent* QuickBarComponent = Cast<UAOQuickBarComponent>(ResolvedTarget.InventoryComponent.Get()))
		{
			if (!QuickBarComponent->IsValidInventorySlotIndex(ResolvedTarget.QuickBarSlotIndex))
			{
				return false;
			}

			QuickBarComponent->SetActivateIndex(ResolvedTarget.QuickBarSlotIndex);
			OutExecutionResult.bSucceeded = true;
			return true;
		}

		return false;
	}

	if (UAOInventoryComponent* InventoryComponent = ResolvedTarget.InventoryComponent.Get())
	{
		if (!InventoryComponent->IsValidInventorySlotIndex(ResolvedTarget.SlotIndex))
		{
			return false;
		}

		OutExecutionResult.bSucceeded = InventoryComponent->TryUseItemAtSlot(ResolvedTarget.SlotIndex, UserPawn);
		return OutExecutionResult.bSucceeded;
	}

	return false;
}

int32 UAOAIInventoryRuntimeUseLibrary::CountMatchingInventoryEntries(APawn* UserPawn, const FAOAIInventoryUseCommand& Command, bool bOnlyCountUsableEntries)
{
	TArray<FAOAIResolvedInventoryUseTarget> ResolvedTargets;
	GatherMatchingInventoryTargets(UserPawn, Command, bOnlyCountUsableEntries, ResolvedTargets);
	return ResolvedTargets.Num();
}

void UAOAIInventoryRuntimeUseLibrary::GatherMatchingInventoryTargets(
	APawn* UserPawn,
	const FAOAIInventoryUseCommand& Command,
	bool bOnlyUsableEntries,
	TArray<FAOAIResolvedInventoryUseTarget>& OutResolvedTargets)
{
	OutResolvedTargets.Reset();

	if (UserPawn == nullptr)
	{
		return;
	}

	// 这里会把显式搜索范围内的所有合法目标都展开出来，供决策层后续按具体 resolved target 逐个选择。
	switch (Command.CommandType)
	{
	case EAOAIInventoryUseCommandType::QuickBarSlot:
	{
		UAOQuickBarComponent* QuickBarComponent = UserPawn->FindComponentByClass<UAOQuickBarComponent>();
		if (QuickBarComponent == nullptr)
		{
			return;
		}

		TArray<int32> CandidateSlotIndices;
		GatherQuickBarCandidateSlotIndices(Command, CandidateSlotIndices);
		OutResolvedTargets.Reserve(CandidateSlotIndices.Num());

		for (int32 CandidateSlotIndex : CandidateSlotIndices)
		{
			if (!QuickBarComponent->IsValidInventorySlotIndex(CandidateSlotIndex) || !QuickBarComponent->HasItemAtSlot(CandidateSlotIndex))
			{
				continue;
			}

			if (bOnlyUsableEntries && !QuickBarComponent->CanUseItemAtSlot(CandidateSlotIndex, UserPawn))
			{
				continue;
			}

			FAOAIResolvedInventoryUseTarget& ResolvedTarget = OutResolvedTargets.AddDefaulted_GetRef();
			ResolvedTarget.InventoryComponent = QuickBarComponent;
			ResolvedTarget.ItemInstance = nullptr;
			ResolvedTarget.SlotIndex = INDEX_NONE;
			ResolvedTarget.bUsedQuickBarSlot = true;
			ResolvedTarget.QuickBarSlotIndex = CandidateSlotIndex;

			if (const FAOInventoryEntry* InventoryEntry = QuickBarComponent->GetInventoryEntryAtSlot(CandidateSlotIndex))
			{
				ResolvedTarget.ItemInstance = InventoryEntry->Instance;
			}
		}

		return;
	}

	case EAOAIInventoryUseCommandType::InventorySearch:
		break;

	default:
		return;
	}

	if (Command.AllowedInventoryComponentClasses.IsEmpty())
	{
		return;
	}

	TArray<UAOInventoryComponent*> CandidateInventories;
	CollectOwnedInventoryComponents(UserPawn, Command.AllowedInventoryComponentClasses, CandidateInventories);
	for (UAOInventoryComponent* InventoryComponent : CandidateInventories)
	{
		if (InventoryComponent == nullptr || !DoesInventoryComponentMatchScope(*InventoryComponent, Command.AllowedInventoryComponentClasses))
		{
			continue;
		}

		const int32 SlotCount = InventoryComponent->GetInventorySlotCount();
		for (int32 SlotIndex = 0; SlotIndex < SlotCount; ++SlotIndex)
		{
			FAOAIInventoryItemQuery Query = Command.ItemQuery;
			// InventorySearch 阶段不再只吃第一个命中项，而是保留全部合法命中结果。
			if (DoesInventoryEntryMatchQuery(*InventoryComponent, SlotIndex, UserPawn, Query))
			{
				const FAOInventoryEntry* InventoryEntry = InventoryComponent->GetInventoryEntryAtSlot(SlotIndex);
				if (InventoryEntry == nullptr || InventoryEntry->Instance == nullptr)
				{
					continue;
				}

				FAOAIResolvedInventoryUseTarget& ResolvedTarget = OutResolvedTargets.AddDefaulted_GetRef();
				ResolvedTarget.InventoryComponent = InventoryComponent;
				ResolvedTarget.ItemInstance = InventoryEntry->Instance;
				ResolvedTarget.SlotIndex = SlotIndex;
				ResolvedTarget.bUsedQuickBarSlot = false;
				ResolvedTarget.QuickBarSlotIndex = INDEX_NONE;
			}
		}
	}
}

void UAOAIInventoryRuntimeUseLibrary::GatherQuickBarCandidateSlotIndices(const FAOAIInventoryUseCommand& Command, TArray<int32>& OutSlotIndices)
{
	OutSlotIndices.Reset();

	if (!Command.QuickBarSlotIndices.IsEmpty())
	{
		for (int32 QuickBarSlotIndex : Command.QuickBarSlotIndices)
		{
			if (QuickBarSlotIndex >= 0)
			{
				OutSlotIndices.AddUnique(QuickBarSlotIndex);
			}
		}

		return;
	}

	if (Command.QuickBarSlotIndex >= 0)
	{
		OutSlotIndices.Add(Command.QuickBarSlotIndex);
	}
}

void UAOAIInventoryRuntimeUseLibrary::CollectOwnedInventoryComponents(APawn* UserPawn, const TArray<TSubclassOf<UAOInventoryComponent>>& AllowedInventoryComponentClasses, TArray<UAOInventoryComponent*>& OutInventoryComponents)
{
	OutInventoryComponents.Reset();

	if (UserPawn == nullptr)
	{
		return;
	}

	TArray<UAOInventoryComponent*> OwnedInventoryComponents;
	UserPawn->GetComponents(OwnedInventoryComponents);

	for (const TSubclassOf<UAOInventoryComponent>& AllowedClass : AllowedInventoryComponentClasses)
	{
		if (AllowedClass == nullptr)
		{
			continue;
		}

		for (UAOInventoryComponent* InventoryComponent : OwnedInventoryComponents)
		{
			if (InventoryComponent == nullptr || InventoryComponent->IsA(UAOQuickBarComponent::StaticClass()))
			{
				continue;
			}

			if (InventoryComponent->IsA(AllowedClass))
			{
				OutInventoryComponents.AddUnique(InventoryComponent);
			}
		}
	}
}

bool UAOAIInventoryRuntimeUseLibrary::DoesInventoryComponentMatchScope(const UAOInventoryComponent& InventoryComponent, const TArray<TSubclassOf<UAOInventoryComponent>>& AllowedInventoryComponentClasses)
{
	for (const TSubclassOf<UAOInventoryComponent>& AllowedClass : AllowedInventoryComponentClasses)
	{
		if (AllowedClass != nullptr && InventoryComponent.IsA(AllowedClass))
		{
			return true;
		}
	}

	return false;
}

bool UAOAIInventoryRuntimeUseLibrary::TryResolveInventorySearchCommand(APawn* UserPawn, const FAOAIInventoryUseCommand& Command, FAOAIResolvedInventoryUseTarget& OutResolvedTarget)
{
	TArray<FAOAIResolvedInventoryUseTarget> ResolvedTargets;
	GatherMatchingInventoryTargets(UserPawn, Command, Command.ItemQuery.bRequireUsableFromInventory, ResolvedTargets);
	if (ResolvedTargets.IsEmpty())
	{
		return false;
	}

	// 旧接口仍只返回一个结果，但候选集已经在上层展开并完成了精细选择。
	OutResolvedTarget = ResolvedTargets[0];
	return true;
}

bool UAOAIInventoryRuntimeUseLibrary::TryResolveQuickBarCommand(APawn* UserPawn, const FAOAIInventoryUseCommand& Command, FAOAIResolvedInventoryUseTarget& OutResolvedTarget)
{
	if (UserPawn == nullptr)
	{
		return false;
	}

	UAOQuickBarComponent* QuickBarComponent = UserPawn->FindComponentByClass<UAOQuickBarComponent>();
	if (QuickBarComponent == nullptr)
	{
		return false;
	}

	TArray<int32> CandidateSlotIndices;
	GatherQuickBarCandidateSlotIndices(Command, CandidateSlotIndices);
	if (CandidateSlotIndices.Num() != 1)
	{
		return false;
	}

	const int32 CandidateSlotIndex = CandidateSlotIndices[0];
	if (!QuickBarComponent->IsValidInventorySlotIndex(CandidateSlotIndex) || !QuickBarComponent->HasItemAtSlot(CandidateSlotIndex))
	{
		return false;
	}

	OutResolvedTarget.InventoryComponent = QuickBarComponent;
	OutResolvedTarget.ItemInstance = nullptr;
	OutResolvedTarget.SlotIndex = INDEX_NONE;
	OutResolvedTarget.bUsedQuickBarSlot = true;
	OutResolvedTarget.QuickBarSlotIndex = CandidateSlotIndex;

	if (const FAOInventoryEntry* InventoryEntry = QuickBarComponent->GetInventoryEntryAtSlot(CandidateSlotIndex))
	{
		OutResolvedTarget.ItemInstance = InventoryEntry->Instance;
	}

	return true;
}

bool UAOAIInventoryRuntimeUseLibrary::DoesResolvedTargetMatchCommand(const FAOAIResolvedInventoryUseTarget& ResolvedTarget, const FAOAIInventoryUseCommand& Command)
{
	if (ResolvedTarget.bUsedQuickBarSlot)
	{
		if (Command.CommandType != EAOAIInventoryUseCommandType::QuickBarSlot)
		{
			return false;
		}

		if (!Command.QuickBarSlotIndices.IsEmpty())
		{
			return Command.QuickBarSlotIndices.Contains(ResolvedTarget.QuickBarSlotIndex);
		}

		return ResolvedTarget.QuickBarSlotIndex == Command.QuickBarSlotIndex;
	}

	return Command.CommandType == EAOAIInventoryUseCommandType::InventorySearch;
}

bool UAOAIInventoryRuntimeUseLibrary::DoesInventoryEntryMatchQuery(UAOInventoryComponent& InventoryComponent, int32 SlotIndex, APawn* UserPawn, const FAOAIInventoryItemQuery& Query)
{
	const FAOInventoryEntry* InventoryEntry = InventoryComponent.GetInventoryEntryAtSlot(SlotIndex);
	if (InventoryEntry == nullptr || InventoryEntry->Instance == nullptr || InventoryEntry->StackCount <= 0)
	{
		return false;
	}

	UAOInventoryItemInstance* ItemInstance = InventoryEntry->Instance;
	if (Query.RequiredItemInstanceClass != nullptr && !ItemInstance->IsA(Query.RequiredItemInstanceClass))
	{
		return false;
	}

	const UAOInventoryItemDefinition* ItemDefinition = ItemInstance->GetItemCDO();
	if (ItemDefinition == nullptr)
	{
		return false;
	}

	if (Query.RequiredItemDefinitionClass != nullptr && !ItemDefinition->IsA(Query.RequiredItemDefinitionClass))
	{
		return false;
	}

	if (Query.SemanticTag.IsValid() && !ItemDefinition->HasSemanticTag(Query.SemanticTag))
	{
		return false;
	}

	for (const TSubclassOf<UAOInventoryItemFragment>& RequiredFragmentClass : Query.RequiredFragmentClasses)
	{
		if (RequiredFragmentClass == nullptr)
		{
			continue;
		}

		if (ItemDefinition->FindFragmentByClass(RequiredFragmentClass) == nullptr)
		{
			return false;
		}
	}

	if (Query.bRequireUsableFromInventory && !InventoryComponent.CanUseItemAtSlot(SlotIndex, UserPawn))
	{
		return false;
	}

	return true;
}
