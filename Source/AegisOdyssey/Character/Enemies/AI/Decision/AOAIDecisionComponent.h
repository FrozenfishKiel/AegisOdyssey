// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "AegisOdyssey/Character/Enemies/AI/Decision/AOAIDecisionTypes.h"
#include "AegisOdyssey/Character/Enemies/AI/Decision/Inventory/AOAIInventoryDecisionTypes.h"
#include "Components/PawnComponent.h"
#include "AOAIDecisionComponent.generated.h"

class AActor;
class UAbilitySystemComponent;
class UAOAIDecisionProfile;
class UAOHealthAttributeSet;
struct FGameplayEffectSpec;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnAISubmittedInventoryDecisionChanged, const FAOAIInventoryDecisionResult&);

USTRUCT(BlueprintType)
struct FAOAIDecisionDebugSnapshot
{
	GENERATED_BODY()

	// 当前调试面板是否已经锁定到一个 AI 决策组件。
	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	bool bIsTrackingAI = false;

	// 当前被观察 AI 的显示名。
	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	FName TrackedActorName = NAME_None;

	// 当前统一决策队列中的条目数量。
	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	int32 DecisionQueueCount = 0;
	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	FGameplayTag SelectedIntentTag;
	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	bool bHasCurrentEvaluationInventoryDecision = false;
	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	FGameplayTag CurrentEvaluationInventoryActionTag;

	// 当前队首待提交决策标签。
	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	FGameplayTag CurrentQueuedDecisionTag;
	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	FGameplayTag CurrentSubmittedDecisionTag;
	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	FGameplayTag LastSubmittedDecisionTag;

	// 当前是否存在已经正式提交给执行层的库存决策结果。
	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	bool bHasCurrentSubmittedInventoryDecision = false;

	// 当前已经正式提交给执行层的库存决策结果快照。
	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	FAOAIInventoryDecisionResult CurrentSubmittedInventoryDecision;
	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	float PendingSubmitDelaySeconds = -1.0f;
};

USTRUCT(BlueprintType)
struct FAOAIDecisionIntentRuntimeState
{
	GENERATED_BODY()

	// 该意图本帧评估得到的欲望值。
	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	float Desire = 0.0f;

	// 该意图本帧评估得到的最终分数。
	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	float Score = 0.0f;

	// 该意图上一次正式进入执行链的时间。
	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	float LastExecutedTime = -1.0f;
};

USTRUCT()
struct FAOAIDecisionRecentDamageEntry
{
	GENERATED_BODY()

	// 这次受击记录发生时的世界时间。
	UPROPERTY()
	float WorldTimeSeconds = -1.0f;

	// 这次受击记录对应的伤害量。
	UPROPERTY()
	float DamageAmount = 0.0f;
};

UCLASS(Blueprintable, Meta = (BlueprintSpawnableComponent))
class AEGISODYSSEY_API UAOAIDecisionComponent : public UPawnComponent
{
	GENERATED_BODY()

public:
	// 创建 AI 决策组件，并初始化运行时缓存。
	UAOAIDecisionComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// 从 Actor 或 Controller 反查其实际使用的 AI 决策组件。
	UFUNCTION(BlueprintPure, Category = "AO|AI|Decision")
	static UAOAIDecisionComponent* FindAIDecisionComponent(const AActor* Actor);

	// 确保当前配置已经从 DecisionProfile 同步到组件缓存。
	void EnsureDecisionDefinitionsInitialized();

	// 读取当前战斗意图定义列表，供战斗 Evaluator 自行评估。
	const TArray<FAOAIDecisionIntentDefinition>& GetIntentDefinitions();

	// 读取当前库存动作定义列表，供库存 Evaluator 自行评估。
	const TArray<FAOAIInventoryDesireDefinition>& GetInventoryDesireDefinitions();

	// 读取当前缓存的战斗意图运行时状态。
	const TMap<FGameplayTag, FAOAIDecisionIntentRuntimeState>& GetIntentRuntimeStates() const { return IntentRuntimeStates; }

	// 读取当前缓存的库存动作运行时状态。
	const TMap<FGameplayTag, FAOAIInventoryDecisionRuntimeState>& GetInventoryRuntimeStates() const { return InventoryRuntimeStates; }

	// 刷新组件内部依赖的观察上下文，例如 ASC 缓存和 RecentDamage 统计。
	void RefreshObservationContext();

	// 读取指定 Actor 的 AI 攻击距离，供 Evaluator 自行收集事实。
	float ResolveAIAttackRangeFromActor(const AActor* Actor, float DefaultRange = 0.0f) const;

