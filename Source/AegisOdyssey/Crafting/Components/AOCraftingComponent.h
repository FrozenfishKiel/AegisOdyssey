#pragma once

#include "CoreMinimal.h"
#include "Components/PawnComponent.h"
#include "AegisOdyssey/Crafting/Data/AOCraftingObservationTypes.h"
#include "AegisOdyssey/Crafting/Data/AOCraftingRecipeTypes.h"
#include "AegisOdyssey/AbilitySystem/Attributes/Combat/AOCombatAttributeSet.h"
#include "AegisOdyssey/AbilitySystem/Attributes/Core/AOHealthAttributeSet.h"
#include "AOCraftingComponent.generated.h"

class AAOItem;
class UAOInventoryComponent;
class UAOInventoryItemDefinition;
class UDataTable;
class UMVVM_InventoryItemContextMenu;
struct FAOItemCatalogRow;
struct FGameplayEffectSpec;
struct FAOInventoryReceiveBatch;

UENUM(BlueprintType)
enum class EAOCraftingQueueEntryState : uint8
{
	Queued,
	Active
};

/**
 * 单条制造队列记录。
 * 一条队列项可以代表一整批制作请求，但内部仍按 1 个 1 个顺序完成。
 */
USTRUCT(BlueprintType)
struct FAOCraftingQueueEntry
{
	GENERATED_BODY()

	/** 队列项唯一 Id，UI 刷新时依赖它稳定追踪同一批制作记录。 */
	UPROPERTY(BlueprintReadOnly, Category = "Crafting")
	int32 QueueEntryId = INDEX_NONE;

	/** 这条队列项对应的配方行名。 */
	UPROPERTY(BlueprintReadOnly, Category = "Crafting")
	FName RecipeRowName = NAME_None;

	/** 当前是等待中还是正在制作。 */
	UPROPERTY(BlueprintReadOnly, Category = "Crafting")
	EAOCraftingQueueEntryState State = EAOCraftingQueueEntryState::Queued;

	/** 单次制作 1 个成品所需时长，批量制作会重复使用这份时长。 */
	UPROPERTY(BlueprintReadOnly, Category = "Crafting")
	float ResolvedDurationSeconds = 0.0f;

	/** 当前这一轮单件制作的开始时间，仅 Active 阶段有意义。 */
	UPROPERTY(BlueprintReadOnly, Category = "Crafting")
	float StartServerWorldTimeSeconds = 0.0f;

	/** 当前这一轮单件制作的预计完成时间，仅 Active 阶段有意义。 */
	UPROPERTY(BlueprintReadOnly, Category = "Crafting")
	float ExpectedFinishServerWorldTimeSeconds = 0.0f;

	/** 这条批量队列最初总共要制作多少个。 */
	UPROPERTY(BlueprintReadOnly, Category = "Crafting")
	int32 TotalCraftCount = 0;

	/** 这条批量队列还剩多少个未完成，每完成 1 个就递减 1。 */
	UPROPERTY(BlueprintReadOnly, Category = "Crafting")
	int32 RemainingCraftCount = 0;

	/** 每次完成时实际发放的产物模板，批量制作会重复使用。 */
	UPROPERTY(BlueprintReadOnly, Category = "Crafting")
	TArray<FAOCraftingResolvedItemEntry> OutputEntries;
};

/** 单次整批扣料计划中的一条槽位扣料记录。 */
USTRUCT()
struct FAOCraftingMaterialConsumePlanEntry
{
	GENERATED_BODY()

	/** 本次扣料落在哪个库存组件。 */
	UPROPERTY()
	TObjectPtr<UAOInventoryComponent> InventoryComponent = nullptr;

	/** 本次扣料落在哪个槽位。 */
	UPROPERTY()
	int32 SlotIndex = INDEX_NONE;

	/** 要扣除的材料 ItemId。 */
	UPROPERTY()
	int32 ItemId = INDEX_NONE;

	/** 这个槽位本次要扣除的数量。 */
	UPROPERTY()
	int32 Count = 0;
};

/**
 * 制造系统核心组件。
 * 负责处理制作请求、材料扣除、队列推进、产物发放，以及向 UI 暴露观察快照。
 */
