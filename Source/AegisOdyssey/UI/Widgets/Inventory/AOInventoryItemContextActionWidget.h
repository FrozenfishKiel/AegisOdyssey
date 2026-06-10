#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AOInventoryItemContextActionWidget.generated.h"

class UButton;
class UMVVM_InventoryItemContextAction;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAOInventoryItemContextActionWidgetInvoked, UAOInventoryItemContextActionWidget*, ActionWidget);

// 右键菜单中的单个动作项 Widget。
// 样式完全交给蓝图承载，C++ 只负责注入动作项 ViewModel、同步基础交互状态并转发点击。
// 你可以把它理解成“菜单里的一行”，而不是会自己决定业务逻辑的按钮类。
UCLASS()
class AEGISODYSSEY_API UAOInventoryItemContextActionWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 注入这一行动作项对应的 ViewModel。
	// 每次菜单刷新时，主菜单都会把对应的动作项数据重新分发给子 Widget。
	// 该函数的目标是切换这行 UI 当前消费的数据源，而不是创建新的动作语义。
	UFUNCTION(BlueprintCallable, Category = "AO|Inventory Context Menu")
	void SetActionViewModel(UMVVM_InventoryItemContextAction* InActionViewModel);

	// 返回当前动作项正在消费的 ViewModel。
	// 蓝图若需要绑定这行数据，可以通过它拿到当前动作项的唯一数据入口。
	UFUNCTION(BlueprintPure, Category = "AO|Inventory Context Menu")
	UMVVM_InventoryItemContextAction* GetActionViewModel() const { return ActionViewModel; }

	// 按当前动作项 ViewModel 刷新显示态。
	// 目前主要负责同步按钮可点击状态、可见性等基础表现，不在这里决定按钮文案和业务语义。
	UFUNCTION(BlueprintCallable, Category = "AO|Inventory Context Menu")
	void RefreshFromViewModel();

	// 对外广播“这一行动作被点击了”。
	// 主菜单 Widget 会监听这个事件，再把执行链回收到统一的菜单逻辑中。
	UPROPERTY(BlueprintAssignable)
	FAOInventoryItemContextActionWidgetInvoked OnInvoked;

protected:
	// Widget 初始化入口。
	// 这里主要做一次性的按钮事件绑定，并补一次首帧显示同步。
	virtual void NativeOnInitialized() override;

	// 内部按钮点击回调。
	// 当前类不自己执行库存逻辑，只负责把“这行被点了”这个事实向外广播。
	UFUNCTION()
	void HandleButtonClicked();

	// Blueprint 扩展点。
	// 当动作项 ViewModel 被替换时，蓝图可以在这里绑定文本、图标、样式切换或过渡表现。
	UFUNCTION(BlueprintImplementableEvent, Category = "AO|Inventory Context Menu")
	void OnActionViewModelSet(UMVVM_InventoryItemContextAction* InActionViewModel);

protected:
	// 供动作项蓝图 MVVM 源在 Widget 初始化阶段直接读取的当前 ViewModel。
	// 名字需要和蓝图里登记的 ViewModel Source 名保持一致，这样蓝图第一次初始化绑定时就能拿到一个有效对象，
	// 避免 CreateWidget 刚创建出来时因为源还是空指针而在日志里报 invalid source。
	UPROPERTY(BlueprintReadOnly, Category = "AO|Inventory Context Menu", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMVVM_InventoryItemContextAction> MVVM_InventoryItemContextAction = nullptr;

	// 动作项主按钮。
	// 它承载这一行动作的主要点击区域，具体视觉样式、排版和交互动效都由蓝图负责。
	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UButton> ActionButton = nullptr;

	// 当前动作项的唯一数据入口。
	// 该 Widget 的文本、图标、启用态等展示都应围绕它组织，不再散落读取其他外部状态。
	UPROPERTY(BlueprintReadOnly, Category = "AO|Inventory Context Menu", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMVVM_InventoryItemContextAction> ActionViewModel = nullptr;

private:
	// 确保动作项 Widget 在真正接到业务动作前，也有一个可复用的占位 ViewModel。
	// 这样 MVVM 蓝图初始化时永远能拿到合法源；后续真正打开菜单时，再把占位源切换成主菜单里复用的动作项 ViewModel。
	void EnsureFallbackActionViewModel();

	// 把当前动作项 ViewModel 注册到 UMVVMView，供动作项蓝图直接绑定 Label / IsEnabled 等字段。
	// 动作项 ViewModel 会被主菜单复用，因此这里不能假设“只在第一次创建按钮时绑一次就够了”。
	void BindViewModelToWidget();

	// 注册到动作项蓝图 MVVM 视图时使用的固定名称。
	// 动作项蓝图如果直接绑动作项 ViewModel，名字必须和这里保持一致。
	static const FName ActionViewModelName;

	// 动作项隐藏或尚未接到真实动作时使用的占位 ViewModel。
	// 生命周期跟随当前动作项 Widget，本质上只是用来托底 MVVM 初始化时序，不参与真实库存动作执行。
	UPROPERTY(Transient)
	TObjectPtr<UMVVM_InventoryItemContextAction> FallbackActionViewModel = nullptr;
};
