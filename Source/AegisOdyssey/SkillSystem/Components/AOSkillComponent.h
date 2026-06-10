// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AegisOdyssey/AbilitySystem/AOAbilitySet.h"
#include "AegisOdyssey/Inventory/AOInventoryComponent.h"
#include "Components/GameFrameworkInitStateInterface.h"
#include "Components/PawnComponent.h"
#include "GameplayAbilitySpecHandle.h"
#include "GameplayTagContainer.h"
#include "AOSkillComponent.generated.h"

class AActor;
class UAOAbilitySystem;
class UAOHeroComponent;
class UAOInventoryItemInstance;
class UAOSkillDefinition;
class UAOSkillInstance;
class UTexture2D;
enum EInputType : uint8;

/**
 * 单个技能槽的运行时条目。
 *
 * 这一层只表达“槽位当前装了谁、它对应哪个输入、它在 ASC 上当前授予了什么结果”。
 * 它不是技能定义，也不是技能实例本体，而是“槽位和实例之间的装配关系对象”。
 */
USTRUCT(BlueprintType)
struct FAOSkillSlotEntry
{
	GENERATED_BODY()

public:
	FAOSkillSlotEntry() = default;

	explicit FAOSkillSlotEntry(int32 InSlotIndex)
		: SlotIndex(InSlotIndex)
	{
	}

	// 这是第几个技能槽。
	UPROPERTY(BlueprintReadOnly, Category = "Skill")
	int32 SlotIndex = INDEX_NONE;

	// 该槽位对应的输入标签。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill", meta = (Categories = "InputTag"))
	FGameplayTag InputTag;

	// 当前装在这个槽里的运行时技能实例。
	UPROPERTY(BlueprintReadOnly, Category = "Skill")
	TObjectPtr<UAOSkillInstance> SkillInstance = nullptr;

	// 当前槽位在 ASC 上授予出去的主能力句柄。
	UPROPERTY(BlueprintReadOnly, Category = "Skill")
	FGameplayAbilitySpecHandle GrantedAbilitySpecHandle;

	// 当前槽位通过 AbilitySet 一并授予出去的其它结果句柄集合。
	UPROPERTY(BlueprintReadOnly, Category = "Skill")
	FAOAbilitySet_GrantedHandles GrantedHandles;

	void ResetRuntimeBinding()
	{
		SkillInstance = nullptr;
		GrantedAbilitySpecHandle = FGameplayAbilitySpecHandle();
		GrantedHandles = FAOAbilitySet_GrantedHandles();
	}
};

/**
 * 单个技能槽对 UI 暴露的观察快照。
 *
 * 这是纯展示模型，专门给 HUD / MVVM / 蓝图观察使用。
 * 真正的运行时真相仍然只存在于 SkillComponent / SkillInstance / ASC 中。
 */
USTRUCT(BlueprintType)
struct FAOSkillSlotViewData
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, Category = "Skill")
	int32 SlotIndex = INDEX_NONE;

	UPROPERTY(BlueprintReadOnly, Category = "Skill")
	FGameplayTag InputTag;

	UPROPERTY(BlueprintReadOnly, Category = "Skill")
	bool bHasSkill = false;

	UPROPERTY(BlueprintReadOnly, Category = "Skill")
	TObjectPtr<UAOSkillInstance> SkillInstance = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Skill")
	TObjectPtr<UAOSkillDefinition> SkillDefinition = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Skill")
	TObjectPtr<UAOInventoryItemInstance> SourceItemInstance = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Skill")
	FText SkillName;

	UPROPERTY(BlueprintReadOnly, Category = "Skill")
	FText SkillDescription;

	UPROPERTY(BlueprintReadOnly, Category = "Skill")
	TObjectPtr<UTexture2D> SkillIcon = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Skill")
	FText SourceItemDisplayName;

	UPROPERTY(BlueprintReadOnly, Category = "Skill")
	int32 SkillLevel = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Skill")
	int32 SkillQuality = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Skill|Cooldown")
	bool bOnCooldown = false;

	UPROPERTY(BlueprintReadOnly, Category = "Skill|Cooldown")
	float CooldownRemaining = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Skill|Cooldown")
	float CooldownTotalDuration = 0.0f;
};