UCLASS(Blueprintable, meta = (BlueprintSpawnableComponent))
class AEGISODYSSEY_API UAOCraftingComponent : public UPawnComponent
{
	GENERATED_BODY()

public:
	DECLARE_MULTICAST_DELEGATE(FOnCraftingObservationChanged);

	UAOCraftingComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/** 兼容旧入口的“制作一个”请求。 */
	UFUNCTION(BlueprintCallable, Category = "Crafting")
	bool RequestEnqueueRecipe(FName RecipeRowName);

	/**
	 * 统一制作请求入口。
	 * Single、Ten、All 都会先按“当前最多还能做多少个”裁剪，再决定是否真正开始制作。
	 */
	UFUNCTION(BlueprintCallable, Category = "Crafting")
	bool RequestCraftRecipe(FName RecipeRowName, EAOCraftingRequestType RequestType);

	/**
	 * 当前制造队列的权威快照。
	 * 这里的数量语义已经是“批量队列项”，不是单件成品数量。
	 */
	UFUNCTION(BlueprintPure, Category = "Crafting")
	const TArray<FAOCraftingQueueEntry>& GetCraftingQueue() const { return CraftingQueue; }

	/** 返回队列 UI 需要显示的固定槽位数。 */
	UFUNCTION(BlueprintPure, Category = "Crafting")
	int32 GetMaxQueueSize() const { return MaxQueueSize; }

	/**
	 * 取出或创建当前制造源持有的右键菜单主 ViewModel。
	 * 该对象只服务制造配方菜单，不再依赖 AOInventoryUI 外层宿主。
	 */
	UMVVM_InventoryItemContextMenu* GetOrCreateCraftingContextMenuViewModel();

	/** 最近一次制作请求的受理结果与反馈文案。 */
	UFUNCTION(BlueprintPure, Category = "Crafting")
	const FAOCraftingRequestResult& GetLastCraftRequestResult() const { return LastCraftRequestResult; }

	/** 构造配方列表 UI 所需快照。 */
	UFUNCTION(BlueprintPure, Category = "Crafting")
	TArray<FAOCraftingRecipeListEntryViewData> BuildRecipeListViewData() const;

	/** 构造指定配方详情区所需快照。 */
	UFUNCTION(BlueprintPure, Category = "Crafting")
	bool BuildRecipeDetailViewData(FName RecipeRowName, FAOCraftingRecipeDetailViewData& OutViewData) const;

	/** 构造制造队列 UI 所需快照。 */
	UFUNCTION(BlueprintPure, Category = "Crafting")
	TArray<FAOCraftingQueueEntryViewData> BuildQueueViewData() const;

	/**
	 * 处理角色死亡等运行时中断。
	 * 当前设计下不会返还已经预扣的材料。
	 */
	void HandleOwnerRuntimeInterrupted();

	/** 制造观察数据变化后供 UI 监听的非反射委托。 */
	FOnCraftingObservationChanged OnCraftingObservationChanged;

protected:
	/** 初始化运行期依赖和监听。 */
	virtual void BeginPlay() override;
	/** 结束时解绑运行期监听。 */
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	/** 注册复制字段。 */
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	/** 组件注册时初始化库存观察源。 */
	virtual void OnRegister() override;

	/** CraftingQueue 复制到客户端后的同步入口。 */
	UFUNCTION()
	void OnRep_CraftingQueue();

	/** 旧的单次制作 RPC，保留给历史调用链。 */
	UFUNCTION(Server, Reliable)
	void ServerRequestEnqueueRecipe(FName RecipeRowName);

	/** 统一制作请求的服务端 RPC。 */
	UFUNCTION(Server, Reliable)
	void ServerRequestCraftRecipe(FName RecipeRowName, EAOCraftingRequestType RequestType);

	/** 服务端将最近一次请求结果同步给本地玩家 UI。 */
	UFUNCTION(Client, Reliable)
	void ClientNotifyCraftRequestResult(const FAOCraftingRequestResult& RequestResult);

private:
	/** 配方运行期解析结果。 */
	struct FResolvedCraftingRecipeRuntimeData
	{
		const FAOCraftingRecipeRow* RecipeRow = nullptr;
		TArray<FAOCraftingResolvedItemEntry> ResolvedOutputs;
		float ResolvedDurationSeconds = 0.0f;
	};

