// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AOMVVMViewModelBase.h"
#include "AOCombatFeedbackViewData.h"
#include "AegisOdyssey/UI/Common/Inventory/AOInventoryAcquisitionNotification.h"
#include "AegisOdyssey/Interaction/InteractionOption.h"
#include "AegisOdyssey/SkillSystem/Components/AOSkillComponent.h"
#include "MVVM_HUD.generated.h"

class AAOPlayerController;
class AAOPlayerState;
class UAOAbilitySystem;
class UAbilitySystemComponent;
class UAOSkillComponent;
class UMVVM_CombatFeedbackFeed;
class UMVVM_CombatResources;
class UMVVM_Crafting;
class UMVVM_ItemHoverTooltip;
class UMVVM_LocalCombatState;
class UMVVM_TargetHealthBarCollection;
struct FAOCombatResultMessage;

// HUD 当前激活的一条 ASC 属性监听记录。
// 它只保存“绑定到了哪个属性、拿到了哪个委托句柄”，解绑时统一按这个表回收。
struct FHUDAttributeDelegateBinding
{
	FGameplayAttribute Attribute;
	FDelegateHandle DelegateHandle;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAOInventoryAcquisitionDynamicDelegate, const FAOInventoryAcquisitionNotification&, Notification);

// Main HUD 建立 ViewModel 时所需的外部依赖集合。
// 这些引用本身不是 HUD 数据，而是 HUD 数据后续读取和监听的“观察源”。
USTRUCT()
struct FPlayerMainHUDViewModelParams
{
	GENERATED_BODY()

	// 角色能力系统入口，生命/体力/韧性等属性观察都从这里取。
	UPROPERTY()
	TWeakObjectPtr<UAbilitySystemComponent> ASC;

	// 本地 PlayerController。
	UPROPERTY()
	TWeakObjectPtr<APlayerController> PC;

	// 本地 PlayerState。
	UPROPERTY()
	TWeakObjectPtr<APlayerState> PS;

	// 技能观察源，技能栏数据和冷却读取都走这里。
	UPROPERTY()
	TWeakObjectPtr<UAOSkillComponent> SkillComponent;
};

// Main HUD 的 MVVM ViewModel。
// 它负责把底层 ASC、技能观察和战斗反馈整理成蓝图 / UMG 直接可绑定的数据。
UCLASS()
class AEGISODYSSEY_API UMVVM_HUD : public UAOMVVMViewModelBase
{
	GENERATED_BODY()

public:
	// 技能观察数据变化事件。
	DECLARE_MULTICAST_DELEGATE(FOnSkillObservationDataChanged);

	// 物品获得通知变化事件。
	DECLARE_MULTICAST_DELEGATE(FOnInventoryAcquisitionChanged);