/**
 * 已装配技能对 UI 暴露的观察快照。
 */
USTRUCT(BlueprintType)
struct FAOEquippedSkillViewData
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, Category = "Skill")
	TObjectPtr<UAOSkillInstance> SkillInstance = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Skill")
	TObjectPtr<UAOSkillDefinition> SkillDefinition = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Skill")
	TObjectPtr<UAOInventoryItemInstance> SourceItemInstance = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Skill")
	int32 SlotIndex = INDEX_NONE;

	UPROPERTY(BlueprintReadOnly, Category = "Skill")
	FText SkillName;

	UPROPERTY(BlueprintReadOnly, Category = "Skill")
	FText SkillDescription;

	UPROPERTY(BlueprintReadOnly, Category = "Skill")
	TObjectPtr<UTexture2D> SkillIcon = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Skill")
	FText SourceItemDisplayName;

	UPROPERTY(BlueprintReadOnly, Category = "Skill")
	int32 SkillLevel = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Skill")
	int32 SkillQuality = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Skill|Cooldown")
	bool bOnCooldown = false;

	UPROPERTY(BlueprintReadOnly, Category = "Skill|Cooldown")
	float CooldownRemaining = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Skill|Cooldown")
	float CooldownTotalDuration = 0.0f;
};

/**
 * 角色技能系统的运行时总入口。
 *
 * 它负责：
 * 1. 承载固定数量的技能槽
 * 2. 创建并维护 SkillInstance
 * 3. 维护槽位与实例的装配关系
 * 4. 把槽位变化翻译成 ASC 授予变化
 * 5. 订阅 Hero 输入广播，但不把技能语义判断塞回 Hero
 * 6. 对 HUD / MVVM 输出只读观察快照
 */
UCLASS(Blueprintable, meta = (BlueprintSpawnableComponent))
class AEGISODYSSEY_API UAOSkillComponent : public UPawnComponent, public IGameFrameworkInitStateInterface
{
	GENERATED_BODY()

public:
	DECLARE_MULTICAST_DELEGATE(FOnSkillObservationChanged);

	static const FName NAME_ActorFeatureName;

	UAOSkillComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual FName GetFeatureName() const override { return NAME_ActorFeatureName; }

	static UAOSkillComponent* FindSkillComponent(const AActor* Actor)
	{
		return Actor ? Actor->FindComponentByClass<UAOSkillComponent>() : nullptr;
	}

	UFUNCTION(BlueprintPure, Category = "Skill")
	bool IsValidSkillSlotIndex(int32 SlotIndex) const;

	UFUNCTION(BlueprintPure, Category = "Skill")
	int32 GetNumSkillSlots() const { return NumSkillSlots; }

	UFUNCTION(BlueprintCallable, Category = "Skill", BlueprintAuthorityOnly)
	UAOSkillInstance* CreateSkillInstance(UAOSkillDefinition* SkillDefinition, UAOInventoryItemInstance* SourceItemInstance = nullptr);

	UFUNCTION(BlueprintCallable, Category = "Skill", BlueprintAuthorityOnly)
	bool EquipSkillInstanceToSlot(UAOSkillInstance* SkillInstance, int32 SlotIndex);

	UFUNCTION(BlueprintCallable, Category = "Skill", BlueprintAuthorityOnly)
	bool UnequipSkillFromSlot(int32 SlotIndex);

	UFUNCTION(BlueprintCallable, Category = "Skill", BlueprintAuthorityOnly)
	UAOSkillInstance* GetOrCreateSkillInstanceFromSourceItem(UAOInventoryItemInstance* SourceItemInstance);

	UFUNCTION(BlueprintCallable, Category = "Skill", BlueprintAuthorityOnly)
	bool EquipSkillSourceItemToSlot(UAOInventoryItemInstance* SourceItemInstance, int32 SlotIndex);

	UFUNCTION(BlueprintCallable, Category = "Skill", BlueprintAuthorityOnly)
	bool RemoveSkillInstanceForSourceItem(UAOInventoryItemInstance* SourceItemInstance);