	/** 扣料预演时记录的单槽位剩余数量快照。 */
	struct FInventorySlotSnapshot
	{
		UAOInventoryComponent* InventoryComponent = nullptr;
		int32 SlotIndex = INDEX_NONE;
		int32 RemainingCount = 0;
	};

	/** 服务端权威受理制作请求的主入口。 */
	FAOCraftingRequestResult TryRequestCraftRecipeOnAuthority(FName RecipeRowName, EAOCraftingRequestType RequestType);
	/** 历史单次制作入口的服务端实现。 */
	bool TryEnqueueRecipeOnAuthority(FName RecipeRowName);
	/** 判断配方是否已对拥有者解锁。 */
	bool IsRecipeUnlockedForOwner(FName RecipeRowName) const;
	/** 解析配方运行期用到的产物与时长数据。 */
	bool ResolveRecipeRuntimeData(FName RecipeRowName, FResolvedCraftingRecipeRuntimeData& OutRuntimeData) const;
	/** 将产物条目转换成库存接收批次。 */
	bool BuildOutputReceiveBatch(const TArray<FAOCraftingResolvedItemEntry>& OutputEntries, FAOInventoryReceiveBatch& OutReceiveBatch) const;

	/**
	 * 按目标制作数量构造一次性扣料计划。
	 * 批量请求会先整批扣料，再按 1 个 1 个顺序完成队列项。
	 */
	bool BuildMaterialConsumePlan(const FAOCraftingRecipeRow& RecipeRow, int32 CraftCount, TArray<FAOCraftingMaterialConsumePlanEntry>& OutConsumePlan) const;
	/** 执行扣料计划，并返回已成功执行的计划条数。 */
	bool ExecuteMaterialConsumePlan(const TArray<FAOCraftingMaterialConsumePlanEntry>& ConsumePlan, int32& OutConsumedPlanCount);
	/** 当扣料中途失败时回滚已执行部分。 */
	void RollbackMaterialConsumePlan(const TArray<FAOCraftingMaterialConsumePlanEntry>& ConsumePlan, int32 ConsumedPlanCount);
	/** 统计当前所有观察库存里某个 ItemId 可用的材料总数。 */
	int32 CountAvailableMaterialByItemId(int32 ItemId) const;

	/** 计算某个配方在当前材料条件下最多还能做多少个。 */
	int32 CountMaxCraftableCount(const FAOCraftingRecipeRow& RecipeRow) const;

	/** 按请求类型把理论请求数裁剪成这次实际能做的数量。 */
	int32 ResolveActualCraftCount(EAOCraftingRequestType RequestType, int32 MaxCraftableCount) const;

	/** 构造制作请求失败结果。 */
	FAOCraftingRequestResult BuildCraftRequestFailureResult(
		FName RecipeRowName,
		EAOCraftingRequestType RequestType,
		EAOCraftingRecipeBlockReason FailureReason) const;
	/** 构造制作请求成功结果。 */
	FAOCraftingRequestResult BuildCraftRequestSuccessResult(
		FName RecipeRowName,
		EAOCraftingRequestType RequestType,
		int32 RequestedCraftCount,
		int32 ActualCraftCount) const;

	/** 解析当前配方在现状下的阻塞原因和最终时长。 */
	EAOCraftingRecipeBlockReason ResolveRecipeBlockReason(FName RecipeRowName, const FAOCraftingRecipeRow* RecipeRow, float& OutResolvedDurationSeconds) const;
	/** 通过 ItemId 查找物品定义。 */
	const UAOInventoryItemDefinition* FindItemDefinitionByItemId(int32 ItemId) const;
	/** 从配方行解析主要产物定义。 */
	UAOInventoryItemDefinition* ResolvePrimaryOutputDefinition(const FAOCraftingRecipeRow* RecipeRow) const;
	/** 从已解析产物条目中解析主要产物定义。 */
	UAOInventoryItemDefinition* ResolvePrimaryOutputDefinition(const TArray<FAOCraftingResolvedItemEntry>& OutputEntries) const;
	/** 从配方表查找指定配方行。 */
	bool FindRecipeRow(FName RecipeRowName, const FAOCraftingRecipeRow*& OutRecipeRow) const;

	/** 当前没有 Active 项时，从队列头部启动下一条批量制造。 */
	bool StartNextQueuedEntry();