	// 检查指定 Actor 是否命中目标标签。
	bool HasMatchingTag(const AActor* Actor, const FGameplayTag& Tag, bool bExactMatch) const;

	// 读取当前缓存 ASC 上的属性值。
	float GetAttributeValue(const FGameplayAttribute& Attribute) const;

	// 按给定时间窗口读取 RecentDamage 比例。
	float GetRecentDamageRatio(float CurrentWorldTimeSeconds, float WindowSeconds) const;

	// 读取当前缓存的战斗事实。
	const FAOAIDecisionCombatFacts& GetCombatFacts() const { return CombatFacts; }

	// 缓存战斗 Evaluator 计算出的事实、意图评分和战术态。
	void CacheCombatEvaluation(
		float CurrentWorldTimeSeconds,
		const FAOAIDecisionCombatFacts& InCombatFacts,
		const TMap<FGameplayTag, FAOAIDecisionIntentRuntimeState>& InIntentRuntimeStates,
		const FGameplayTag& InSelectedIntentTag,
		const FAOAIDecisionTacticalState& InTacticalState);

	// 缓存库存 Evaluator 计算出的事实、候选汇总和最新评估结果。
	void CacheInventoryEvaluation(
		float CurrentWorldTimeSeconds,
		const FAOAIInventoryDecisionFacts& InInventoryDecisionFacts,
		const TMap<FGameplayTag, FAOAIInventoryDecisionRuntimeState>& InInventoryRuntimeStates,
		const FAOAIInventoryDecisionResult& InEvaluationInventoryDecisionResult);

	// 回写一个已经进入执行链的主意图，用于节奏与重复惩罚统计。
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Decision")
	bool CommitExecutedIntent(FGameplayTag ExecutedIntentTag, float CurrentWorldTimeSeconds = -1.0f);

	// 回写一个已经正式执行的库存动作，用于库存动作节奏与重复统计。
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Decision")
	bool CommitExecutedInventoryAction(const FAOAIInventoryDecisionResult& ExecutedDecisionResult, float CurrentWorldTimeSeconds = -1.0f);

	// 重置整个决策组件的运行时状态。
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Decision")
	void ResetDecisionState();

	// 读取最近一次正式执行的主意图标签。
	UFUNCTION(BlueprintPure, Category = "AO|AI|Decision")
	FGameplayTag GetLastExecutedIntentTag() const { return LastExecutedIntentTag; }

	// 读取当前连续重复执行同一主意图的次数。
	UFUNCTION(BlueprintPure, Category = "AO|AI|Decision")
	int32 GetRepeatedIntentCount() const { return RepeatedIntentCount; }

	// 读取指定意图当前帧的 Desire / Score。
	UFUNCTION(BlueprintPure, Category = "AO|AI|Decision")
	bool GetIntentRuntimeMetrics(FGameplayTag IntentTag, float& OutDesire, float& OutScore) const;

	// 写入待执行动作方向，供后续任务读取。
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Decision")
	void SetPendingActionDirection(FVector InWorldDirection);

	// 清空待执行动作方向。
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Decision")
	void ClearPendingActionDirection();

	// 读取待执行动作方向。
	UFUNCTION(BlueprintPure, Category = "AO|AI|Decision")
	bool GetPendingActionDirection(FVector& OutWorldDirection) const;

	// 以简单标签形式创建并入队一个决策条目。
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Decision|Queue")
	bool EnqueueDecisionTag(FGameplayTag DecisionTag, float CurrentWorldTimeSeconds);

	// 将完整决策条目压入统一提交队列。
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Decision|Queue")
	bool EnqueueDecisionItem(const FAOAIDecisionQueueItem& DecisionItem);

	// 把当前评估结果投影为正式队列条目，并尝试按时间窗提交队首。
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Decision|Queue")
	void SubmitCurrentDecisionOutputs(float CurrentWorldTimeSeconds);

	// 在允许提交时弹出并返回队列头部条目。
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Decision|Queue")
	bool TrySubmitNextDecision(float CurrentWorldTimeSeconds, FAOAIDecisionQueueItem& OutSubmittedDecision);

	// 读取当前队列长度。
	UFUNCTION(BlueprintPure, Category = "AO|AI|Decision|Queue")
	int32 GetDecisionQueueCount() const { return DecisionQueue.Num(); }

	// 当前是否存在待提交标签。
	UFUNCTION(BlueprintPure, Category = "AO|AI|Decision|Queue")
	bool HasPendingDecisionTag() const;

