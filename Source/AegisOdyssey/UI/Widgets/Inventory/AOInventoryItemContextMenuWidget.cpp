#include "AOInventoryItemContextMenuWidget.h"

#include "AegisOdyssey/UI/ViewModel/Inventory/MVVM_InventoryItemContextAction.h"
#include "AegisOdyssey/UI/ViewModel/Inventory/MVVM_InventoryItemContextMenu.h"
#include "AegisOdyssey/UI/Widgets/Inventory/AOInventoryItemContextActionWidget.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/PanelWidget.h"
#include "Components/TextBlock.h"
#include "Input/Events.h"
#include "InputCoreTypes.h"
#include "ModelViewViewModel/Public/MVVMSubsystem.h"
#include "ModelViewViewModel/Public/View/MVVMView.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOInventoryItemContextMenuWidget)

const FName UAOInventoryItemContextMenuWidget::ContextMenuViewModelName(TEXT("InventoryItemContextMenu"));

UAOInventoryItemContextMenuWidget::UAOInventoryItemContextMenuWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsFocusable(true);
}

void UAOInventoryItemContextMenuWidget::InitializeForInventorySlot()
{
	RefreshDisplay();
}

void UAOInventoryItemContextMenuWidget::SetContextMenuViewModel(UMVVM_InventoryItemContextMenu* InViewModel)
{
	ContextMenuViewModel = InViewModel;
	OnContextMenuViewModelSet(ContextMenuViewModel);
	BindViewModelToWidget();
}

void UAOInventoryItemContextMenuWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (CloseButton)
	{
		CloseButton->OnClicked.AddDynamic(this, &ThisClass::HandleCloseButtonClicked);
	}

	BindViewModelToWidget();
	RefreshDisplay();
}

FReply UAOInventoryItemContextMenuWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::Escape)
	{
		CloseMenu();
		return FReply::Handled();
	}

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

FReply UAOInventoryItemContextMenuWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton || InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
	{
		const FGeometry& BorderGeometry = RootBorder ? RootBorder->GetCachedGeometry() : InGeometry;
		if (RootBorder && !BorderGeometry.IsUnderLocation(InMouseEvent.GetScreenSpacePosition()))
		{
			CloseMenu();
			return FReply::Handled();
		}
	}

	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void UAOInventoryItemContextMenuWidget::HandleActionWidgetInvoked(UAOInventoryItemContextActionWidget* ActionWidget)
{
	if (ActionWidget == nullptr)
	{
		return;
	}

	UMVVM_InventoryItemContextAction* ActionViewModel = ActionWidget->GetActionViewModel();
	if (ActionViewModel == nullptr || ContextMenuViewModel == nullptr)
	{
		return;
	}

	ActionViewModel->ExecuteAction();

	if (!ContextMenuViewModel->IsMenuVisible())
	{
		RemoveFromParent();
	}
}

void UAOInventoryItemContextMenuWidget::HandleCloseButtonClicked()
{
	CloseMenu();
}

