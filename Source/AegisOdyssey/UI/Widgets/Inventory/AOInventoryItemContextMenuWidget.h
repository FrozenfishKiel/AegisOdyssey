#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Input/Events.h"
#include "Input/Reply.h"
#include "AOInventoryItemContextMenuWidget.generated.h"

class UBorder;
class UButton;
class UImage;
class UMVVM_InventoryItemContextMenu;
class UPanelWidget;
class UTextBlock;
class UAOInventoryItemContextActionWidget;

// 库存右键菜单 Widget。
// 这一层只负责消费外部注入的 ContextMenuViewModel，并把动作项 ViewModel 分发给动作项子 Widget。
// 它不负责生成菜单业务语义，也不在这里手写按钮样式或构造库存动作规则。
UCLASS()
class AEGISODYSSEY_API UAOInventoryItemContextMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 构造右键菜单 Widget，并把自己标记为可聚焦。
	// 菜单要响应 Escape 关闭与其他键盘交互，因此这层必须允许拿到键盘焦点。
	UAOInventoryItemContextMenuWidget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// 每次菜单被重新打开时，按当前 ViewModel 快照刷新显示。
	// 它是菜单可重入场景下的统一刷新入口，负责把最新显示态重新投影到当前 Widget。
	void InitializeForInventorySlot();

	// 注入当前菜单对应的 ViewModel。
	// 菜单本体不自己查库存数据，只消费外部已经组好的菜单快照。
	// 切换 ViewModel 后，Widget 应把自己视为进入了一次新的菜单上下文。
	UFUNCTION(BlueprintCallable, Category = "AO|Inventory Context Menu")
	void SetContextMenuViewModel(UMVVM_InventoryItemContextMenu* InViewModel);

	// 返回当前菜单正在消费的 ViewModel。
	// 供蓝图或外部桥接逻辑读取当前绑定对象，不应用它反向修改业务状态。
	UFUNCTION(BlueprintPure, Category = "AO|Inventory Context Menu")
	UMVVM_InventoryItemContextMenu* GetContextMenuViewModel() const { return ContextMenuViewModel; }

protected:
	// Widget 初始化入口。
	// 这里主要完成一次性的事件绑定和初始显示同步，不承担菜单业务构建。
	virtual void NativeOnInitialized() override;

	// 处理键盘输入，当前主要关心 Escape 关闭菜单。
	// 只有菜单已拿到焦点时，这里才会成为键盘关闭路径。
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

	// 处理点击菜单外部时自动关闭。
	// 左键和右键点到菜单外部都会收起当前右键菜单，防止上下文菜单在界面上悬挂残留。
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	// 某个动作项子 Widget 被点击后的统一回调入口。
	// 这层只负责接住子 Widget 的点击通知，并把执行链交还给对应的动作项 ViewModel。
	UFUNCTION()
	void HandleActionWidgetInvoked(UAOInventoryItemContextActionWidget* ActionWidget);

	// 预留关闭按钮的点击回调。
	// 如果蓝图皮肤提供显式关闭按钮，就通过这里走统一关闭链。
	UFUNCTION()
	void HandleCloseButtonClicked();

	// 按当前菜单快照统一刷新显示。
	// 包括菜单可见性、头部文本图标、屏幕位置以及动作项区域的整体同步。
	void RefreshDisplay();

	// 刷新动作项区域，把主菜单里的动作项 ViewModel 分发给子 Widget。
	// 这里不负责定义动作按钮样式，只负责控制子 Widget 数量、可见性和数据注入。
	void RefreshActionEntries();

	// 主动关闭菜单，并把关闭动作回写给 ViewModel。
	// 这是菜单本体的统一收口关闭入口，避免按钮关闭、外部点击关闭、键盘关闭走成多套分支。
	void CloseMenu();

	// Blueprint 扩展点。
	// 当外部切换到新的菜单 ViewModel 时，蓝图可以在这里做文本绑定、动画准备或其他表现层初始化。
	UFUNCTION(BlueprintImplementableEvent, Category = "AO|Inventory Context Menu")
	void OnContextMenuViewModelSet(UMVVM_InventoryItemContextMenu* InViewModel);

protected:
	// 菜单实际内容的根节点，用来判断点击是否落在菜单内部。
	// 同时也是蓝图通常承载整体背景板、描边和内边距的主要容器。
	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UBorder> RootBorder = nullptr;

	// 物品图标控件。
	// 负责显示 ContextMenuViewModel 当前提供的头部物品图标。
	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UImage> ItemIconImage = nullptr;

	// 物品名称文本控件。
	// 一般绑定或同步 ViewModel 中的 ItemDisplayName。
	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ItemNameText = nullptr;

	// 物品补充信息文本控件。
	// 用于显示名称下方的堆叠数、说明文字或其他次级信息。
	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ItemInfoText = nullptr;

	// 动作项列表容器。
	// 它只负责承接若干动作项子 Widget，具体按钮样式和行布局完全由动作项蓝图 Widget 承载。
	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UPanelWidget> ActionListContainer = nullptr;

	// 预留关闭按钮。
	// 不是所有菜单皮肤都必须有它，因此这里保持 Optional 绑定。
	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UButton> CloseButton = nullptr;

	// 动作项 Widget 蓝图类。
	// 主菜单不会自己在 C++ 里构造按钮样式，而是依赖这个蓝图类生成每一行动作项视图。
	UPROPERTY(EditDefaultsOnly, Category = "AO|Inventory Context Menu")
	TSubclassOf<UAOInventoryItemContextActionWidget> ActionEntryWidgetClass;

	// 动作项 Widget 池。
	// 菜单重复打开时优先复用已有子 Widget，减少频繁 CreateWidget 带来的瞬时分配和回收压力。
	UPROPERTY(Transient)
	TArray<TObjectPtr<UAOInventoryItemContextActionWidget>> ActionEntryWidgets;

	// 当前右键菜单的唯一数据入口。
	// 这层 Widget 的头部显示、动作列表和显隐状态都应以它为准，不再各处散落读取别的数据源。
	UPROPERTY(BlueprintReadOnly, Category = "AO|Inventory Context Menu", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMVVM_InventoryItemContextMenu> ContextMenuViewModel = nullptr;

private:
	// 把外部注入的 ViewModel 注册到 UMVVMView，供蓝图绑定使用。
	// 这是蓝图 MVVM 绑定能否拿到菜单 ViewModel 的关键桥接步骤。
	void BindViewModelToWidget();

	// 兼容尚未完全走纯绑定的菜单皮肤，手动同步头部显示。
	// 这里只处理头部图标和文本，不参与动作决议、动作生成或库存业务逻辑。
	void RefreshHeaderFromViewModel();

	// 确保动作项子 Widget 池至少有指定数量。
	// 这里只做实例复用和扩容，不在 C++ 里定义任何按钮、文本或交互样式细节。
	void EnsureActionEntryPoolSize(int32 RequiredEntryCount);

	// 把菜单请求时记录的屏幕绝对坐标，转换成当前玩家屏幕根 Widget 的本地坐标。
	// 同时应用鼠标右侧偏移和边缘回收，保证菜单尽量稳定地出现在鼠标右侧且不飞出屏幕。
	FVector2D ResolveMenuCanvasPosition() const;

	// 注册到 MVVM 视图时使用的固定 ViewModel 名称。
	// 蓝图如果要通过 MVVM View 拿菜单 ViewModel，需要和这个名字保持一致，否则绑定会拿不到对象。
	static const FName ContextMenuViewModelName;
};
