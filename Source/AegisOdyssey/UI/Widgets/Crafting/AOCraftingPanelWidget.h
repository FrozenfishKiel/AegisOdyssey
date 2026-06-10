// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AegisOdyssey/UI/Widgets/Crafting/AOCraftingWidgetBase.h"
#include "AOCraftingPanelWidget.generated.h"

class UAOCraftingRecipeDetailWidget;
class UAOCraftingRecipeListWidget;

// 制造界面的根面板控件。
// 它只负责暴露正式子区域引用，不再手动级联驱动子 Widget 刷新。
UCLASS(Abstract, Blueprintable)
class AEGISODYSSEY_API UAOCraftingPanelWidget : public UAOCraftingWidgetBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "AO|Crafting UI")
	UAOCraftingRecipeListWidget* GetRecipeListWidget() const { return RecipeListWidget; }

	UFUNCTION(BlueprintPure, Category = "AO|Crafting UI")
	UAOCraftingRecipeDetailWidget* GetRecipeDetailWidget() const { return RecipeDetailWidget; }

protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UAOCraftingRecipeListWidget> RecipeListWidget = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UAOCraftingRecipeDetailWidget> RecipeDetailWidget = nullptr;
};
