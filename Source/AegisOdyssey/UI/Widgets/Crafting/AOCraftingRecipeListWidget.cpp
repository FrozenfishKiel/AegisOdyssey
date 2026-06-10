// Fill out your copyright notice in the Description page of Project Settings.

#include "AOCraftingRecipeListWidget.h"

#include "AegisOdyssey/Crafting/Components/AOCraftingComponent.h"
#include "AegisOdyssey/Inventory/AOInventoryItemDefinition.h"
#include "AegisOdyssey/Inventory/Fragments/AOFragment_InventoryIcon.h"
#include "AegisOdyssey/UI/ViewModel/AOCombatFeedbackBlueprintLibrary.h"
#include "AegisOdyssey/UI/ViewModel/Inventory/AOInventoryItemContextMenuTypes.h"
#include "AegisOdyssey/UI/ViewModel/Inventory/MVVM_InventoryItemContextMenu.h"
#include "AegisOdyssey/UI/ViewModel/MVVM_Crafting.h"
#include "AegisOdyssey/UI/ViewModel/MVVM_ItemHoverTooltip.h"
#include "AegisOdyssey/UI/Widgets/Inventory/AOInventoryItemContextMenuWidget.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/PanelWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/Widget.h"
#include "Input/Events.h"
#include "Input/Reply.h"
#include "InputCoreTypes.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOCraftingRecipeListWidget)

void UAOCraftingRecipeListEntryWidget::SetEntryData(const FAOCraftingRecipeListEntryViewData& InEntryData, bool bInSelected)
{
	DisplayMode = EAOCraftingRecipeListEntryDisplayMode::Recipe;
	EntryData = InEntryData;
	bSelected = bInSelected;
	RefreshDisplay();
}

void UAOCraftingRecipeListEntryWidget::SetMaterialData(const FAOCraftingMaterialViewData& InMaterialData)
{
	DisplayMode = EAOCraftingRecipeListEntryDisplayMode::Material;
	MaterialData = InMaterialData;
	bSelected = false;
	RefreshDisplay();
}

void UAOCraftingRecipeListEntryWidget::SetOutputData(const FAOCraftingOutputViewData& InOutputData)
{
	DisplayMode = EAOCraftingRecipeListEntryDisplayMode::Output;
	OutputData = InOutputData;
	bSelected = false;
	RefreshDisplay();
}

void UAOCraftingRecipeListEntryWidget::SetQueueEntryData(
	const FAOCraftingQueueEntryViewData& InQueueEntryData,
	float InProgressPercent)
{
	DisplayMode = EAOCraftingRecipeListEntryDisplayMode::Queue;
	QueueEntryData = InQueueEntryData;
	QueueProgressPercent = FMath::Clamp(InProgressPercent, 0.0f, 1.0f);
	bSelected = false;
	RefreshDisplay();
}

void UAOCraftingRecipeListEntryWidget::SetQueueEmptyState()
{
	DisplayMode = EAOCraftingRecipeListEntryDisplayMode::QueueEmpty;
	QueueEntryData = FAOCraftingQueueEntryViewData();
	QueueProgressPercent = 0.0f;
	bSelected = false;
	RefreshDisplay();
}

void UAOCraftingRecipeListEntryWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (SelectButton != nullptr)
	{
		SelectButton->OnClicked.AddDynamic(this, &ThisClass::HandleSelectButtonClicked);
	}

	RefreshDisplay();
}

void UAOCraftingRecipeListEntryWidget::NativeDestruct()
{
	HideHoverTooltip();
	Super::NativeDestruct();
}

void UAOCraftingRecipeListEntryWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);
	ShowHoverTooltip(InGeometry);
}

void UAOCraftingRecipeListEntryWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	HideHoverTooltip();
	Super::NativeOnMouseLeave(InMouseEvent);
}

FReply UAOCraftingRecipeListEntryWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (DisplayMode == EAOCraftingRecipeListEntryDisplayMode::Recipe
		&& InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
	{
		OnContextRequested.Broadcast(EntryData.RecipeRowName, InMouseEvent.GetScreenSpacePosition());
		return FReply::Handled();
	}

	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void UAOCraftingRecipeListEntryWidget::HandleSelectButtonClicked()
{
	if (DisplayMode == EAOCraftingRecipeListEntryDisplayMode::Recipe)
	{
		OnSelected.Broadcast(EntryData.RecipeRowName);
	}
}

void UAOCraftingRecipeListEntryWidget::RefreshDisplay()
{
	switch (DisplayMode)
	{
	case EAOCraftingRecipeListEntryDisplayMode::Recipe:
		RefreshRecipeDisplay();
		break;
	case EAOCraftingRecipeListEntryDisplayMode::Material:
		RefreshMaterialDisplay();
		break;
	case EAOCraftingRecipeListEntryDisplayMode::Output:
		RefreshOutputDisplay();
		break;
	case EAOCraftingRecipeListEntryDisplayMode::Queue:
	case EAOCraftingRecipeListEntryDisplayMode::QueueEmpty:
		RefreshQueueDisplay();
		break;
	default:
		break;
	}
}

void UAOCraftingRecipeListEntryWidget::RefreshRecipeDisplay()
{
	if (RecipeIconImage != nullptr)
	{
		FSlateBrush ResolvedBrush;
		const bool bHasValidIcon = ResolveIcon(EntryData.PrimaryOutputDefinition, ResolvedBrush);
		RecipeIconImage->SetBrush(ResolvedBrush);
		RecipeIconImage->SetVisibility(bHasValidIcon ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	}

	if (RecipeNameText != nullptr)
	{
		RecipeNameText->SetText(ResolveDisplayName(EntryData.PrimaryOutputDefinition, EntryData.RecipeRowName));
	}

	if (DurationText != nullptr)
	{
		DurationText->SetText(BuildDurationText());
	}

	if (StateText != nullptr)
	{
		StateText->SetText(BuildStateText());
	}

	if (SelectedIndicator != nullptr)
	{
		SelectedIndicator->SetVisibility(bSelected ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	}
}

void UAOCraftingRecipeListEntryWidget::RefreshMaterialDisplay()
{
	if (ItemIconImage != nullptr)
	{
		FSlateBrush ResolvedBrush;
		const bool bHasValidIcon = ResolveIcon(MaterialData.ItemDefinition, ResolvedBrush);
		ItemIconImage->SetBrush(ResolvedBrush);
		ItemIconImage->SetVisibility(bHasValidIcon ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	}

	if (ItemNameText != nullptr)
	{
		ItemNameText->SetText(ResolveDisplayName(MaterialData.ItemDefinition, NAME_None));
	}

	if (CountText != nullptr)
	{
		CountText->SetText(BuildMaterialCountText());
	}

	if (StatusText != nullptr)
	{
		StatusText->SetText(MaterialData.bSatisfied ? FText::GetEmpty() : FText::FromString(TEXT("不足")));
	}

	if (SatisfiedIndicator != nullptr)
	{
		SatisfiedIndicator->SetVisibility(MaterialData.bSatisfied ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	}
}

void UAOCraftingRecipeListEntryWidget::RefreshOutputDisplay()
{
	if (ItemIconImage != nullptr)
	{
		FSlateBrush ResolvedBrush;
		const bool bHasValidIcon = ResolveIcon(OutputData.ItemDefinition, ResolvedBrush);
		ItemIconImage->SetBrush(ResolvedBrush);
		ItemIconImage->SetVisibility(bHasValidIcon ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	}

	if (ItemNameText != nullptr)
	{
		ItemNameText->SetText(ResolveDisplayName(OutputData.ItemDefinition, NAME_None));
	}

	if (CountText != nullptr)
	{
		CountText->SetText(BuildOutputCountText());
	}
}

void UAOCraftingRecipeListEntryWidget::RefreshQueueDisplay()
{
	const bool bIsQueueEntry = DisplayMode == EAOCraftingRecipeListEntryDisplayMode::Queue;

	if (RecipeIconImage != nullptr)
	{
		RecipeIconImage->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (RecipeNameText != nullptr)
	{
		RecipeNameText->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (DurationText != nullptr)
	{
		DurationText->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (StateText != nullptr)
	{
		StateText->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (SelectedIndicator != nullptr)
	{
		SelectedIndicator->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (QueueEmptyPlaceholder != nullptr)
	{
		QueueEmptyPlaceholder->SetVisibility(bIsQueueEntry ? ESlateVisibility::Collapsed : ESlateVisibility::SelfHitTestInvisible);
	}

	if (!bIsQueueEntry)
	{
		if (ItemIconImage != nullptr)
		{
			ItemIconImage->SetVisibility(ESlateVisibility::Collapsed);
		}

		if (ItemNameText != nullptr)
		{
			ItemNameText->SetText(FText::GetEmpty());
		}

		if (CountText != nullptr)
		{
			CountText->SetText(FText::GetEmpty());
		}

		if (StatusText != nullptr)
		{
			StatusText->SetText(FText::FromString(TEXT("\u7A7A\u95F2\u4E2D")));
			StatusText->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		}

		if (SatisfiedIndicator != nullptr)
		{
			SatisfiedIndicator->SetVisibility(ESlateVisibility::Collapsed);
		}

		if (ProgressBar != nullptr)
		{
			ProgressBar->SetPercent(0.0f);
			ProgressBar->SetVisibility(ESlateVisibility::Collapsed);
		}

		return;
	}

	if (ItemIconImage != nullptr)
	{
		FSlateBrush ResolvedBrush;
		const bool bHasValidIcon = ResolveIcon(QueueEntryData.PrimaryOutputDefinition, ResolvedBrush);
		ItemIconImage->SetBrush(ResolvedBrush);
		ItemIconImage->SetVisibility(bHasValidIcon ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	}

	if (ItemNameText != nullptr)
	{
		ItemNameText->SetText(ResolveDisplayName(QueueEntryData.PrimaryOutputDefinition, QueueEntryData.RecipeRowName));
	}

	if (CountText != nullptr)
	{
		CountText->SetText(BuildQueueCountText());
	}

	if (StatusText != nullptr)
	{
		StatusText->SetText(BuildQueueStatusText());
		StatusText->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}

	if (SatisfiedIndicator != nullptr)
	{
		SatisfiedIndicator->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (ProgressBar != nullptr)
	{
		ProgressBar->SetPercent(QueueProgressPercent);
		ProgressBar->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
}

FText UAOCraftingRecipeListEntryWidget::ResolveDisplayName(const UAOInventoryItemDefinition* ItemDefinition, FName FallbackName) const
{
	if (ItemDefinition != nullptr && !ItemDefinition->DisplayName.IsNone())
	{
		return FText::FromName(ItemDefinition->DisplayName);
	}

	return FallbackName.IsNone() ? FText::GetEmpty() : FText::FromName(FallbackName);
}

bool UAOCraftingRecipeListEntryWidget::ResolveIcon(const UAOInventoryItemDefinition* ItemDefinition, FSlateBrush& OutBrush) const
{
	OutBrush = FSlateBrush();

	if (ItemDefinition == nullptr)
	{
		return false;
	}

	const UAOFragment_InventoryIcon* IconFragment = ItemDefinition->FindFragmentByClass<UAOFragment_InventoryIcon>();
	if (IconFragment == nullptr)
	{
		return false;
	}

	OutBrush = IconFragment->Brush;
	return OutBrush.GetResourceObject() != nullptr;
}

FText UAOCraftingRecipeListEntryWidget::BuildDurationText() const
{
	return FText::Format(
		FText::FromString(TEXT("{0}s")),
		FText::AsNumber(FMath::CeilToInt(FMath::Max(0.0f, EntryData.ResolvedDurationSeconds))));
}

FText UAOCraftingRecipeListEntryWidget::BuildStateText() const
{
	switch (EntryData.BlockReason)
	{
	case EAOCraftingRecipeBlockReason::None:
		return FText::FromString(TEXT("可制作"));
	case EAOCraftingRecipeBlockReason::InvalidRecipe:
		return FText::FromString(TEXT("配方无效"));
	case EAOCraftingRecipeBlockReason::Locked:
		return FText::FromString(TEXT("尚未解锁"));
	case EAOCraftingRecipeBlockReason::MissingMaterials:
		return FText::FromString(TEXT("材料不足"));
	case EAOCraftingRecipeBlockReason::QueueFull:
		return FText::FromString(TEXT("制造队列已满"));
	default:
		return FText::GetEmpty();
	}
}

FText UAOCraftingRecipeListEntryWidget::BuildMaterialCountText() const
{
	return FText::Format(
		FText::FromString(TEXT("{0}/{1}")),
		FText::AsNumber(MaterialData.OwnedCount),
		FText::AsNumber(MaterialData.RequiredCount));
}

FText UAOCraftingRecipeListEntryWidget::BuildOutputCountText() const
{
	return FText::Format(
		FText::FromString(TEXT("x{0}")),
		FText::AsNumber(OutputData.Count));
}

FText UAOCraftingRecipeListEntryWidget::BuildQueueCountText() const
{
	return FText::Format(
		FText::FromString(TEXT("{0}/{1}")),
		FText::AsNumber(QueueEntryData.RemainingCraftCount),
		FText::AsNumber(QueueEntryData.TotalCraftCount));
}

FText UAOCraftingRecipeListEntryWidget::BuildQueueStatusText() const
{
	switch (QueueEntryData.State)
	{
	case EAOCraftingQueueEntryViewState::Active:
		return FText::FromString(TEXT("制作中"));
	case EAOCraftingQueueEntryViewState::Queued:
		return FText::FromString(TEXT("等待中"));
	default:
		return FText::GetEmpty();
	}
}

const UAOInventoryItemDefinition* UAOCraftingRecipeListEntryWidget::ResolveHoverTooltipItemDefinition() const
{
	// 制造条目的 Tooltip 真相继续来自当前观察快照，
	// 不在悬浮时额外回到底层 CraftingComponent 再查一遍数据。
	switch (DisplayMode)
	{
	case EAOCraftingRecipeListEntryDisplayMode::Recipe:
		return EntryData.PrimaryOutputDefinition;
	case EAOCraftingRecipeListEntryDisplayMode::Material:
		return MaterialData.ItemDefinition;
	case EAOCraftingRecipeListEntryDisplayMode::Output:
		return OutputData.ItemDefinition;
	case EAOCraftingRecipeListEntryDisplayMode::Queue:
		return QueueEntryData.PrimaryOutputDefinition;
	default:
		return nullptr;
	}
}

void UAOCraftingRecipeListEntryWidget::ShowHoverTooltip(const FGeometry& InGeometry)
{
	UMVVM_ItemHoverTooltip* TooltipViewModel = UAOCombatFeedbackBlueprintLibrary::GetItemHoverTooltipViewModel(this);
	if (TooltipViewModel == nullptr)
	{
		return;
	}

	const UAOInventoryItemDefinition* ItemDefinition = ResolveHoverTooltipItemDefinition();
	if (ItemDefinition == nullptr)
	{
		TooltipViewModel->HideTooltip(this);
		return;
	}

	// 当前方案与库存格子保持一致：进入时取一次条目附近的位置，显示后不跟随鼠标。
	const FVector2D ScreenSpacePosition = InGeometry.LocalToAbsolute(InGeometry.GetLocalSize() * 0.5f);
	TooltipViewModel->ShowTooltip(ItemDefinition, ScreenSpacePosition, this);
}

void UAOCraftingRecipeListEntryWidget::HideHoverTooltip()
{
	if (UMVVM_ItemHoverTooltip* TooltipViewModel = UAOCombatFeedbackBlueprintLibrary::GetItemHoverTooltipViewModel(this))
	{
		TooltipViewModel->HideTooltip(this);
	}
}

void UAOCraftingRecipeListWidget::SelectRecipe(FName InRecipeRowName)
{
	if (UMVVM_Crafting* CraftingViewModel = GetCraftingViewModel())
	{
		CraftingViewModel->SetSelectedRecipeRowName(InRecipeRowName);
	}
}

void UAOCraftingRecipeListWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	RefreshQueueProgressWidgets();
}

void UAOCraftingRecipeListWidget::HandleCraftingViewModelChanged()
{
	CachedRecipeList.Reset();
	CachedQueueList.Reset();
	QueueSlotCount = 0;
	SelectedRecipeRowName = NAME_None;

	if (const UMVVM_Crafting* CraftingViewModel = GetCraftingViewModel())
	{
		CachedRecipeList = CraftingViewModel->GetRecipeList();
		CachedQueueList = CraftingViewModel->GetQueueList();
		QueueSlotCount = CraftingViewModel->GetQueueSlotCount();
		SelectedRecipeRowName = CraftingViewModel->GetSelectedRecipeRowName();
	}

	RebuildRecipeEntryWidgets();
	RebuildQueueEntryWidgets();
	RefreshQueueProgressWidgets();
}

void UAOCraftingRecipeListWidget::RebuildRecipeEntryWidgets()
{
	if (RecipeListContainer == nullptr)
	{
		return;
	}

	RecipeListContainer->ClearChildren();
	RecipeEntryWidgets.Reset();

	if (RecipeEntryWidgetClass == nullptr)
	{
		return;
	}

	for (const FAOCraftingRecipeListEntryViewData& RecipeEntry : CachedRecipeList)
	{
		UAOCraftingRecipeListEntryWidget* RecipeEntryWidget =
			CreateWidget<UAOCraftingRecipeListEntryWidget>(GetOwningPlayer(), RecipeEntryWidgetClass);
		if (RecipeEntryWidget == nullptr)
		{
			continue;
		}

		RecipeEntryWidget->SetEntryData(RecipeEntry, RecipeEntry.RecipeRowName == SelectedRecipeRowName);
		RecipeEntryWidget->OnSelected.AddDynamic(this, &ThisClass::HandleRecipeEntrySelected);
		RecipeEntryWidget->OnContextRequested.AddDynamic(this, &ThisClass::HandleRecipeEntryContextRequested);
		RecipeListContainer->AddChild(RecipeEntryWidget);
		RecipeEntryWidgets.Add(RecipeEntryWidget);
	}
}

void UAOCraftingRecipeListWidget::RebuildQueueEntryWidgets()
{
	if (QueueListContainer == nullptr)
	{
		return;
	}

	QueueListContainer->ClearChildren();
	QueueEntryWidgets.Reset();

	if (QueueEntryWidgetClass == nullptr || QueueSlotCount <= 0)
	{
		return;
	}

	for (int32 QueueSlotIndex = 0; QueueSlotIndex < QueueSlotCount; ++QueueSlotIndex)
	{
		UAOCraftingRecipeListEntryWidget* QueueEntryWidget =
			CreateWidget<UAOCraftingRecipeListEntryWidget>(GetOwningPlayer(), QueueEntryWidgetClass);
		if (QueueEntryWidget == nullptr)
		{
			continue;
		}

		QueueListContainer->AddChild(QueueEntryWidget);
		QueueEntryWidgets.Add(QueueEntryWidget);
	}

	RefreshQueueEntryWidgets();
}

void UAOCraftingRecipeListWidget::RefreshQueueEntryWidgets()
{
	if (QueueEntryWidgets.IsEmpty())
	{
		return;
	}

	for (int32 QueueSlotIndex = 0; QueueSlotIndex < QueueEntryWidgets.Num(); ++QueueSlotIndex)
	{
		UAOCraftingRecipeListEntryWidget* QueueEntryWidget = QueueEntryWidgets[QueueSlotIndex];
		if (QueueEntryWidget == nullptr)
		{
			continue;
		}

		if (CachedQueueList.IsValidIndex(QueueSlotIndex))
		{
			const FAOCraftingQueueEntryViewData& QueueEntry = CachedQueueList[QueueSlotIndex];
			QueueEntryWidget->SetQueueEntryData(QueueEntry, ResolveQueueEntryProgressPercent(QueueEntry));
		}
		else
		{
			QueueEntryWidget->SetQueueEmptyState();
		}
	}
}

void UAOCraftingRecipeListWidget::RefreshQueueProgressWidgets()
{
	if (QueueEntryWidgets.IsEmpty())
	{
		return;
	}

	for (int32 QueueSlotIndex = 0; QueueSlotIndex < QueueEntryWidgets.Num(); ++QueueSlotIndex)
	{
		UAOCraftingRecipeListEntryWidget* QueueEntryWidget = QueueEntryWidgets[QueueSlotIndex];
		if (QueueEntryWidget == nullptr)
		{
			continue;
		}

		if (CachedQueueList.IsValidIndex(QueueSlotIndex))
		{
			const FAOCraftingQueueEntryViewData& QueueEntry = CachedQueueList[QueueSlotIndex];
			QueueEntryWidget->SetQueueEntryData(QueueEntry, ResolveQueueEntryProgressPercent(QueueEntry));
		}
	}
}

float UAOCraftingRecipeListWidget::ResolveQueueEntryProgressPercent(const FAOCraftingQueueEntryViewData& QueueEntry) const
{
	if (const UMVVM_Crafting* CraftingViewModel = GetCraftingViewModel())
	{
		return CraftingViewModel->GetQueueEntryProgressRatio(QueueEntry);
	}

	return 0.0f;
}

void UAOCraftingRecipeListWidget::HandleRecipeEntrySelected(FName InRecipeRowName)
{
	SelectRecipe(InRecipeRowName);
}

void UAOCraftingRecipeListWidget::EnsureContextMenuWidget()
{
	if (IsValid(ActiveContextMenuWidget) || ContextMenuWidgetClass == nullptr || GetOwningPlayer() == nullptr)
	{
		return;
	}

	ActiveContextMenuWidget = CreateWidget<UAOInventoryItemContextMenuWidget>(GetOwningPlayer(), ContextMenuWidgetClass);
}

bool UAOCraftingRecipeListWidget::BuildCraftingContextMenuActions(TArray<FAOInventoryItemContextAction>& OutActions) const
{
	OutActions.Reset();

	FAOInventoryItemContextAction& CraftOneAction = OutActions.AddDefaulted_GetRef();
	CraftOneAction.ActionType = EAOInventoryItemActionType::CraftOne;
	CraftOneAction.Label = FText::FromString(TEXT("制作一个"));
	CraftOneAction.bEnabled = true;
	CraftOneAction.bCloseMenuAfterExecute = true;
	CraftOneAction.SortOrder = 0;

	FAOInventoryItemContextAction& CraftTenAction = OutActions.AddDefaulted_GetRef();
	CraftTenAction.ActionType = EAOInventoryItemActionType::CraftTen;
	CraftTenAction.Label = FText::FromString(TEXT("制作十个"));
	CraftTenAction.bEnabled = true;
	CraftTenAction.bCloseMenuAfterExecute = true;
	CraftTenAction.SortOrder = 1;

	FAOInventoryItemContextAction& CraftAllAction = OutActions.AddDefaulted_GetRef();
	CraftAllAction.ActionType = EAOInventoryItemActionType::CraftAll;
	CraftAllAction.Label = FText::FromString(TEXT("制作全部"));
	CraftAllAction.bEnabled = true;
	CraftAllAction.bCloseMenuAfterExecute = true;
	CraftAllAction.SortOrder = 2;

	return !OutActions.IsEmpty();
}

bool UAOCraftingRecipeListWidget::ResolveRecipeContextVisuals(
	FName InRecipeRowName,
	FText& OutDisplayName,
	FSlateBrush& OutIconBrush,
	bool& bOutHasValidIcon) const
{
	OutDisplayName = FText::FromName(InRecipeRowName);
	OutIconBrush = FSlateBrush();
	bOutHasValidIcon = false;

	const UMVVM_Crafting* CraftingViewModel = GetCraftingViewModel();
	if (CraftingViewModel == nullptr)
	{
		return false;
	}

	const FAOCraftingRecipeDetailViewData& RecipeDetail = CraftingViewModel->GetSelectedRecipeDetail();
	if (RecipeDetail.RecipeRowName != InRecipeRowName)
	{
		return false;
	}

	const UAOInventoryItemDefinition* ItemDefinition = RecipeDetail.PrimaryOutputDefinition;
	if (ItemDefinition != nullptr && !ItemDefinition->DisplayName.IsNone())
	{
		OutDisplayName = FText::FromName(ItemDefinition->DisplayName);
	}

	if (ItemDefinition != nullptr)
	{
		if (const UAOFragment_InventoryIcon* IconFragment = ItemDefinition->FindFragmentByClass<UAOFragment_InventoryIcon>())
		{
			OutIconBrush = IconFragment->Brush;
			bOutHasValidIcon = OutIconBrush.GetResourceObject() != nullptr;
		}
	}

	return true;
}

void UAOCraftingRecipeListWidget::HandleRecipeEntryContextRequested(FName InRecipeRowName, FVector2D ScreenSpacePosition)
{
	SelectRecipe(InRecipeRowName);

	UMVVM_Crafting* CraftingViewModel = GetCraftingViewModel();
	if (CraftingViewModel == nullptr)
	{
		return;
	}

	const FAOCraftingRecipeDetailViewData& RecipeDetail = CraftingViewModel->GetSelectedRecipeDetail();
	if (RecipeDetail.RecipeRowName != InRecipeRowName)
	{
		return;
	}

	UAOCraftingComponent* CraftingComponent = CraftingViewModel->GetObservedCraftingComponent();
	if (CraftingComponent == nullptr)
	{
		return;
	}

	TArray<FAOInventoryItemContextAction> MenuActions;
	if (!BuildCraftingContextMenuActions(MenuActions))
	{
		return;
	}

	FText DisplayName;
	FSlateBrush IconBrush;
	bool bHasValidIcon = false;
	if (!ResolveRecipeContextVisuals(InRecipeRowName, DisplayName, IconBrush, bHasValidIcon))
	{
		return;
	}

	EnsureContextMenuWidget();
	if (!IsValid(ActiveContextMenuWidget))
	{
		return;
	}

	UMVVM_InventoryItemContextMenu* ContextMenuViewModel = CraftingComponent->GetOrCreateCraftingContextMenuViewModel();
	if (ContextMenuViewModel == nullptr)
	{
		return;
	}

	ContextMenuViewModel->SetRequestingInventoryUI(nullptr);
	ContextMenuViewModel->SetRequestingCraftingViewModel(CraftingViewModel);
	ContextMenuViewModel->OpenForCraftingRecipe(
		InRecipeRowName,
		DisplayName,
		IconBrush,
		bHasValidIcon,
		FText::GetEmpty(),
		ScreenSpacePosition,
		MenuActions);

	ActiveContextMenuWidget->SetContextMenuViewModel(ContextMenuViewModel);
	ActiveContextMenuWidget->InitializeForInventorySlot();

	if (!ActiveContextMenuWidget->IsInViewport())
	{
		ActiveContextMenuWidget->AddToViewport(1000);
	}

	ActiveContextMenuWidget->SetKeyboardFocus();
}
