// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AegisOdyssey/Character/Enemies/AI/AOAIInventoryUseTypes.h"
#include "AOAIInventoryRuntimeUseLibrary.generated.h"

class APawn;
class UAOInventoryComponent;

UCLASS()
class AEGISODYSSEY_API UAOAIInventoryRuntimeUseLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	static bool TryResolveUseCommand(APawn* UserPawn, const FAOAIInventoryUseCommand& Command, FAOAIResolvedInventoryUseTarget& OutResolvedTarget);
	static bool TryExecuteResolvedTarget(APawn* UserPawn, const FAOAIResolvedInventoryUseTarget& ResolvedTarget, FAOAIInventoryUseExecutionResult& OutExecutionResult);
	static bool TryExecuteUseCommand(APawn* UserPawn, const FAOAIInventoryUseCommand& Command, FAOAIInventoryUseExecutionResult& OutExecutionResult);
	static int32 CountMatchingInventoryEntries(APawn* UserPawn, const FAOAIInventoryUseCommand& Command, bool bOnlyCountUsableEntries);
	static void GatherMatchingInventoryTargets(APawn* UserPawn, const FAOAIInventoryUseCommand& Command, bool bOnlyUsableEntries, TArray<FAOAIResolvedInventoryUseTarget>& OutResolvedTargets);

private:
	static void CollectOwnedInventoryComponents(APawn* UserPawn, const TArray<TSubclassOf<UAOInventoryComponent>>& AllowedInventoryComponentClasses, TArray<UAOInventoryComponent*>& OutInventoryComponents);
	static bool DoesInventoryComponentMatchScope(const UAOInventoryComponent& InventoryComponent, const TArray<TSubclassOf<UAOInventoryComponent>>& AllowedInventoryComponentClasses);
	static void GatherQuickBarCandidateSlotIndices(const FAOAIInventoryUseCommand& Command, TArray<int32>& OutSlotIndices);
	static bool TryResolveInventorySearchCommand(APawn* UserPawn, const FAOAIInventoryUseCommand& Command, FAOAIResolvedInventoryUseTarget& OutResolvedTarget);
	static bool TryResolveQuickBarCommand(APawn* UserPawn, const FAOAIInventoryUseCommand& Command, FAOAIResolvedInventoryUseTarget& OutResolvedTarget);
	static bool DoesResolvedTargetMatchCommand(const FAOAIResolvedInventoryUseTarget& ResolvedTarget, const FAOAIInventoryUseCommand& Command);
	static bool DoesInventoryEntryMatchQuery(UAOInventoryComponent& InventoryComponent, int32 SlotIndex, APawn* UserPawn, const FAOAIInventoryItemQuery& Query);
};