void UAOInventoryItemContextMenuWidget::RefreshDisplay()
{
	if (ContextMenuViewModel == nullptr)
	{
		SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	SetVisibility(ContextMenuViewModel->IsMenuVisible() ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	RefreshHeaderFromViewModel();
	RefreshActionEntries();

	// 先把头部和动作区都刷新到当前快照，再做一次预排版，
	// 这样定位时拿到的 DesiredSize 才是本次菜单真正的尺寸。
	ForceLayoutPrepass();

	if (RootBorder)
	{
		RootBorder->SetRenderTranslation(ResolveMenuCanvasPosition());
	}
	else
	{
		SetPositionInViewport(ResolveMenuCanvasPosition(), false);
	}
}

void UAOInventoryItemContextMenuWidget::RefreshActionEntries()
{
	if (ActionListContainer == nullptr || ContextMenuViewModel == nullptr)
	{
		return;
	}

	const TArray<UMVVM_InventoryItemContextAction*> ActionViewModels = ContextMenuViewModel->GetActionViewModels();
	EnsureActionEntryPoolSize(ActionViewModels.Num());

	for (int32 ActionIndex = 0; ActionIndex < ActionEntryWidgets.Num(); ++ActionIndex)
	{
		UAOInventoryItemContextActionWidget* ActionEntryWidget = ActionEntryWidgets[ActionIndex];
		if (ActionEntryWidget == nullptr)
		{
			continue;
		}

		if (!ActionViewModels.IsValidIndex(ActionIndex) || ActionViewModels[ActionIndex] == nullptr)
		{
			ActionEntryWidget->SetActionViewModel(nullptr);
			ActionEntryWidget->SetVisibility(ESlateVisibility::Collapsed);
			continue;
		}

		ActionEntryWidget->SetActionViewModel(ActionViewModels[ActionIndex]);
		ActionEntryWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
}

void UAOInventoryItemContextMenuWidget::CloseMenu()
{
	if (ContextMenuViewModel)
	{
		ContextMenuViewModel->CloseMenu();
	}

	RemoveFromParent();
}

void UAOInventoryItemContextMenuWidget::BindViewModelToWidget()
{
	if (ContextMenuViewModel == nullptr)
	{
		return;
	}

	if (UMVVMView* View = UMVVMSubsystem::GetViewFromUserWidget(this))
	{
		TScriptInterface<INotifyFieldValueChanged> ViewModelInterface(ContextMenuViewModel);
		View->SetViewModel(ContextMenuViewModelName, ViewModelInterface);
	}

	// 主菜单实例会被复用，因此这里只 SetViewModel 还不够，
	// 还需要把当前整份快照主动重播给已经存在的头部绑定和动作项绑定。
	ContextMenuViewModel->BroadcastCurrentSnapshot();
}

void UAOInventoryItemContextMenuWidget::RefreshHeaderFromViewModel()
{
	if (ContextMenuViewModel == nullptr)
	{
		return;
	}

	if (ItemNameText)
	{
		ItemNameText->SetText(ContextMenuViewModel->GetItemDisplayName());
	}

	if (ItemInfoText)
	{
		ItemInfoText->SetText(ContextMenuViewModel->GetItemInfoText());
	}

	if (ItemIconImage)
	{
		ItemIconImage->SetBrush(ContextMenuViewModel->GetItemIconBrush());
		ItemIconImage->SetVisibility(
			ContextMenuViewModel->HasValidItemIcon() ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	}
}

void UAOInventoryItemContextMenuWidget::EnsureActionEntryPoolSize(int32 RequiredEntryCount)
{
	if (ActionListContainer == nullptr)
	{
		return;
	}

	// 动作项蓝图类必须由宿主蓝图显式配置。
	// 这里不再做默认回退，避免路径或接线错误时被隐式兜底掩盖掉。
	if (ActionEntryWidgetClass == nullptr)
	{
		return;
	}

	while (ActionEntryWidgets.Num() < RequiredEntryCount)
	{
		UAOInventoryItemContextActionWidget* ActionEntryWidget =
			CreateWidget<UAOInventoryItemContextActionWidget>(GetOwningPlayer(), ActionEntryWidgetClass);
		if (ActionEntryWidget == nullptr)
		{
			return;
		}

		ActionEntryWidget->OnInvoked.AddDynamic(this, &ThisClass::HandleActionWidgetInvoked);
		ActionListContainer->AddChild(ActionEntryWidget);
		ActionEntryWidgets.Add(ActionEntryWidget);
	}
}

FVector2D UAOInventoryItemContextMenuWidget::ResolveMenuCanvasPosition() const
{
	if (ContextMenuViewModel == nullptr || GetOwningPlayer() == nullptr)
	{
		return FVector2D::ZeroVector;
	}

	const FGeometry PlayerScreenGeometry = UWidgetLayoutLibrary::GetPlayerScreenWidgetGeometry(GetOwningPlayer());
	const FVector2D ScreenPosition = ContextMenuViewModel->GetScreenSpacePosition();
	return PlayerScreenGeometry.AbsoluteToLocal(ScreenPosition);
}
