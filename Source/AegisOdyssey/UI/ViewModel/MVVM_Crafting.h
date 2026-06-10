#pragma once

#include "CoreMinimal.h"
#include "AegisOdyssey/Crafting/Data/AOCraftingObservationTypes.h"
#include "AegisOdyssey/UI/ViewModel/AOMVVMViewModelBase.h"
#include "MVVM_Crafting.generated.h"

class UAOCraftingComponent;

/**
 * 制造系统主 ViewModel。
 * 负责观察某个制造组件，并向 UI 暴露配方、详情、队列和制作反馈快照。
 */
UCLASS()
class AEGISODYSSEY_API UMVVM_Crafting : public UAOMVVMViewModelBase
{
	GENERATED_BODY()

public:
	DECLARE_MULTICAST_DELEGATE(FOnCraftingObservationChanged);

	UMVVM_Crafting(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/**
	 * 切换当前观察的制造组件。
	 * 后续所有列表、详情、队列和反馈都只从这一个观察源拉取。
	 */
	void SetObservedCraftingComponent(UAOCraftingComponent* InCraftingComponent);

	/** 返回当前正在被 UI 观察的制造组件。 */
	UAOCraftingComponent* GetObservedCraftingComponent() const;

	/** 从底层制造组件重新拉取整包观察快照。 */
	UFUNCTION(BlueprintCallable, Category = "AO|Crafting")
	void RefreshObservationData();

	/** 更新当前详情区和右键菜单共享的选中配方。 */
	UFUNCTION(BlueprintCallable, Category = "AO|Crafting")
	void SetSelectedRecipeRowName(FName InRecipeRowName);

	/** 兼容旧入口的“制作一个”请求。 */
	UFUNCTION(BlueprintCallable, Category = "AO|Crafting")
	bool RequestEnqueueRecipe(FName InRecipeRowName);

	/** 统一制作请求入口，Single/Ten/All 都通过这里下发到底层。 */
	UFUNCTION(BlueprintCallable, Category = "AO|Crafting")
	bool RequestCraftRecipe(FName InRecipeRowName, EAOCraftingRequestType RequestType);

	/** 对当前选中配方发起“制作一个”请求。 */
	UFUNCTION(BlueprintCallable, Category = "AO|Crafting")
	bool RequestEnqueueSelectedRecipe();

	/** 对当前选中配方发起指定类型的制作请求。 */
	UFUNCTION(BlueprintCallable, Category = "AO|Crafting")
	bool RequestCraftSelectedRecipe(EAOCraftingRequestType RequestType);

	/** 当前选中的配方行名。 */
	UFUNCTION(BlueprintPure, FieldNotify, Category = "AO|Crafting")
	FName GetSelectedRecipeRowName() const { return SelectedRecipeRowName; }

	/** 配方列表快照。 */
	UFUNCTION(BlueprintPure, FieldNotify, Category = "AO|Crafting")
	TArray<FAOCraftingRecipeListEntryViewData> GetRecipeList() const { return RecipeList; }

	/** 当前选中配方的详情快照。 */
	UFUNCTION(BlueprintPure, FieldNotify, Category = "AO|Crafting")
	const FAOCraftingRecipeDetailViewData& GetSelectedRecipeDetail() const { return SelectedRecipeDetail; }

	/** 制造队列快照。 */
	UFUNCTION(BlueprintPure, FieldNotify, Category = "AO|Crafting")
	TArray<FAOCraftingQueueEntryViewData> GetQueueList() const { return QueueList; }

	/** 队列 UI 需要渲染的固定槽位数量。 */
	UFUNCTION(BlueprintPure, FieldNotify, Category = "AO|Crafting")
	int32 GetQueueSlotCount() const { return QueueSlotCount; }

	/** 最近一次制作请求的用户可见反馈文案。 */
	UFUNCTION(BlueprintPure, FieldNotify, Category = "AO|Crafting")
	FText GetLastCraftRequestFeedback() const { return LastCraftRequestFeedback; }

	/** 当前是否存在一条可显示的制作反馈。 */
	UFUNCTION(BlueprintPure, FieldNotify, Category = "AO|Crafting")
	bool HasCraftRequestFeedback() const { return bHasCraftRequestFeedback; }

	/** 最近一次制作请求的完整结果。 */
	UFUNCTION(BlueprintPure, Category = "AO|Crafting")
	const FAOCraftingRequestResult& GetLastCraftRequestResult() const { return LastCraftRequestResult; }

	/** 当前选中配方是否允许开始制作。 */
	UFUNCTION(BlueprintPure, Category = "AO|Crafting")
	bool CanEnqueueSelectedRecipe() const;

	/** 将底层阻塞原因翻译成 UI 可直接显示的文本。 */
	UFUNCTION(BlueprintPure, Category = "AO|Crafting")
	FText GetRecipeBlockReasonText(EAOCraftingRecipeBlockReason InBlockReason) const;

	/** 当前选中配方的阻塞原因文本。 */
	UFUNCTION(BlueprintPure, Category = "AO|Crafting")
	FText GetSelectedRecipeBlockReasonText() const;

	/** 取出当前正在制作的 Active 队列项。 */
	UFUNCTION(BlueprintPure, Category = "AO|Crafting")
	bool GetActiveQueueEntry(FAOCraftingQueueEntryViewData& OutQueueEntry) const;

	/** 计算任意队列项当前的剩余秒数。 */
	UFUNCTION(BlueprintPure, Category = "AO|Crafting")
	float GetQueueEntryRemainingSeconds(const FAOCraftingQueueEntryViewData& QueueEntry) const;

	/** 计算任意队列项当前单件制作的进度比例。 */
	UFUNCTION(BlueprintPure, Category = "AO|Crafting")
	float GetQueueEntryProgressRatio(const FAOCraftingQueueEntryViewData& QueueEntry) const;

	/** 当前 Active 队列项剩余秒数的便捷入口。 */
	UFUNCTION(BlueprintPure, Category = "AO|Crafting")
	float GetActiveQueueRemainingSeconds() const;

	/** 当前 Active 队列项进度比例的便捷入口。 */
	UFUNCTION(BlueprintPure, Category = "AO|Crafting")
	float GetActiveQueueProgressRatio() const;

	/** 观察快照更新后供 UI 侧监听的非反射委托。 */
	FOnCraftingObservationChanged OnCraftingObservationChanged;

private:
	/** 底层制造组件广播变化后的统一响应入口。 */
	void HandleObservedCraftingObservationChanged();
	/** 解绑旧观察源上的事件。 */
	void UnbindObservedCraftingComponent();

	/**
	 * 返回当前客户端可对齐的服务端世界时间。
	 * 制造队列里的开始/结束时间由服务端写入，这里不能再直接使用客户端本地 World Time。
	 */
	float GetObservedServerWorldTimeSeconds() const;

	/** 当前 UI 绑定的制造真值源。 */
	TWeakObjectPtr<UAOCraftingComponent> ObservedCraftingComponent;
	/** 观察源变化委托句柄。 */
	FDelegateHandle ObservedCraftingObservationChangedHandle;

	/** 当前列表、详情和右键菜单共享的选中配方。 */
	UPROPERTY(BlueprintReadOnly, FieldNotify, meta = (AllowPrivateAccess))
	FName SelectedRecipeRowName = NAME_None;

	/** 配方列表快照缓存。 */
	UPROPERTY(BlueprintReadOnly, FieldNotify, meta = (AllowPrivateAccess))
	TArray<FAOCraftingRecipeListEntryViewData> RecipeList;

	/** 当前选中配方详情快照缓存。 */
	UPROPERTY(BlueprintReadOnly, FieldNotify, meta = (AllowPrivateAccess))
	FAOCraftingRecipeDetailViewData SelectedRecipeDetail;

	/** 制造队列快照缓存。 */
	UPROPERTY(BlueprintReadOnly, FieldNotify, meta = (AllowPrivateAccess))
	TArray<FAOCraftingQueueEntryViewData> QueueList;

	/** 队列 UI 消费的固定槽位数量。 */
	UPROPERTY(BlueprintReadOnly, FieldNotify, meta = (AllowPrivateAccess))
	int32 QueueSlotCount = 0;

	/** 最近一次制作请求的反馈文案缓存。 */
	UPROPERTY(BlueprintReadOnly, FieldNotify, meta = (AllowPrivateAccess))
	FText LastCraftRequestFeedback;

	/** 最近一次制作请求是否产生了可显示反馈。 */
	UPROPERTY(BlueprintReadOnly, FieldNotify, meta = (AllowPrivateAccess))
	bool bHasCraftRequestFeedback = false;

	/** 最近一次制作请求的完整结果缓存。 */
	FAOCraftingRequestResult LastCraftRequestResult;
};
