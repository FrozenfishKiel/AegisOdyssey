#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "AOCraftingRecipeTypes.generated.h"

class UAOInventoryItemDefinition;

USTRUCT(BlueprintType)
struct FAOCraftingItemCount
{
	GENERATED_BODY()

	// 统一物品 ID，制造系统通过它回查物品总表。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Crafting")
	int32 ItemId = INDEX_NONE;

	// 该物品在当前材料项或产物项里的数量。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Crafting")
	int32 Count = 0;
};

USTRUCT(BlueprintType)
struct FAOCraftingRecipeRow : public FTableRowBase
{
	GENERATED_BODY()

	// 角色达到该等级后，这条配方才允许真正入队。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Crafting")
	int32 UnlockLevel = 1;

	// 未达到解锁等级时，这条配方是否仍然显示在制造列表里。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Crafting")
	bool bVisibleBeforeUnlock = true;

	// 制造列表排序权重，数值越小越靠前。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Crafting")
	int32 SortOrder = 0;

	// 这条配方在 UI 中显示给玩家看的名字。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Crafting")
	FText DisplayName;

	// 不带任何角色制造速度加成时的基础制造时长，单位秒。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Crafting")
	float BaseCraftDurationSeconds = 0.0f;

	// 入队时需要立刻扣除的材料列表。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Crafting")
	TArray<FAOCraftingItemCount> MaterialEntries;

	// 制造完成后统一发放的产物列表。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Crafting")
	TArray<FAOCraftingItemCount> OutputEntries;
};

USTRUCT(BlueprintType)
struct FAOCraftingResolvedItemEntry
{
	GENERATED_BODY()

	// 从配方表解析出来的统一物品 ID。
	UPROPERTY(BlueprintReadOnly, Category = "Crafting")
	int32 ItemId = INDEX_NONE;

	// 当前解析出的实际数量。
	UPROPERTY(BlueprintReadOnly, Category = "Crafting")
	int32 Count = 0;

	// 通过物品总表把 ItemId 解析到真实物品定义后的结果。
	UPROPERTY(BlueprintReadOnly, Category = "Crafting")
	TSubclassOf<UAOInventoryItemDefinition> ItemDefinitionClass = nullptr;
};
