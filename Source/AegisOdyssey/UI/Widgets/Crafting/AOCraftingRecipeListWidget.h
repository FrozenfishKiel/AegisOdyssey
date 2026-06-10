// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AegisOdyssey/Crafting/Data/AOCraftingObservationTypes.h"
#include "AegisOdyssey/UI/ViewModel/Inventory/AOInventoryItemContextMenuTypes.h"
#include "AegisOdyssey/UI/Widgets/Crafting/AOCraftingWidgetBase.h"
#include "AOCraftingRecipeListWidget.generated.h"

class UAOInventoryItemContextMenuWidget;
class UAOInventoryItemDefinition;
class UButton;
class UImage;
class UProgressBar;
class UAOCraftingComponent;
class UMVVM_ItemHoverTooltip;
class UMVVM_InventoryItemContextMenu;
class UPanelWidget;
class UTextBlock;
class UWidget;
struct FSlateBrush;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAOCraftingRecipeListEntrySelected, FName, RecipeRowName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FAOCraftingRecipeListEntryContextRequested, FName, RecipeRowName, FVector2D, ScreenSpacePosition);

enum class EAOCraftingRecipeListEntryDisplayMode : uint8
{
	Recipe,
	Material,
	Output,
	Queue,
	QueueEmpty
};

/**
 * 复用型制造列表条目控件。
 * 同一个 Widget 类会根据 DisplayMode 切换为配方、材料、产物、队列项和空队列占位五种显示形态。
 */
UCLASS(Abstract, Blueprintable)
class AEGISODYSSEY_API UAOCraftingRecipeListEntryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** 用配方列表条目快照刷新这一行，并同步选中态。 */
	UFUNCTION(BlueprintCallable, Category = "AO|Crafting UI")
	void SetEntryData(const FAOCraftingRecipeListEntryViewData& InEntryData, bool bInSelected);

	/** 用材料条目快照刷新这一行。 */
	UFUNCTION(BlueprintCallable, Category = "AO|Crafting UI")
	void SetMaterialData(const FAOCraftingMaterialViewData& InMaterialData);

	/** 用产物条目快照刷新这一行。 */
	UFUNCTION(BlueprintCallable, Category = "AO|Crafting UI")
	void SetOutputData(const FAOCraftingOutputViewData& InOutputData);

	/** 用队列条目快照和当前进度刷新这一行。 */
	UFUNCTION(BlueprintCallable, Category = "AO|Crafting UI")
	void SetQueueEntryData(const FAOCraftingQueueEntryViewData& InQueueEntryData, float InProgressPercent);

	/** 将复用条目切换为空队列槽位占位显示。 */
	UFUNCTION(BlueprintCallable, Category = "AO|Crafting UI")
	void SetQueueEmptyState();

	/** 返回当前配方条目缓存的阻塞原因，仅在配方模式下有意义。 */
	UFUNCTION(BlueprintPure, Category = "AO|Crafting UI")
	EAOCraftingRecipeBlockReason GetBlockReason() const { return EntryData.BlockReason; }

	/** 左键选中配方条目时抛出的事件。 */
	UPROPERTY(BlueprintAssignable, Category = "AO|Crafting UI")
	FAOCraftingRecipeListEntrySelected OnSelected;

	/** 右键请求打开配方上下文菜单时抛出的事件。 */
	UPROPERTY(BlueprintAssignable, Category = "AO|Crafting UI")
	FAOCraftingRecipeListEntryContextRequested OnContextRequested;

protected:
	/** 初始化按钮事件等一次性绑定。 */
	virtual void NativeOnInitialized() override;
	/** 清理悬浮提示等运行期状态。 */
	virtual void NativeDestruct() override;
	/** 鼠标进入时尝试显示悬浮提示。 */
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	/** 鼠标离开时隐藏悬浮提示。 */
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;
	/** 处理右键菜单请求等鼠标按下交互。 */
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

