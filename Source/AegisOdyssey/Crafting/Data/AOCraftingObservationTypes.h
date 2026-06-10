#pragma once

#include "CoreMinimal.h"
#include "AOCraftingObservationTypes.generated.h"

class UAOInventoryItemDefinition;

UENUM(BlueprintType)
enum class EAOCraftingRecipeBlockReason : uint8
{
	None,
	InvalidRecipe,
	Locked,
	MissingMaterials,
	QueueFull
};

UENUM(BlueprintType)
enum class EAOCraftingQueueEntryViewState : uint8
{
	Queued,
	Active
};

UENUM(BlueprintType)
enum class EAOCraftingRequestType : uint8
{
	Single,
	Ten,
	All
};

USTRUCT(BlueprintType)
struct FAOCraftingRequestResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Crafting")
	FName RecipeRowName = NAME_None;

	UPROPERTY(BlueprintReadOnly, Category = "Crafting")
	EAOCraftingRequestType RequestType = EAOCraftingRequestType::Single;

	UPROPERTY(BlueprintReadOnly, Category = "Crafting")
	bool bAccepted = false;

	UPROPERTY(BlueprintReadOnly, Category = "Crafting")
	int32 RequestedCraftCount = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Crafting")
	int32 ActualCraftCount = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Crafting")
	EAOCraftingRecipeBlockReason FailureReason = EAOCraftingRecipeBlockReason::None;

	UPROPERTY(BlueprintReadOnly, Category = "Crafting")
	FText FeedbackText;
};

USTRUCT(BlueprintType)
struct FAOCraftingMaterialViewData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Crafting")
	int32 ItemId = INDEX_NONE;

	// UI 直接拿 Definition 读名字、图标和其他物品信息，不再重复镜像显示字段。
	UPROPERTY(BlueprintReadOnly, Category = "Crafting")
	TObjectPtr<UAOInventoryItemDefinition> ItemDefinition = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Crafting")
	int32 RequiredCount = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Crafting")
	int32 OwnedCount = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Crafting")
	bool bSatisfied = false;
};

USTRUCT(BlueprintType)
struct FAOCraftingOutputViewData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Crafting")
	int32 ItemId = INDEX_NONE;

	UPROPERTY(BlueprintReadOnly, Category = "Crafting")
	TObjectPtr<UAOInventoryItemDefinition> ItemDefinition = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Crafting")
	int32 Count = 0;
};

USTRUCT(BlueprintType)
struct FAOCraftingRecipeListEntryViewData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Crafting")
	FName RecipeRowName = NAME_None;

	// 配方列表主显示统一依赖主产物 Definition。
	UPROPERTY(BlueprintReadOnly, Category = "Crafting")
	TObjectPtr<UAOInventoryItemDefinition> PrimaryOutputDefinition = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Crafting")
	float ResolvedDurationSeconds = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Crafting")
	bool bCanEnqueue = false;

	UPROPERTY(BlueprintReadOnly, Category = "Crafting")
	EAOCraftingRecipeBlockReason BlockReason = EAOCraftingRecipeBlockReason::InvalidRecipe;
};

USTRUCT(BlueprintType)
struct FAOCraftingRecipeDetailViewData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Crafting")
	FName RecipeRowName = NAME_None;

	UPROPERTY(BlueprintReadOnly, Category = "Crafting")
	TObjectPtr<UAOInventoryItemDefinition> PrimaryOutputDefinition = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Crafting")
	float ResolvedDurationSeconds = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Crafting")
	bool bCanEnqueue = false;

	UPROPERTY(BlueprintReadOnly, Category = "Crafting")
	EAOCraftingRecipeBlockReason BlockReason = EAOCraftingRecipeBlockReason::InvalidRecipe;

	UPROPERTY(BlueprintReadOnly, Category = "Crafting")
	TArray<FAOCraftingMaterialViewData> MaterialEntries;

	UPROPERTY(BlueprintReadOnly, Category = "Crafting")
	TArray<FAOCraftingOutputViewData> OutputEntries;
};

USTRUCT(BlueprintType)
struct FAOCraftingQueueEntryViewData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Crafting")
	int32 QueueEntryId = INDEX_NONE;

	UPROPERTY(BlueprintReadOnly, Category = "Crafting")
	FName RecipeRowName = NAME_None;

	UPROPERTY(BlueprintReadOnly, Category = "Crafting")
	TObjectPtr<UAOInventoryItemDefinition> PrimaryOutputDefinition = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Crafting")
	EAOCraftingRecipeBlockReason BlockReason = EAOCraftingRecipeBlockReason::InvalidRecipe;

	UPROPERTY(BlueprintReadOnly, Category = "Crafting")
	EAOCraftingQueueEntryViewState State = EAOCraftingQueueEntryViewState::Queued;

	UPROPERTY(BlueprintReadOnly, Category = "Crafting")
	float ResolvedDurationSeconds = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Crafting")
	float StartServerWorldTimeSeconds = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Crafting")
	float ExpectedFinishServerWorldTimeSeconds = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Crafting")
	int32 TotalCraftCount = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Crafting")
	int32 RemainingCraftCount = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Crafting")
	int32 CompletedCraftCount = 0;
};
