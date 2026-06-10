// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AegisOdyssey/Crafting/Data/AOCraftingObservationTypes.h"
#include "AegisOdyssey/UI/Widgets/Crafting/AOCraftingWidgetBase.h"
#include "AOCraftingRecipeDetailWidget.generated.h"

class UButton;
class UPanelWidget;
class UTextBlock;
class UAOCraftingRecipeListEntryWidget;

// 配方详情容器负责标题区、阻断原因、制造按钮，以及材料/产出列表重建。
UCLASS(Abstract, Blueprintable)
class AEGISODYSSEY_API UAOCraftingRecipeDetailWidget : public UAOCraftingWidgetBase
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;

	// 当前详情区正在消费的配方详情快照。
	UFUNCTION(BlueprintPure, Category = "AO|Crafting UI")
	const FAOCraftingRecipeDetailViewData& GetRecipeDetail() const { return CachedRecipeDetail; }

	// 当前详情区要显示给玩家的阻断原因文案。
	UFUNCTION(BlueprintPure, Category = "AO|Crafting UI")
	FText GetBlockReasonText() const { return CachedBlockReasonText; }

	// 当前选中配方是否允许制造/入队。
	UFUNCTION(BlueprintPure, Category = "AO|Crafting UI")
	bool CanEnqueueSelectedRecipe() const { return bCachedCanEnqueueSelectedRecipe; }

	// 详情区制造按钮的统一请求入口。
	UFUNCTION(BlueprintCallable, Category = "AO|Crafting UI")
	bool RequestEnqueueSelectedRecipe();

protected:
	virtual void HandleCraftingViewModelChanged() override;

private:
	UFUNCTION()
	void HandleEnqueueButtonClicked();

	// 刷新标题区、时长、阻断原因和按钮状态。
	void RefreshDetailHeader();
	// 依据当前 CachedRecipeDetail.MaterialEntries 重建材料列表。
	void RebuildMaterialEntryWidgets();
	// 依据当前 CachedRecipeDetail.OutputEntries 重建产出列表。
	void RebuildOutputEntryWidgets();

protected:
	// 当前选中配方主标题。
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> RecipeNameText = nullptr;

	// 当前选中配方的制造时长文本。
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> RecipeDurationText = nullptr;

	// 当前选中配方的阻断原因文本。
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> RecipeBlockReasonText = nullptr;

	// 当前选中配方的制造按钮。
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	TObjectPtr<UButton> EnqueueButton = nullptr;

	// 承载材料条目的容器。
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	TObjectPtr<UPanelWidget> MaterialListContainer = nullptr;

	// 承载产出条目的容器。
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	TObjectPtr<UPanelWidget> OutputListContainer = nullptr;

	// 材料条目实际使用的 Widget 类。
	UPROPERTY(EditDefaultsOnly, Category = "AO|Crafting UI")
	TSubclassOf<UAOCraftingRecipeListEntryWidget> MaterialEntryWidgetClass;

	// 产出条目实际使用的 Widget 类。
	UPROPERTY(EditDefaultsOnly, Category = "AO|Crafting UI")
	TSubclassOf<UAOCraftingRecipeListEntryWidget> OutputEntryWidgetClass;

private:
	// 最近一次从 ViewModel 拉下来的详情快照。
	UPROPERTY(BlueprintReadOnly, Category = "AO|Crafting UI", meta = (AllowPrivateAccess = true))
	FAOCraftingRecipeDetailViewData CachedRecipeDetail;

	// 最近一次从 ViewModel 拉下来的阻断原因文案。
	UPROPERTY(BlueprintReadOnly, Category = "AO|Crafting UI", meta = (AllowPrivateAccess = true))
	FText CachedBlockReasonText;

	// 最近一次从 ViewModel 拉下来的“是否可制造”判断。
	UPROPERTY(BlueprintReadOnly, Category = "AO|Crafting UI", meta = (AllowPrivateAccess = true))
	bool bCachedCanEnqueueSelectedRecipe = false;

	// 当前已经实例化出来的材料条目，便于统一重建和清理。
	UPROPERTY(Transient)
	TArray<TObjectPtr<UAOCraftingRecipeListEntryWidget>> MaterialEntryWidgets;

	// 当前已经实例化出来的产出条目，便于统一重建和清理。
	UPROPERTY(Transient)
	TArray<TObjectPtr<UAOCraftingRecipeListEntryWidget>> OutputEntryWidgets;
};
