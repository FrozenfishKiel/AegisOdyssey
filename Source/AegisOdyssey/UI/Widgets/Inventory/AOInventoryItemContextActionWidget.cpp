#include "AOInventoryItemContextActionWidget.h"

#include "AegisOdyssey/UI/ViewModel/Inventory/MVVM_InventoryItemContextAction.h"
#include "Components/Button.h"
#include "ModelViewViewModel/Public/MVVMSubsystem.h"
#include "ModelViewViewModel/Public/View/MVVMView.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOInventoryItemContextActionWidget)

// 必须和动作项蓝图里登记的 MVVM ViewModel 名完全一致。
// 右键菜单动作按钮蓝图当前使用的就是 "MVVM_InventoryItemContextAction"，
// 这里一旦不一致，运行时就会直接报 "viewmodel name could not be found"。
const FName UAOInventoryItemContextActionWidget::ActionViewModelName(TEXT("MVVM_InventoryItemContextAction"));

void UAOInventoryItemContextActionWidget::SetActionViewModel(UMVVM_InventoryItemContextAction* InActionViewModel)
{
	// 动作项 Widget 会被复用，因此这里不重新创建 ViewModel，只切换到主菜单当前分发下来的那一个。
	ActionViewModel = InActionViewModel;
	MVVM_InventoryItemContextAction = ActionViewModel != nullptr ? ActionViewModel : FallbackActionViewModel;

	BindViewModelToWidget();
	OnActionViewModelSet(ActionViewModel);
	RefreshFromViewModel();
}

void UAOInventoryItemContextActionWidget::RefreshFromViewModel()
{
	if (ActionButton != nullptr && ActionViewModel != nullptr)
	{
		ActionButton->SetIsEnabled(ActionViewModel->IsEnabled());
	}

	SetVisibility(ActionViewModel != nullptr ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
}

void UAOInventoryItemContextActionWidget::NativeOnInitialized()
{
	EnsureFallbackActionViewModel();

	// 先把占位 ViewModel 暴露给蓝图 MVVM 源，再进入 Super。
	// 这样即便蓝图 MVVM 源在父类初始化阶段就开始读数据，也不会先看到一个空源。
	MVVM_InventoryItemContextAction = FallbackActionViewModel;

	Super::NativeOnInitialized();

	if (ActionButton != nullptr)
	{
		ActionButton->OnClicked.AddDynamic(this, &ThisClass::HandleButtonClicked);
	}

	BindViewModelToWidget();
	RefreshFromViewModel();
}

void UAOInventoryItemContextActionWidget::HandleButtonClicked()
{
	OnInvoked.Broadcast(this);
}

void UAOInventoryItemContextActionWidget::EnsureFallbackActionViewModel()
{
	if (FallbackActionViewModel == nullptr)
	{
		FallbackActionViewModel = NewObject<UMVVM_InventoryItemContextAction>(this);
	}
}

void UAOInventoryItemContextActionWidget::BindViewModelToWidget()
{
	EnsureFallbackActionViewModel();

	// 蓝图 MVVM 初始化时拿到的是这个统一入口。
	// 真正打开菜单后它会被切到主菜单分发下来的动作项 ViewModel；没有真实动作时则退回占位对象。
	MVVM_InventoryItemContextAction = ActionViewModel != nullptr ? ActionViewModel : FallbackActionViewModel;

	if (MVVM_InventoryItemContextAction == nullptr)
	{
		return;
	}

	if (UMVVMView* View = UMVVMSubsystem::GetViewFromUserWidget(this))
	{
		TScriptInterface<INotifyFieldValueChanged> ViewModelInterface(MVVM_InventoryItemContextAction);
		View->SetViewModel(ActionViewModelName, ViewModelInterface);
	}

	// 动作项 Widget 会被反复复用。
	// 每次切换到新的动作项 ViewModel 后都主动重播一次当前快照，
	// 这样按钮文本和启用态会立刻切到最新数据，不会残留上一轮右键菜单的显示结果。
	MVVM_InventoryItemContextAction->BroadcastCurrentSnapshot();
}
