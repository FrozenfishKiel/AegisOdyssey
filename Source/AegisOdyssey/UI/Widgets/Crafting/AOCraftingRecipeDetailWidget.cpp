// Fill out your copyright notice in the Description page of Project Settings.

#include "AOCraftingRecipeDetailWidget.h"

#include "AegisOdyssey/Inventory/AOInventoryItemDefinition.h"
#include "AegisOdyssey/UI/ViewModel/MVVM_Crafting.h"
#include "AegisOdyssey/UI/Widgets/Crafting/AOCraftingRecipeListWidget.h"
#include "Components/Button.h"
#include "Components/PanelWidget.h"
#include "Components/TextBlock.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOCraftingRecipeDetailWidget)

namespace AOCraftingRecipeDetailWidgetPrivate
{
	FText BuildDurationText(float InDurationSeconds)
	{
		return FText::Format(
			FText::FromString(TEXT("{0}s")),
			FText::AsNumber(FMath::CeilToInt(FMath::Max(0.0f, InDurationSeconds))));
	}

	FText ResolveDisplayName(const UAOInventoryItemDefinition* ItemDefinition, FName FallbackName)
	{
		if (ItemDefinition != nullptr && !ItemDefinition->DisplayName.IsNone())
		{
			return FText::FromName(ItemDefinition->DisplayName);
		}

		return FallbackName.IsNone() ? FText::GetEmpty() : FText::FromName(FallbackName);
	}
}

void UAOCraftingRecipeDetailWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (EnqueueButton != nullptr)
	{
		EnqueueButton->OnClicked.AddDynamic(this, &ThisClass::HandleEnqueueButtonClicked);
	}

	RefreshDetailHeader();
	RebuildMaterialEntryWidgets();
	RebuildOutputEntryWidgets();
}

bool UAOCraftingRecipeDetailWidget::RequestEnqueueSelectedRecipe()
{
	if (UMVVM_Crafting* CraftingViewModel = GetCraftingViewModel())
	{
		// 详情区按钮不直接找底层组件，只通过 ViewModel 发命令。
		return CraftingViewModel->RequestEnqueueSelectedRecipe();
	}

	return false;
}

void UAOCraftingRecipeDetailWidget::HandleCraftingViewModelChanged()
{
	// 详情区每次都整包消费 ViewModel 快照，
	// 不在 Widget 层单独维护标题区、材料区、产出区的分裂状态。
	CachedRecipeDetail = FAOCraftingRecipeDetailViewData();
	CachedBlockReasonText = FText::GetEmpty();
	bCachedCanEnqueueSelectedRecipe = false;

	if (const UMVVM_Crafting* CraftingViewModel = GetCraftingViewModel())
	{
		CachedRecipeDetail = CraftingViewModel->GetSelectedRecipeDetail();
		CachedBlockReasonText = CraftingViewModel->HasCraftRequestFeedback()
			? CraftingViewModel->GetLastCraftRequestFeedback()
			: CraftingViewModel->GetSelectedRecipeBlockReasonText();
		bCachedCanEnqueueSelectedRecipe = CraftingViewModel->CanEnqueueSelectedRecipe();
	}

	RefreshDetailHeader();
	RebuildMaterialEntryWidgets();
	RebuildOutputEntryWidgets();
}

void UAOCraftingRecipeDetailWidget::HandleEnqueueButtonClicked()
{
	// 按钮点击后复用统一请求入口，避免按钮逻辑散落在蓝图里。
	RequestEnqueueSelectedRecipe();
}

void UAOCraftingRecipeDetailWidget::RefreshDetailHeader()
{
	// 标题区只消费 CachedRecipeDetail / CachedBlockReasonText 这两个本地快照，
	// 不直接访问 CraftingComponent，保证详情区取数路径和列表保持一致。
	if (RecipeNameText != nullptr)
	{
		RecipeNameText->SetText(AOCraftingRecipeDetailWidgetPrivate::ResolveDisplayName(
			CachedRecipeDetail.PrimaryOutputDefinition,
			CachedRecipeDetail.RecipeRowName));
	}

	if (RecipeDurationText != nullptr)
	{
		RecipeDurationText->SetText(AOCraftingRecipeDetailWidgetPrivate::BuildDurationText(CachedRecipeDetail.ResolvedDurationSeconds));
	}

	if (RecipeBlockReasonText != nullptr)
	{
		RecipeBlockReasonText->SetText(CachedBlockReasonText);
	}

	if (EnqueueButton != nullptr)
	{
		EnqueueButton->SetIsEnabled(bCachedCanEnqueueSelectedRecipe);
	}
}

void UAOCraftingRecipeDetailWidget::RebuildMaterialEntryWidgets()
{
	if (MaterialListContainer == nullptr)
	{
		return;
	}

	// 材料区按当前详情快照整包重建，
	// 一个 MaterialEntry 对应一个显示格子，不在这里做额外合并。
	MaterialListContainer->ClearChildren();
	MaterialEntryWidgets.Reset();

	if (MaterialEntryWidgetClass == nullptr)
	{
		return;
	}

	for (const FAOCraftingMaterialViewData& MaterialEntry : CachedRecipeDetail.MaterialEntries)
	{
		UAOCraftingRecipeListEntryWidget* MaterialEntryWidget =
			CreateWidget<UAOCraftingRecipeListEntryWidget>(GetOwningPlayer(), MaterialEntryWidgetClass);
		if (MaterialEntryWidget == nullptr)
		{
			continue;
		}

		MaterialEntryWidget->SetMaterialData(MaterialEntry);
		MaterialListContainer->AddChild(MaterialEntryWidget);
		MaterialEntryWidgets.Add(MaterialEntryWidget);
	}
}

void UAOCraftingRecipeDetailWidget::RebuildOutputEntryWidgets()
{
	if (OutputListContainer == nullptr)
	{
		return;
	}

	// 产出区也按当前详情快照整包重建，保持和材料区一致的消费方式。
	OutputListContainer->ClearChildren();
	OutputEntryWidgets.Reset();

	if (OutputEntryWidgetClass == nullptr)
	{
		return;
	}

	for (const FAOCraftingOutputViewData& OutputEntry : CachedRecipeDetail.OutputEntries)
	{
		UAOCraftingRecipeListEntryWidget* OutputEntryWidget =
			CreateWidget<UAOCraftingRecipeListEntryWidget>(GetOwningPlayer(), OutputEntryWidgetClass);
		if (OutputEntryWidget == nullptr)
		{
			continue;
		}

		OutputEntryWidget->SetOutputData(OutputEntry);
		OutputListContainer->AddChild(OutputEntryWidget);
		OutputEntryWidgets.Add(OutputEntryWidget);
	}
}
