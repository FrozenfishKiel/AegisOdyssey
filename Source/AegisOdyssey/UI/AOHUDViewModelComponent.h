#pragma once

#include "CoreMinimal.h"
#include "Components/GameFrameworkInitStateInterface.h"
#include "AOHUDViewModelComponent.generated.h"

class UAOCombatMessageSubsystem;
class UAOCraftingComponent;
class UAOInventoryMessageSubsystem;
class UAOSkillComponent;
class UMVVM_CombatFeedbackFeed;
class UMVVM_CombatResources;
class UMVVM_Crafting;
class UMVVM_ItemHoverTooltip;
class UMVVM_HUD;
class UMVVM_LocalCombatState;
class UMVVM_TargetHealthBarCollection;
struct FAOCombatFeedbackViewData;
struct FAOInventoryAcquisitionMessage;
struct FAOInventoryAcquisitionNotification;
struct FActorInitStateChangedParams;
struct FAOCombatResultMessage;
struct FPlayerMainHUDViewModelParams;

// HUD 侧的统一 ViewModel 桥接组件。
// 它位于“战斗结果消息层”和“HUD / UMG / 蓝图表现层”之间，负责本地过滤、数据整理和 ViewModel 装配。
UCLASS()
class AEGISODYSSEY_API UAOHUDViewModelComponent : public UActorComponent, public IGameFrameworkInitStateInterface
{
	GENERATED_BODY()

public:
	UAOHUDViewModelComponent();

	// 生命周期内建立 ViewModel，并尽早绑定统一战斗消息源。
	virtual void InitializeComponent() override;
	virtual void UninitializeComponent() override;
	virtual void BeginPlay() override;
	virtual void OnRegister() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// 这些接口来自 InitState 流程。
	// 当前主要用于和现有 HUD 初始化流程保持一致，后续如果 HUD 初始化继续拆层，也仍从这里扩展。
	virtual void CheckDefaultInitialization() override;
	virtual void OnActorInitStateChanged(const FActorInitStateChangedParams& Params) override;
	virtual void HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) override;

	// 初始化本组件持有的所有 ViewModel。
	// 当前由 Main HUD 聚合根继续持有多个战斗子 ViewModel，后续再拆更多 HUD 子面板，也仍从这里统一管理。
	void InitializeAllViewModels();

	// 清空 ViewModel 与外部绑定。
	// 这是组件销毁或重新初始化时的总清理入口。
	void ClearAllViewModels();

	// 把玩家、ASC、技能组件等基础观察源交给 HUD ViewModel。
	void SetHUDViewModelParams(const FPlayerMainHUDViewModelParams& PlayerMainHUDViewModelParams);

	UMVVM_HUD* GetHUDMVVM() const { return HUDViewModel; }

	// 以下是 HUD 聚合根对子 ViewModel 的桥接获取入口。
	// 蓝图 / Widget 想按职责直接拿子 ViewModel 时，优先走这些函数。
	UFUNCTION(BlueprintPure, Category = "AO|HUD")
	UMVVM_CombatResources* GetCombatResourcesViewModel() const;

	UFUNCTION(BlueprintPure, Category = "AO|HUD")
	UMVVM_LocalCombatState* GetLocalCombatStateViewModel() const;

	UFUNCTION(BlueprintPure, Category = "AO|HUD")
	UMVVM_CombatFeedbackFeed* GetCombatFeedbackFeedViewModel() const;

	UFUNCTION(BlueprintPure, Category = "AO|HUD")
	UMVVM_TargetHealthBarCollection* GetTargetHealthBarCollectionViewModel() const;

	// 返回 HUD 当前持有的 Crafting ViewModel。
	// 制作 UI 的正式数据源最终都应收口到这一份实例。
	UFUNCTION(BlueprintPure, Category = "AO|HUD")
	UMVVM_Crafting* GetCraftingViewModel() const;

	// 返回 HUD 当前持有的全局物品 Tooltip ViewModel。
	// 所有能稳定解析出 ItemDefinition 的 UI 条目都应复用这一份实例。
	UFUNCTION(BlueprintPure, Category = "AO|HUD")
	UMVVM_ItemHoverTooltip* GetItemHoverTooltipViewModel() const;

private:
	// 技能观察数据桥接。
	void BindSkillObservationSource(UAOSkillComponent* SkillComponent);
	void UnbindSkillObservationSource();
	void HandleSkillObservationChanged();

	// 战斗消息桥接。
	void BindCombatMessageSource();
	void UnbindCombatMessageSource();

	// 物品获得消息桥接。
	void BindInventoryMessageSource();
	void UnbindInventoryMessageSource();

	// 制造观察桥接。
	// 这条链把角色上的 CraftingComponent 绑定进 HUD 持有的 Crafting ViewModel。
	void BindCraftingObservationSource(UAOCraftingComponent* CraftingComponent);
	void UnbindCraftingObservationSource();

	// 本地 HUD 过滤的“观察者锚点”。
	// 当前单人测试里就是本地玩家 Pawn，后续多人也应该继续从这里统一收束，而不是让各个 Widget 自己找。
	AActor* GetLocalPlayerActor() const;

	// 把统一战斗消息整理成本地 HUD 可直接消费的 ViewData。
	// 如果返回 false，表示这条消息与本地 HUD 无关，不应该继续往表现层扩散。
	bool BuildLocalCombatFeedbackViewData(const FAOCombatResultMessage& Message, FAOCombatFeedbackViewData& OutFeedback) const;

	// 收到统一战斗结果后的 HUD 主入口。
	// 这里只做本地过滤和桥接，不做表现设计，也不允许在这里重判战斗真相。
	void HandleCombatResultMessage(const FAOCombatResultMessage& Message);

	bool BuildLocalInventoryAcquisitionNotification(const FAOInventoryAcquisitionMessage& Message, FAOInventoryAcquisitionNotification& OutNotification) const;

	UFUNCTION()
	void HandleInventoryAcquisitionMessage(FAOInventoryAcquisitionMessage Message);
	void ScheduleDefaultInitializationRetry();
	void ClearDefaultInitializationRetry();

private:
	// 当前 HUD 对外暴露的主聚合 ViewModel。
	// 它本身仍然保留兼容字段，但更重要的职责已经变成“持有并暴露子 ViewModel 实例入口”。
	UPROPERTY()
	TObjectPtr<UMVVM_HUD> HUDViewModel = nullptr;

	// 已绑定的世界级战斗消息子系统。
	UPROPERTY()
	TObjectPtr<UAOCombatMessageSubsystem> BoundCombatMessageSubsystem = nullptr;

	UPROPERTY()
	TObjectPtr<UAOInventoryMessageSubsystem> BoundInventoryMessageSubsystem = nullptr;

	// 已绑定的技能观察源及其回调句柄。
	TWeakObjectPtr<UAOSkillComponent> BoundSkillComponent;
	FDelegateHandle SkillObservationChangedHandle;

	TWeakObjectPtr<UAOCraftingComponent> BoundCraftingComponent;
	FTimerHandle DefaultInitializationRetryTimerHandle;
};