	// 技能槽库存适配层把“整份槽位投影结果”交给这里，
	// SkillComponent 再把它翻译成真正的技能运行时真相。
	void SyncSkillSlotsFromInventoryProjection(const TArray<FAOInventoryEntry>& Slots);

	bool CanEquipSkillInstanceToSlot(const UAOSkillInstance* SkillInstance, int32 SlotIndex, bool bAllowReplace = true) const;
	bool IsSkillInstanceEquipped(const UAOSkillInstance* SkillInstance) const;
	UAOSkillInstance* FindSkillInstanceBySourceItem(const UAOInventoryItemInstance* SourceItemInstance) const;
	UAOSkillDefinition* FindSkillDefinitionFromSourceItem(const UAOInventoryItemInstance* SourceItemInstance) const;

	UFUNCTION(BlueprintPure, Category = "Skill|Source")
	bool IsSkillSourceItem(const UAOInventoryItemInstance* SourceItemInstance) const;

	UFUNCTION(BlueprintPure, Category = "Skill|Source")
	bool CanAcceptSourceItemForSkillSlot(const UAOInventoryItemInstance* SourceItemInstance, int32 SlotIndex) const;

	UFUNCTION(BlueprintCallable, Category = "Skill|Source")
	bool RequestEquipSourceItemToSlot(UAOInventoryItemInstance* SourceItemInstance, int32 SlotIndex);

	const TArray<FAOSkillSlotEntry>& GetSkillSlots() const { return SkillSlots; }
	const TArray<TObjectPtr<UAOSkillInstance>>& GetEquippedSkillInstances() const { return EquippedSkillInstances; }

	UFUNCTION(BlueprintPure, Category = "Skill|View")
	TArray<FAOSkillSlotViewData> GetSkillSlotViewDataList() const;

	UFUNCTION(BlueprintPure, Category = "Skill|View")
	bool GetSkillSlotViewData(int32 SlotIndex, FAOSkillSlotViewData& OutViewData) const;

	UFUNCTION(BlueprintPure, Category = "Skill|View")
	TArray<FAOEquippedSkillViewData> GetEquippedSkillViewDataList() const;

	UFUNCTION(BlueprintPure, Category = "Skill|View")
	bool GetEquippedSkillViewData(const UAOSkillInstance* SkillInstance, FAOEquippedSkillViewData& OutViewData) const;

	UFUNCTION(BlueprintPure, Category = "Skill")
	bool IsSkillSlotInputTag(const FGameplayTag& InputTag) const;

	UFUNCTION(BlueprintPure, Category = "Skill")
	int32 FindSkillSlotIndexByInputTag(const FGameplayTag& InputTag) const;

	// 这条链仍然走 Hero 的统一输入入口，供真实输入链复用。
	UFUNCTION(BlueprintCallable, Category = "Skill")
	bool InjectSkillSlotInputCommand(FGameplayTag InputTag, TEnumAsByte<EInputType> InputType) const;

	UFUNCTION(BlueprintCallable, Category = "Skill")
	bool InjectSkillSlotInputCommandByIndex(int32 SlotIndex, TEnumAsByte<EInputType> InputType) const;

	// 这条链直接落到 SkillComponent -> ASC，不再回灌 Hero 输入广播。
	UFUNCTION(BlueprintCallable, Category = "Skill")
	bool ExecuteSkillSlotCommand(FGameplayTag InputTag, TEnumAsByte<EInputType> InputType) const;

	UFUNCTION(BlueprintCallable, Category = "Skill")
	bool ExecuteSkillSlotCommandByIndex(int32 SlotIndex, TEnumAsByte<EInputType> InputType) const;

	UFUNCTION(BlueprintPure, Category = "Skill|Cooldown")
	bool GetSkillInstanceCooldownState(const UAOSkillInstance* SkillInstance, float& OutTimeRemaining, float& OutTotalDuration) const;

	UFUNCTION(BlueprintPure, Category = "Skill|Cooldown")
	bool GetSkillSlotCooldownState(int32 SlotIndex, float& OutTimeRemaining, float& OutTotalDuration) const;