	// 读取当前队首标签。
	UFUNCTION(BlueprintPure, Category = "AO|AI|Decision|Queue")
	FGameplayTag GetCurrentQueuedDecisionTag() const;

	// 读取下一次允许提交的时间点。
	UFUNCTION(BlueprintPure, Category = "AO|AI|Decision|Queue")
	float GetNextQueuedDecisionSubmitTimeSeconds() const { return NextDecisionSubmitTimeSeconds; }

	// 判断当前统一决策主链是否匹配给定标签。
	UFUNCTION(BlueprintPure, Category = "AO|AI|Decision|Queue")
	bool MatchesCurrentDecisionTag(FGameplayTag DecisionTag) const;

	// 读取当前正式提交中的决策标签。
	UFUNCTION(BlueprintPure, Category = "AO|AI|Decision|Queue")
	FGameplayTag GetCurrentSubmittedDecisionTag() const { return CurrentSubmittedDecisionTag; }

	// 读取最近一次正式提交过的决策标签。
	UFUNCTION(BlueprintPure, Category = "AO|AI|Decision|Queue")
	FGameplayTag GetLastSubmittedDecisionTag() const { return LastSubmittedDecisionTag; }

	// 当前是否存在正式提交中的决策标签。
	UFUNCTION(BlueprintPure, Category = "AO|AI|Decision|Queue")
	bool HasCurrentSubmittedDecisionTag() const { return CurrentSubmittedDecisionTag.IsValid(); }

	// 读取当前正式提交中的库存决策结果。
	UFUNCTION(BlueprintPure, Category = "AO|AI|Decision|Queue")
	bool GetCurrentSubmittedInventoryDecisionResult(FAOAIInventoryDecisionResult& OutResult) const;

	// 读取最近一次正式提交过的库存决策结果。
	UFUNCTION(BlueprintPure, Category = "AO|AI|Decision|Queue")
	bool GetLastSubmittedInventoryDecisionResult(FAOAIInventoryDecisionResult& OutResult) const;

	// 读取当前最新的库存评估结果。这是 Evaluator 缓存，不是队列或已提交结果。
	UFUNCTION(BlueprintPure, Category = "AO|AI|InventoryDecision")
	bool GetCurrentEvaluationInventoryDecisionResult(FAOAIInventoryDecisionResult& OutResult) const;

	// 读取当前缓存的库存评估事实。
	UFUNCTION(BlueprintPure, Category = "AO|AI|InventoryDecision")
	const FAOAIInventoryDecisionFacts& GetInventoryDecisionFacts() const { return InventoryDecisionFacts; }

	// 读取库存动作执行记忆。
	UFUNCTION(BlueprintPure, Category = "AO|AI|InventoryDecision")
	const FAOAIInventoryDecisionExecutionRecord& GetInventoryDecisionExecutionRecord() const { return InventoryDecisionExecutionRecord; }

	// 读取当前缓存的战术态。
	UFUNCTION(BlueprintPure, Category = "AO|AI|Decision")
	const FAOAIDecisionTacticalState& GetTacticalState() const { return TacticalState; }

	// 正式提交库存结果变更事件。
	FOnAISubmittedInventoryDecisionChanged& OnSubmittedInventoryDecisionChanged() { return SubmittedInventoryDecisionChangedEvent; }

	// 构建当前 AI 决策组件的调试快照。
	// 这份快照只用于调试观察层，不参与正式执行语义。
	UFUNCTION(BlueprintPure, Category = "AO|AI|Decision|Debug")
	bool BuildDebugSnapshot(FAOAIDecisionDebugSnapshot& OutDebugSnapshot) const;

	// 测试专用：直接写入一份已提交库存决策结果，供 ViewModel / HUD 调试链路验证使用。
	void SetDebugSubmittedInventoryDecisionResultForTests(const FAOAIInventoryDecisionResult& InSubmittedInventoryDecisionResult);

protected:
	// 生命周期开始时绑定受击事件监听。
	void BeginPlay() override;

	// 生命周期结束时解绑受击事件监听。
	void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	// 确保配置定义和运行时状态已经按当前 DecisionProfile 初始化。
	void EnsureIntentDefinitionsInitialized();

	// 按需从 DecisionProfile 同步配置。
	void ApplyDecisionProfileIfNeeded();

	// 按当前意图定义重建运行时状态表。
	void SyncRuntimeStatesWithDefinitions();

	// 按当前库存动作定义重建运行时状态表。
	void SyncInventoryRuntimeStatesWithDefinitions();

