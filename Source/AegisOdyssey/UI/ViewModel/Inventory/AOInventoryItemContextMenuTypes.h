#pragma once

#include "CoreMinimal.h"
#include "AOInventoryItemContextMenuTypes.generated.h"

UENUM(BlueprintType)
enum class EAOInventoryItemActionType : uint8
{
	Use,
	Unequip,
	CraftOne,
	CraftTen,
	CraftAll,
	Close,
};

USTRUCT(BlueprintType)
struct FAOInventoryItemContextAction
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "AO|Inventory")
	EAOInventoryItemActionType ActionType = EAOInventoryItemActionType::Close;

	UPROPERTY(BlueprintReadOnly, Category = "AO|Inventory")
	FText Label;

	UPROPERTY(BlueprintReadOnly, Category = "AO|Inventory")
	bool bEnabled = true;

	UPROPERTY(BlueprintReadOnly, Category = "AO|Inventory")
	bool bCloseMenuAfterExecute = true;

	UPROPERTY(BlueprintReadOnly, Category = "AO|Inventory")
	int32 SortOrder = 0;
};