	UMVVM_HUD(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// 注入 HUD 所依赖的观察源。
	// 这一步只是交依赖，不直接代表所有数据已经同步完成。
	void SetPlayerViewModelParams(const FPlayerMainHUDViewModelParams& Params);

	UFUNCTION(BlueprintNativeEvent)
	void OnParamSet();

	UAOAbilitySystem* GetSourceASC() const;
	AAOPlayerController* GetSourcePC() const;
	AAOPlayerState* GetSourcePS() const;
	UAOSkillComponent* GetSourceSkillComponent() const;

	// 以下是 HUD 聚合根对子 ViewModel 的明确实例入口。
	// 后续蓝图应优先从这里拿子 ViewModel，而不是继续把所有战斗字段都直接挂在 UMVVM_HUD 自己身上。
	UFUNCTION(BlueprintPure, Category = "AO|HUD")
	UMVVM_CombatResources* GetCombatResourcesViewModel() const { return CombatResourcesViewModel; }

	UFUNCTION(BlueprintPure, Category = "AO|HUD")
	UMVVM_LocalCombatState* GetLocalCombatStateViewModel() const { return LocalCombatStateViewModel; }

	UFUNCTION(BlueprintPure, Category = "AO|HUD")
	UMVVM_CombatFeedbackFeed* GetCombatFeedbackFeedViewModel() const { return CombatFeedbackFeedViewModel; }

	UFUNCTION(BlueprintPure, Category = "AO|HUD")
	UMVVM_TargetHealthBarCollection* GetTargetHealthBarCollectionViewModel() const { return TargetHealthBarCollectionViewModel; }

	UFUNCTION(BlueprintPure, Category = "AO|HUD")
	UMVVM_Crafting* GetCraftingViewModel() const { return CraftingViewModel; }

	UFUNCTION(BlueprintPure, Category = "AO|HUD")
	UMVVM_ItemHoverTooltip* GetItemHoverTooltipViewModel() const { return ItemHoverTooltipViewModel; }

public:
	// 以下是一组保留给现有 HUD 绑定的兼容接口。
	// 资源条数值真相已经开始同步给 CombatResources 子 ViewModel，这里后续只作为过渡层保留。
	void SetHealth(float InHealth);

	UFUNCTION(BlueprintPure)
	float GetHealth() const { return Health; }

	void SetMaxHealth(float InMaxHealth);

	UFUNCTION(BlueprintPure)
	float GetMaxHealth() const { return MaxHealth; }

	// 生命百分比，适合血条直接绑定。
	UFUNCTION(BlueprintPure, FieldNotify, Category = "AO|ViewModel")
	float GetHealthPercent() const;

public:
	void SetMaxVigor(float InMaxVigor);

	UFUNCTION(BlueprintPure)
	float GetMaxVigor() const { return MaxVigor; }

	// 体力百分比，适合体力条直接绑定。
	UFUNCTION(BlueprintPure, FieldNotify, Category = "AO|ViewModel")
	float GetVigorPercent() const;

	UFUNCTION(BlueprintPure)
	float GetVigor() const { return Vigor; }

	void SetVigor(float InVigor);

public:
	UFUNCTION(BlueprintPure, FieldNotify, Category = "AO|ViewModel")
	float GetStamina() const { return Stamina; }

	void SetStamina(float InStamina);

	// 韧性百分比，适合韧性条直接绑定。
	UFUNCTION(BlueprintPure, FieldNotify, Category = "AO|ViewModel")
	float GetStaminaPercent() const;

	UFUNCTION(BlueprintPure, FieldNotify, Category = "AO|ViewModel")
	float GetMaxStamina() const { return MaxStamina; }

	void SetMaxStamina(float InMaxStamina);

public:
	// 交互入口观察数据。
	void SetInteractionOptions(const TArray<FInteractionOption>& InInteractionOptions);

	UFUNCTION(BlueprintPure, FieldNotify, Category = "AO|Interaction")
	const TArray<FInteractionOption>& GetInteractionOptions() const { return InteractionOptions; }

	UFUNCTION(BlueprintPure, FieldNotify, Category = "AO|Interaction")
	FInteractionOption GetPrimaryInteractionOption() const;

public:
	// 技能观察数据刷新入口。
	// 当技能组件触发变化时，桥接层会调用这里把最新槽位和装备数据重新同步进 ViewModel。
	UFUNCTION(BlueprintCallable, Category = "AO|Skill")
	void RefreshSkillObservationData();

	void SetSkillObservationData(
		const TArray<FAOSkillSlotViewData>& InSkillSlotViewDataList,
		const TArray<FAOEquippedSkillViewData>& InEquippedSkillViewDataList);

	UFUNCTION(BlueprintPure, FieldNotify, Category = "AO|Skill")
	TArray<FAOSkillSlotViewData> GetSkillSlotViewDataList() const { return SkillSlotViewDataList; }

	UFUNCTION(BlueprintPure, FieldNotify, Category = "AO|Skill")
	TArray<FAOEquippedSkillViewData> GetEquippedSkillViewDataList() const { return EquippedSkillViewDataList; }

	UFUNCTION(BlueprintPure, Category = "AO|Skill|Cooldown")
	bool GetSkillSlotCooldownState(int32 SlotIndex, float& OutTimeRemaining, float& OutTotalDuration) const;

	UFUNCTION(BlueprintPure, Category = "AO|Skill|Cooldown")
	float GetSkillSlotCooldownRemaining(int32 SlotIndex) const;

	UFUNCTION(BlueprintPure, Category = "AO|Skill|Cooldown")
	bool IsSkillSlotOnCooldown(int32 SlotIndex) const;

	FOnSkillObservationDataChanged OnSkillObservationDataChanged;
	FOnInventoryAcquisitionChanged OnInventoryAcquisitionChanged;

	UPROPERTY(BlueprintAssignable, Category = "AO|Inventory")
	FAOInventoryAcquisitionDynamicDelegate OnInventoryAcquisitionReceived;

public:
	// 当桥接层已经完成本地过滤与路由语义整理后，直接从这里进入 HUD ViewModel。
	// 战斗反馈本身只进入 CombatFeedbackFeed 子 ViewModel；
	// HUD 聚合根这里只负责补齐序号并刷新需要的本地战斗状态镜像。
	void ApplyCombatFeedbackViewData(const FAOCombatFeedbackViewData& FeedbackViewData);

	void ApplyInventoryAcquisitionNotification(const FAOInventoryAcquisitionNotification& Notification);

	UFUNCTION(BlueprintPure, FieldNotify, Category = "AO|Inventory")
	const FAOInventoryAcquisitionNotification& GetLatestInventoryAcquisition() const { return LatestInventoryAcquisition; }

	UFUNCTION(BlueprintPure, FieldNotify, Category = "AO|Inventory")
	TArray<FAOInventoryAcquisitionNotification> GetPendingInventoryAcquisitionList() const { return PendingInventoryAcquisitionList; }

	UFUNCTION(BlueprintCallable, Category = "AO|Inventory")
	TArray<FAOInventoryAcquisitionNotification> ConsumePendingInventoryAcquisitionList();

	UFUNCTION(BlueprintCallable, Category = "AO|Inventory")
	void ClearPendingInventoryAcquisitionList();

public:
	UFUNCTION()
	void OnRep_Health();

	UFUNCTION()
	void OnRep_MaxHealth();

	UFUNCTION()
	void OnRep_MaxVigor();

	UFUNCTION()
	void OnRep_Vigor();

	UFUNCTION()
	void OnRep_Stamina();

	UFUNCTION()
	void OnRep_MaxStamina();

protected:
	// 初始化 HUD 聚合根下的子 ViewModel。
	// 后续再拆更多战斗 UI 数据块时，也应从这里继续扩，而不是在外部零散创建。
	void EnsureChildViewModels();

	// 把 HUD 常驻资源条需要的 ASC 属性监听统一挂载到当前观察源上。
	// 这个绑定必须支持切源重绑和重复初始化去重，否则 HUD 初始化链多次探测时会积累重复回调。
	void BindAttributeDelegates();

	// 解绑上一份 ASC 属性监听。
	// HUD 或观察源切换时必须先清理，避免旧 ASC 继续驱动已经过时的 ViewModel。
	void UnbindAttributeDelegates();

	// 注册一条通用 HUD 属性监听。
	// 后续新增 HUD 资源显示时，优先只补“属性 -> Setter”映射，不要再扩散出新的专用绑定成员。
	void RegisterAttributeDelegate(
		UAbilitySystemComponent* SourceASC,
		const FGameplayAttribute& Attribute,
		const TFunction<void(float)>& ApplyValue);

public:
	// 显式初始化 HUD 聚合根下的子 ViewModel。
	// 不在 UObject 构造阶段创建子对象，避免 Outer 仍在构造中时触发运行时断言。
	void InitializeChildViewModels();

	// 根据当前观察源快照刷新本地玩家的战斗状态 ViewModel。
	// 这里统一把 ASC 标签和最近一次本地相关结果翻译成 UI 可直接绑定的状态字段。
	void RefreshLocalCombatStateFromSource();

private:
	// 以下是常驻 HUD 数值状态。
	UPROPERTY(ReplicatedUsing = OnRep_Health, BlueprintReadOnly, FieldNotify, Getter, meta = (AllowPrivateAccess))
	float Health = 0.0f;

	UPROPERTY(ReplicatedUsing = OnRep_MaxHealth, BlueprintReadOnly, FieldNotify, Getter, meta = (AllowPrivateAccess))
	float MaxHealth = 0.0f;

	UPROPERTY(ReplicatedUsing = OnRep_MaxVigor, BlueprintReadOnly, FieldNotify, Getter, meta = (AllowPrivateAccess))
	float MaxVigor = 0.0f;

	UPROPERTY(ReplicatedUsing = OnRep_Vigor, BlueprintReadOnly, FieldNotify, Getter, meta = (AllowPrivateAccess))
	float Vigor = 0.0f;

	UPROPERTY(ReplicatedUsing = OnRep_Stamina, BlueprintReadOnly, FieldNotify, Getter, meta = (AllowPrivateAccess))
	float Stamina = 0.0f;

	UPROPERTY(ReplicatedUsing = OnRep_MaxStamina, BlueprintReadOnly, FieldNotify, Getter, meta = (AllowPrivateAccess))
	float MaxStamina = 0.0f;

	UPROPERTY(BlueprintReadOnly, FieldNotify, meta = (AllowPrivateAccess))
	TArray<FInteractionOption> InteractionOptions;

	// 技能栏槽位观察快照。
	UPROPERTY(BlueprintReadOnly, FieldNotify, meta = (AllowPrivateAccess))
	TArray<FAOSkillSlotViewData> SkillSlotViewDataList;

	// 已装备技能观察快照。
	UPROPERTY(BlueprintReadOnly, FieldNotify, meta = (AllowPrivateAccess))
	TArray<FAOEquippedSkillViewData> EquippedSkillViewDataList;

	UPROPERTY(BlueprintReadOnly, FieldNotify, meta = (AllowPrivateAccess))
	FAOInventoryAcquisitionNotification LatestInventoryAcquisition;

	UPROPERTY(BlueprintReadOnly, FieldNotify, meta = (AllowPrivateAccess))
	TArray<FAOInventoryAcquisitionNotification> PendingInventoryAcquisitionList;

	// 本地玩家战斗资源子 ViewModel。
	UPROPERTY(Transient)
	TObjectPtr<UMVVM_CombatResources> CombatResourcesViewModel = nullptr;

	// 本地玩家战斗状态子 ViewModel。
	UPROPERTY(Transient)
	TObjectPtr<UMVVM_LocalCombatState> LocalCombatStateViewModel = nullptr;

	// 本地战斗反馈流子 ViewModel。
	UPROPERTY(Transient)
	TObjectPtr<UMVVM_CombatFeedbackFeed> CombatFeedbackFeedViewModel = nullptr;

	// 本地目标血条观察集合子 ViewModel。
	UPROPERTY(Transient)
	TObjectPtr<UMVVM_TargetHealthBarCollection> TargetHealthBarCollectionViewModel = nullptr;

	// 本地角色制造观察子 ViewModel。
	UPROPERTY(Transient)
	TObjectPtr<UMVVM_Crafting> CraftingViewModel = nullptr;

	// HUD 全局唯一的物品悬浮信息框子 ViewModel。
	UPROPERTY(Transient)
	TObjectPtr<UMVVM_ItemHoverTooltip> ItemHoverTooltipViewModel = nullptr;

	// HUD 观察源参数缓存。
	FPlayerMainHUDViewModelParams PlayerViewModelParams;

	// 本地反馈自增序号。
	int32 CombatFeedbackSequence = 0;
	int32 InventoryAcquisitionSequence = 0;

	// 当前已绑定属性监听的 ASC。
	TWeakObjectPtr<UAbilitySystemComponent> BoundAttributeASC;

	// 当前激活的 ASC 属性监听表。
	// 解绑时统一遍历这个表，不关心具体绑定了多少条属性，也不要求头文件跟着每次新增属性一起膨胀。
	TArray<FHUDAttributeDelegateBinding> ActiveAttributeBindings;

private:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool IsSupportedForNetworking() const override { return true; }
};