	// 判断当前是否处于允许维护统一提交队列的服务端权威路径。
	bool HasDecisionQueueAuthority() const;

	// 根据随机时间窗安排下一次队列提交时刻。
	void ScheduleNextDecisionSubmitTime(float CurrentWorldTimeSeconds);

	// 将当前主意图映射成统一队列中的正式决策标签。
	FGameplayTag BuildCurrentIntentDecisionTag() const;

	// 将当前库存决策映射成统一队列条目。
	bool BuildCurrentInventoryDecisionItem(float CurrentWorldTimeSeconds, FAOAIDecisionQueueItem& OutDecisionItem) const;

	// 当队列条目代表库存动作时，回查其对应的库存决策结果。
	bool TryResolveInventoryDecisionResultForQueuedItem(const FAOAIDecisionQueueItem& DecisionItem, FAOAIInventoryDecisionResult& OutResult) const;

	// 判断两个队列条目是否可视为同一决策，避免重复排队。
	bool IsQueuedDecisionEquivalent(const FAOAIDecisionQueueItem& ExistingItem, const FAOAIDecisionQueueItem& CandidateItem) const;

	// 更新最新的库存评估结果缓存。
	void SetCurrentEvaluationInventoryDecisionResult(const FAOAIInventoryDecisionResult& NewResult);

	// 为库存队列条目保存一份结果快照。
	int32 StoreInventoryDecisionPayload(const FAOAIInventoryDecisionResult& InventoryDecisionResult);

	// 读取指定快照编号对应的库存决策结果。
	bool TryGetInventoryDecisionPayload(int32 PayloadId, FAOAIInventoryDecisionResult& OutResult) const;

	// 清理已经不再被引用的库存快照。
	void CompactInventoryDecisionPayloads();

	// 清空最近受击统计缓存。
	void ResetRecentDamageTracking();

	// 绑定自身生命值变化监听，用于 RecentDamage 统计。
	void BindHealthAttributeEventsIfNeeded();

	// 解绑自身生命值变化监听。
	void UnbindHealthAttributeEvents();

	// 记录受击事件，供 RecentDamage 因子评估使用。
	void HandleSelfHealthChanged(AActor* EffectInstigator, AActor* EffectCauser, const FGameplayEffectSpec* EffectSpec, float EffectMagnitude, float OldValue, float NewValue);

	// 从指定 Actor 上解析 AbilitySystemComponent。
	UAbilitySystemComponent* ResolveAbilitySystemComponentFromActor(const AActor* Actor) const;

	// 计算调试显示所使用的最大 RecentDamage 统计窗口。
	float GetRecentDamageDebugWindowSeconds() const;

	// 清理超出调试窗口的旧受击记录。
	void PruneRecentDamageEntries(float CurrentWorldTimeSeconds);

	// 判断执行侧回写的库存动作是否与当前正式提交结果一致。
	bool DoesInventoryDecisionResultMatchSubmitted(const FAOAIInventoryDecisionResult& ExecutedDecisionResult) const;

private:
	// 当前帧用于决策的战斗事实。
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AO|AI|Decision|Facts", meta = (AllowPrivateAccess = true))
	FAOAIDecisionCombatFacts CombatFacts;

	// 统一决策配置资产。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AO|AI|Decision|Config", meta = (AllowPrivateAccess = true))
	TObjectPtr<const UAOAIDecisionProfile> DecisionProfile = nullptr;

	// 战斗意图定义表。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AO|AI|Decision|Config", meta = (AllowPrivateAccess = true))
	TArray<FAOAIDecisionIntentDefinition> IntentDefinitions;

	// 库存动作定义表。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AO|AI|InventoryDecision|Config", meta = (AllowPrivateAccess = true))
	TArray<FAOAIInventoryDesireDefinition> InventoryDesireDefinitions;

	// 战斗意图运行时状态缓存。
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AO|AI|Decision|Runtime", meta = (AllowPrivateAccess = true))
	TMap<FGameplayTag, FAOAIDecisionIntentRuntimeState> IntentRuntimeStates;

	// 库存动作运行时状态缓存。
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AO|AI|InventoryDecision|Runtime", meta = (AllowPrivateAccess = true))
	TMap<FGameplayTag, FAOAIInventoryDecisionRuntimeState> InventoryRuntimeStates;

	// 当前战术态缓存。
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AO|AI|Decision|Runtime", meta = (AllowPrivateAccess = true))
	FAOAIDecisionTacticalState TacticalState;

