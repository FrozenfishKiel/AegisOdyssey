// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "AOAIInventoryUseTypes.generated.h"

class UAOInventoryComponent;
class UAOInventoryItemDefinition;
class UAOInventoryItemFragment;
class UAOInventoryItemInstance;

UENUM(BlueprintType)
enum class EAOAIInventoryUseCommandType : uint8
{
	InventorySearch UMETA(DisplayName = "Inventory Search"),
	QuickBarSlot UMETA(DisplayName = "QuickBar Slot")
};

USTRUCT(BlueprintType)
struct FAOAIInventoryItemQuery
{
	GENERATED_BODY()

	// 这里是给后续决策层预留的高层语义位。
	// 首版执行层不解释这个字段，只负责把它沿着命令边界传下去。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	FGameplayTag SemanticTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	TSubclassOf<UAOInventoryItemInstance> RequiredItemInstanceClass = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	TSubclassOf<UAOInventoryItemDefinition> RequiredItemDefinitionClass = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	TArray<TSubclassOf<UAOInventoryItemFragment>> RequiredFragmentClasses;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	bool bRequireUsableFromInventory = true;
};

USTRUCT(BlueprintType)
struct FAOAIInventoryUseCommand
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	EAOAIInventoryUseCommandType CommandType = EAOAIInventoryUseCommandType::InventorySearch;

	// 决策层显式指定本次允许搜索哪些自身库存组件。
	// 对 InventorySearch 来说，这个范围必须由上层明确给出，执行层不替上层决定搜索对象。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	TArray<TSubclassOf<UAOInventoryComponent>> AllowedInventoryComponentClasses;

	// 只有在 InventorySearch 模式下，这组硬约束才会参与解析。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	FAOAIInventoryItemQuery ItemQuery;

	// 这是“像玩家按数字键一样操作 QuickBar”的等价运行时命令。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config", meta = (ClampMin = "0", UIMin = "0"))
	int32 QuickBarSlotIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	TArray<int32> QuickBarSlotIndices;
};

USTRUCT(BlueprintType)
struct FAOAIResolvedInventoryUseTarget
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
	TObjectPtr<UAOInventoryComponent> InventoryComponent = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
	TObjectPtr<UAOInventoryItemInstance> ItemInstance = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
	int32 SlotIndex = INDEX_NONE;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
	bool bUsedQuickBarSlot = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
	int32 QuickBarSlotIndex = INDEX_NONE;
};

USTRUCT(BlueprintType)
struct FAOAIInventoryUseExecutionResult
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
	bool bSucceeded = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
	FAOAIResolvedInventoryUseTarget ResolvedTarget;
};