	/**
	 * 处理当前单件制作完成。
	 * 如果是批量队列，则每次只发放 1 次产物、剩余次数减 1，然后继续下一轮单件制作。
	 */
	void HandleActiveCraftingFinished();
	/** 查找当前 Active 队列项索引。 */
	int32 FindActiveCraftingEntryIndex() const;
	/** 向 UI 广播制造观察数据变化。 */
	void NotifyCraftingObservationChanged();
	/** 按当前 Active 项刷新完成计时器。 */
	void RefreshActiveCraftingTimer();
	/** 解析配方的最终制作时长。 */
	float ResolveCraftingDurationSeconds(const FAOCraftingRecipeRow& RecipeRow) const;
	/** 解析拥有者提供的总制作速度加成。 */
	float ResolveTotalCraftingSpeedBonus() const;
	/** 库存装不下时将产物掉落到世界。 */
	bool DropCraftingOutputsToWorld(const TArray<FAOCraftingResolvedItemEntry>& OutputEntries) const;
	/** 更新最近一次制作请求结果缓存。 */
	void UpdateLastCraftRequestResult(const FAOCraftingRequestResult& RequestResult);
	/** 获取拥有者当前使用的制造配方表。 */
	const UDataTable* GetOwnerCraftingRecipeTable() const;
	/** 解析拥有者的战斗属性集。 */
	const UAOCombatAttributeSet* ResolveCombatAttributeSet() const;
	/** 解析拥有者的生命属性集。 */
	const UAOHealthAttributeSet* ResolveHealthAttributeSet() const;
	/** 绑定死亡中断监听。 */
	void TryBindOwnerOutOfHealthDelegate();
	/** 响应拥有者生命归零，强制中断制造。 */
	void HandleOwnerOutOfHealth(AActor* EffectInstigator, AActor* EffectCauser, const FGameplayEffectSpec* EffectSpec, float EffectMagnitude, float OldValue, float NewValue);

	/** 按角色身上库存组件的注册顺序收集材料来源。 */
	void CollectOwnerInventoryComponentsInRegistrationOrder(TArray<UAOInventoryComponent*>& OutInventoryComponents) const;
	/** 绑定当前材料来源的库存变化监听。 */
	void BindObservedInventorySources();
	/** 解绑当前材料来源的库存变化监听。 */
	void UnbindObservedInventorySources();
	/** 库存变化后刷新制造观察数据。 */
	void HandleObservedInventoryChanged();

private:
	/** 复制给拥有者的制造队列权威数据。 */
	UPROPERTY(ReplicatedUsing = OnRep_CraftingQueue)
	TArray<FAOCraftingQueueEntry> CraftingQueue;

	/**
	 * 队列区域的固定槽位数量。
	 * 这里的 5 表示最多 5 条批量制造队列项，不是最多 5 个成品。
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Crafting")
	int32 MaxQueueSize = 5;

	/** 产物无法放回库存时，掉落到地上的存活时长。 */
	UPROPERTY(EditDefaultsOnly, Category = "Crafting")
	float DroppedCraftItemLifeSeconds = 300.0f;

	/** 地面掉落物使用的 Actor 类。 */
	UPROPERTY(EditDefaultsOnly, Category = "Crafting")
	TSubclassOf<AAOItem> DroppedItemActorClass;

	/** 最近一次制作请求结果，主要供拥有者 UI 做反馈显示。 */
	FAOCraftingRequestResult LastCraftRequestResult;

	/** 已绑定的生命属性集，用于监听死亡中断。 */
	TWeakObjectPtr<const UAOHealthAttributeSet> BoundHealthAttributeSet;

	/** 当前参与制造可做性判断的库存来源。 */
	TArray<TWeakObjectPtr<UAOInventoryComponent>> ObservedInventoryComponents;

	/** 当前 Active 单件制作使用的计时器。 */
	FTimerHandle ActiveCraftingTimerHandle;

	/** 队列项自增唯一 Id。 */
	int32 NextQueueEntryId = 1;

	/**
	 * 当前制造源持有的右键菜单主 ViewModel。
	 * 只承接“当前配方右键菜单”的展示快照和动作上下文。
	 */
	UPROPERTY(Transient)
	TObjectPtr<UMVVM_InventoryItemContextMenu> CraftingContextMenuViewModel = nullptr;
};