	UFUNCTION(BlueprintPure, Category = "Skill|Cooldown")
	bool IsSkillInstanceOnCooldown(const UAOSkillInstance* SkillInstance) const;

	UFUNCTION(BlueprintPure, Category = "Skill|Cooldown")
	bool IsSkillSlotOnCooldown(int32 SlotIndex) const;

	UFUNCTION(BlueprintPure, Category = "Skill|Cooldown")
	float GetSkillInstanceCooldownRemaining(const UAOSkillInstance* SkillInstance) const;

	UFUNCTION(BlueprintPure, Category = "Skill|Cooldown")
	float GetSkillSlotCooldownRemaining(int32 SlotIndex) const;

	FOnSkillObservationChanged OnSkillObservationChanged;

protected:
	virtual void OnActorInitStateChanged(const FActorInitStateChangedParams& Params) override;
	virtual void HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) override;
	virtual void CheckDefaultInitialization() override;
	virtual void OnRegister() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void ReadyForReplication() override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void OnRep_SkillSlots();

	UFUNCTION()
	void OnRep_EquippedSkillInstances();

	UFUNCTION(Server, Reliable)
	void ServerRequestEquipSourceItemToSlot(UAOInventoryItemInstance* SourceItemInstance, int32 SlotIndex);

private:
	void InitializeParams();
	void InitializeOrRefreshSkillSlots();
	void BindHeroInputDelegates();
	void UnbindHeroInputDelegates();
	void HandleHeroInputPressed(FGameplayTag InputTag, EInputType InputType);
	void HandleHeroInputStarted(FGameplayTag InputTag, EInputType InputType);
	void HandleHeroInputReleased(FGameplayTag InputTag, EInputType InputType);
	void HandleObservedHeroInput(FGameplayTag InputTag, EInputType InputType);
	void RegisterSkillInstanceSubObject(UAOSkillInstance* SkillInstance);
	void UnregisterSkillInstanceSubObject(UAOSkillInstance* SkillInstance);
	void RemoveSkillInstanceFromEquippedSet(UAOSkillInstance* SkillInstance);
	bool DestroySkillInstanceIfUnused(UAOSkillInstance* SkillInstance);
	UAOAbilitySystem* GetAbilitySystemComponent() const;
	bool QueryCooldownStateFromTags(const FGameplayTagContainer& CooldownTags, float& OutTimeRemaining, float& OutTotalDuration) const;
	void NotifySkillObservationChanged();
	bool BuildSkillSlotViewData(const FAOSkillSlotEntry& SlotEntry, FAOSkillSlotViewData& OutViewData) const;
	bool BuildEquippedSkillViewData(const UAOSkillInstance* SkillInstance, FAOEquippedSkillViewData& OutViewData) const;
	void ResolveSkillDisplayData(const UAOSkillInstance* SkillInstance, FText& OutSkillName, FText& OutSkillDescription, TObjectPtr<UTexture2D>& OutSkillIcon, FText& OutSourceItemDisplayName) const;
	bool GrantSkillToSlot(FAOSkillSlotEntry& SlotEntry);
	void RevokeSkillFromSlot(FAOSkillSlotEntry& SlotEntry);
	bool RefreshGrantedAbilityForSlot(int32 SlotIndex);

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill", meta = (AllowPrivateAccess = true, ClampMin = "1"))
	int32 NumSkillSlots = 4;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill", meta = (AllowPrivateAccess = true, Categories = "InputTag"))
	TArray<FGameplayTag> DefaultSlotInputTags;

	UPROPERTY(ReplicatedUsing = OnRep_SkillSlots)
	TArray<FAOSkillSlotEntry> SkillSlots;

	UPROPERTY(ReplicatedUsing = OnRep_EquippedSkillInstances)
	TArray<TObjectPtr<UAOSkillInstance>> EquippedSkillInstances;

	TWeakObjectPtr<UAOHeroComponent> BoundHeroComponent;
	FDelegateHandle OnHeroInputPressedHandle;
	FDelegateHandle OnHeroInputStartedHandle;
	FDelegateHandle OnHeroInputReleasedHandle;
};