private:
	/** 处理 SelectButton 的点击，向外广播当前配方选中事件。 */
	UFUNCTION()
	void HandleSelectButtonClicked();

	/** 根据当前 DisplayMode 将缓存数据刷新到控件树。 */
	void RefreshDisplay();
	/** 刷新配方模式显示。 */
	void RefreshRecipeDisplay();
	/** 刷新材料模式显示。 */
	void RefreshMaterialDisplay();
	/** 刷新产物模式显示。 */
	void RefreshOutputDisplay();
	/** 刷新队列模式显示，包括进度条和空槽位占位。 */
	void RefreshQueueDisplay();

	/** 优先使用 Definition 名称，否则回退到传入的 Name。 */
	FText ResolveDisplayName(const UAOInventoryItemDefinition* ItemDefinition, FName FallbackName) const;
	/** 从 Definition 解析图标刷子。 */
	bool ResolveIcon(const UAOInventoryItemDefinition* ItemDefinition, FSlateBrush& OutBrush) const;
	/** 组装配方模式下的制作时长文本。 */
	FText BuildDurationText() const;
	/** 组装配方模式下的状态文本。 */
	FText BuildStateText() const;
	/** 组装材料模式下的需求/库存数量文本。 */
	FText BuildMaterialCountText() const;
	/** 组装产物模式下的产出数量文本。 */
	FText BuildOutputCountText() const;
	/** 组装队列模式下的剩余数量/总数量文本。 */
	FText BuildQueueCountText() const;
	/** 组装队列模式下的状态文本。 */
	FText BuildQueueStatusText() const;

	/**
	 * 将当前条目统一解释成 Tooltip 需要的 ItemDefinition。
	 * 配方、材料和产物虽然来源不同，但悬浮提示最终都只认 Definition。
	 */
	const UAOInventoryItemDefinition* ResolveHoverTooltipItemDefinition() const;
	/** 根据当前条目数据打开悬浮提示。 */
	void ShowHoverTooltip(const FGeometry& InGeometry);
	/** 关闭悬浮提示。 */
	void HideHoverTooltip();

protected:
	/** 配方模式下的点击按钮。 */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	TObjectPtr<UButton> SelectButton = nullptr;

	/** 配方模式下显示配方主图标。 */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	TObjectPtr<UImage> RecipeIconImage = nullptr;

	/** 配方模式下显示配方名称。 */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> RecipeNameText = nullptr;

	/** 配方模式下显示制作时长。 */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> DurationText = nullptr;

	/** 配方或队列模式下显示状态文本。 */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> StateText = nullptr;

	/** 配方模式下显示选中态。 */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	TObjectPtr<UWidget> SelectedIndicator = nullptr;

	/** 材料、产物、队列模式下显示物品图标。 */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	TObjectPtr<UImage> ItemIconImage = nullptr;

	/** 材料、产物、队列模式下显示物品名称。 */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ItemNameText = nullptr;

	/** 材料、产物、队列模式下显示数量文本。 */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> CountText = nullptr;

	/** 材料或队列模式下显示补充状态文本。 */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> StatusText = nullptr;

	/** 材料模式下显示材料是否满足。 */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	TObjectPtr<UWidget> SatisfiedIndicator = nullptr;

	/** 队列模式下显示当前单件制作进度。 */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> ProgressBar = nullptr;

	/** 空队列槽位模式下显示占位内容。 */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	TObjectPtr<UWidget> QueueEmptyPlaceholder = nullptr;

private:
	/** 当前条目承载的配方列表快照。 */
	UPROPERTY(BlueprintReadOnly, Category = "AO|Crafting UI", meta = (AllowPrivateAccess = true))
	FAOCraftingRecipeListEntryViewData EntryData;

	/** 当前条目承载的材料快照。 */
	UPROPERTY(BlueprintReadOnly, Category = "AO|Crafting UI", meta = (AllowPrivateAccess = true))
	FAOCraftingMaterialViewData MaterialData;

	/** 当前条目承载的产物快照。 */
	UPROPERTY(BlueprintReadOnly, Category = "AO|Crafting UI", meta = (AllowPrivateAccess = true))
	FAOCraftingOutputViewData OutputData;

	/** 当前条目承载的队列快照。 */
	UPROPERTY(BlueprintReadOnly, Category = "AO|Crafting UI", meta = (AllowPrivateAccess = true))
	FAOCraftingQueueEntryViewData QueueEntryData;

	/** 列表控件推送下来的当前队列进度百分比。 */
	UPROPERTY(BlueprintReadOnly, Category = "AO|Crafting UI", meta = (AllowPrivateAccess = true))
	float QueueProgressPercent = 0.0f;

	/** 当前是否处于配方模式的选中态。 */
	UPROPERTY(BlueprintReadOnly, Category = "AO|Crafting UI", meta = (AllowPrivateAccess = true))
	bool bSelected = false;

	/** 当前这一行正以哪种显示模式解释缓存数据。 */
	EAOCraftingRecipeListEntryDisplayMode DisplayMode = EAOCraftingRecipeListEntryDisplayMode::Recipe;
};

/**
 * 制造主列表控件。
 * 负责承接 ViewModel 快照，驱动配方列表、固定数量队列槽位和配方右键菜单。
 */
UCLASS(Abstract, Blueprintable)
class AEGISODYSSEY_API UAOCraftingRecipeListWidget : public UAOCraftingWidgetBase
{
	GENERATED_BODY()

public:
	/** 返回当前缓存的配方列表快照。 */
	UFUNCTION(BlueprintPure, Category = "AO|Crafting UI")
	TArray<FAOCraftingRecipeListEntryViewData> GetRecipeList() const { return CachedRecipeList; }