	// 当前帧评估出的主意图。
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AO|AI|Decision|Runtime", meta = (AllowPrivateAccess = true))
	FGameplayTag SelectedIntentTag;

	// 最近一次正式执行过的主意图。
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AO|AI|Decision|Runtime", meta = (AllowPrivateAccess = true))
	FGameplayTag LastExecutedIntentTag;

	// 连续重复执行同一主意图的次数。
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AO|AI|Decision|Runtime", meta = (AllowPrivateAccess = true))
	int32 RepeatedIntentCount = 0;

	// 库存评估事实缓存。
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AO|AI|InventoryDecision|Runtime", meta = (AllowPrivateAccess = true))
	FAOAIInventoryDecisionFacts InventoryDecisionFacts;


	// 当前最新的库存评估结果缓存。
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AO|AI|InventoryDecision|Runtime", meta = (AllowPrivateAccess = true))
	FAOAIInventoryDecisionResult CurrentEvaluationInventoryDecisionResult;

	// 库存动作执行记忆。
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AO|AI|InventoryDecision|Runtime", meta = (AllowPrivateAccess = true))
	FAOAIInventoryDecisionExecutionRecord InventoryDecisionExecutionRecord;

	// 统一决策提交队列。
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AO|AI|Decision|Queue", meta = (AllowPrivateAccess = true))
	TArray<FAOAIDecisionQueueItem> DecisionQueue;

	// 库存决策条目的快照池，避免入队结果被后续评估覆盖。
	UPROPERTY(Transient)
	TArray<FAOAIInventoryDecisionResult> InventoryDecisionPayloads;

	// 队列下一次允许提交的时间。
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AO|AI|Decision|Queue", meta = (AllowPrivateAccess = true))
	float NextDecisionSubmitTimeSeconds = -1.0f;

	// 当前已经正式提交到主链的决策标签。
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AO|AI|Decision|Queue", meta = (AllowPrivateAccess = true))
	FGameplayTag CurrentSubmittedDecisionTag;

	// 最近一次正式提交过的决策标签。
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AO|AI|Decision|Queue", meta = (AllowPrivateAccess = true))
	FGameplayTag LastSubmittedDecisionTag;

	// 当前正式提交中的库存决策结果。
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AO|AI|Decision|Queue", meta = (AllowPrivateAccess = true))
	FAOAIInventoryDecisionResult CurrentSubmittedInventoryDecisionResult;

	// 最近一次正式提交过的库存决策结果。
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AO|AI|Decision|Queue", meta = (AllowPrivateAccess = true))
	FAOAIInventoryDecisionResult LastSubmittedInventoryDecisionResult;

	// 统一决策队列的随机提交时间窗。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AO|AI|Decision|Queue", meta = (AllowPrivateAccess = true, ClampMin = "0.0", UIMin = "0.0"))
	FVector2D DecisionSubmitIntervalRangeSeconds = FVector2D(0.2f, 0.4f);

	// 正式提交库存结果变更事件。
	FOnAISubmittedInventoryDecisionChanged SubmittedInventoryDecisionChangedEvent;

	// 缓存的自身 ASC。
	UPROPERTY(Transient)
	TObjectPtr<UAbilitySystemComponent> CachedSelfAbilitySystemComponent = nullptr;

	// 缓存的自身生命属性集。
	UPROPERTY(Transient)
	TObjectPtr<UAOHealthAttributeSet> CachedSelfHealthAttributeSet = nullptr;

	// 最近一次生命值事件发生的世界时间。
	UPROPERTY(Transient)
	float LastHealthEventWorldTimeSeconds = -1.0f;

	// 最近受击记录。
	UPROPERTY(Transient)
	TArray<FAOAIDecisionRecentDamageEntry> RecentDamageEntries;

	// 待执行动作方向缓存。
	UPROPERTY(Transient)
	FVector PendingActionDirection = FVector::ZeroVector;

	// 当前是否存在有效的待执行动作方向。
	UPROPERTY(Transient)
	bool bHasPendingActionDirection = false;

	// 最近一次战斗 Evaluator 完成缓存回写的时间。
	UPROPERTY(Transient)
	float LastCombatEvaluationTimeSeconds = -1.0f;

	// 最近一次库存 Evaluator 完成缓存回写的时间。
	UPROPERTY(Transient)
	float LastInventoryEvaluationTimeSeconds = -1.0f;

	// 最近一次已经完成统一提交尝试的评估时间。
	UPROPERTY(Transient)
	float LastSubmissionEvaluationTimeSeconds = -1.0f;
};