	/** 切换当前选中的配方。 */
	UFUNCTION(BlueprintCallable, Category = "AO|Crafting UI")
	void SelectRecipe(FName InRecipeRowName);

	/** 返回当前选中的配方行名。 */
	UFUNCTION(BlueprintPure, Category = "AO|Crafting UI")
	FName GetSelectedRecipeRowName() const { return SelectedRecipeRowName; }

protected:
	/** 每帧仅本地刷新 Active 队列项的实时进度条。 */
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	/** ViewModel 快照变化后，刷新配方列表和队列列表。 */
	virtual void HandleCraftingViewModelChanged() override;

private:
	/** 按当前配方快照重建配方条目控件。 */
	void RebuildRecipeEntryWidgets();
	/** 按固定 QueueSlotCount 重建队列槽位控件。 */
	void RebuildQueueEntryWidgets();
	/** 用最新队列快照填充所有固定槽位。 */
	void RefreshQueueEntryWidgets();
	/** 在两次快照刷新之间，仅更新 Active 队列项的本地进度。 */
	void RefreshQueueProgressWidgets();
	/** 通过 ViewModel 的统一公式计算某个队列项的进度百分比。 */
	float ResolveQueueEntryProgressPercent(const FAOCraftingQueueEntryViewData& QueueEntry) const;
	/** 确保右键菜单控件实例已创建。 */
	void EnsureContextMenuWidget();
	/** 构建当前选中配方可用的右键菜单动作。 */
	bool BuildCraftingContextMenuActions(TArray<FAOInventoryItemContextAction>& OutActions) const;
	/** 解析右键菜单展示需要的配方名称和图标。 */
	bool ResolveRecipeContextVisuals(
		FName InRecipeRowName,
		FText& OutDisplayName,
		FSlateBrush& OutIconBrush,
		bool& bOutHasValidIcon) const;

	/** 处理配方条目选中事件。 */
	UFUNCTION()
	void HandleRecipeEntrySelected(FName InRecipeRowName);

	/** 处理配方条目的右键菜单请求。 */
	UFUNCTION()
	void HandleRecipeEntryContextRequested(FName InRecipeRowName, FVector2D ScreenSpacePosition);

protected:
	/** 配方条目容器。 */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	TObjectPtr<UPanelWidget> RecipeListContainer = nullptr;

	/** 队列槽位容器。 */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	TObjectPtr<UPanelWidget> QueueListContainer = nullptr;

	/** 配方列表使用的条目 Widget 类。 */
	UPROPERTY(EditDefaultsOnly, Category = "AO|Crafting UI")
	TSubclassOf<UAOCraftingRecipeListEntryWidget> RecipeEntryWidgetClass;

	/** 队列槽位使用的条目 Widget 类。 */
	UPROPERTY(EditDefaultsOnly, Category = "AO|Crafting UI")
	TSubclassOf<UAOCraftingRecipeListEntryWidget> QueueEntryWidgetClass;

	/** 复用库存系统已有的右键菜单 Widget 类。 */
	UPROPERTY(EditDefaultsOnly, Category = "AO|Crafting UI")
	TSubclassOf<UAOInventoryItemContextMenuWidget> ContextMenuWidgetClass;

private:
	/** 当前观察源下的配方列表快照。 */
	UPROPERTY(BlueprintReadOnly, Category = "AO|Crafting UI", meta = (AllowPrivateAccess = true))
	TArray<FAOCraftingRecipeListEntryViewData> CachedRecipeList;

	/** 当前观察源下的队列列表快照。 */
	UPROPERTY(BlueprintReadOnly, Category = "AO|Crafting UI", meta = (AllowPrivateAccess = true))
	TArray<FAOCraftingQueueEntryViewData> CachedQueueList;

	/** 队列区域应渲染的固定槽位数量。 */
	UPROPERTY(BlueprintReadOnly, Category = "AO|Crafting UI", meta = (AllowPrivateAccess = true))
	int32 QueueSlotCount = 0;

	/** 当前选中的配方行名。 */
	UPROPERTY(BlueprintReadOnly, Category = "AO|Crafting UI", meta = (AllowPrivateAccess = true))
	FName SelectedRecipeRowName = NAME_None;

	/** 当前配方列表里已经创建出来的条目控件。 */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UAOCraftingRecipeListEntryWidget>> RecipeEntryWidgets;

	/** QueueListContainer 下复用的固定队列槽位控件。 */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UAOCraftingRecipeListEntryWidget>> QueueEntryWidgets;

	/** 当前激活中的右键菜单控件实例。 */
	UPROPERTY(Transient)
	TObjectPtr<UAOInventoryItemContextMenuWidget> ActiveContextMenuWidget = nullptr;
};
