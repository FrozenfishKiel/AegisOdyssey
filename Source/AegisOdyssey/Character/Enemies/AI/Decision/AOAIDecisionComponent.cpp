// Fill out your copyright notice in the Description page of Project Settings.

#include "AOAIDecisionComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AegisOdyssey/AOGameplayTags.h"
#include "AegisOdyssey/AbilitySystem/Attributes/Core/AOHealthAttributeSet.h"
#include "AegisOdyssey/Character/Enemies/AI/Decision/AOAIDecisionProfile.h"
#include "AegisOdyssey/Equipment/AOWeaponManagerComponent.h"
#include "AegisOdyssey/Equipment/Weapons/AOWeaponDefinition.h"
#include "AegisOdyssey/Equipment/Weapons/AOWeaponInstance.h"
#include "HAL/PlatformMath.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOAIDecisionComponent)

namespace
{
constexpr float DefaultAIAttackRange = 200.0f;
constexpr int32 MaxQueuedAIDecisionCount = 5;

const AActor* ResolveDecisionOwnerActor(const AActor* Actor)
{
	if (const AController* Controller = Cast<AController>(Actor))
	{
		return Controller->GetPawn();
	}

	return Actor;
}

bool AreQuickBarSlotIndicesEqual(const TArray<int32>& Left, const TArray<int32>& Right)
{
	if (Left.Num() != Right.Num())
	{
		return false;
	}

	for (int32 Index = 0; Index < Left.Num(); ++Index)
	{
		if (Left[Index] != Right[Index])
		{
			return false;
		}
	}

	return true;
}

bool AreInventoryUseCommandsEqual(const FAOAIInventoryUseCommand& Left, const FAOAIInventoryUseCommand& Right)
{
	return Left.CommandType == Right.CommandType
		&& Left.QuickBarSlotIndex == Right.QuickBarSlotIndex
		&& Left.ItemQuery.SemanticTag.MatchesTagExact(Right.ItemQuery.SemanticTag)
		&& Left.ItemQuery.RequiredItemInstanceClass == Right.ItemQuery.RequiredItemInstanceClass
		&& Left.ItemQuery.RequiredItemDefinitionClass == Right.ItemQuery.RequiredItemDefinitionClass
		&& Left.ItemQuery.RequiredFragmentClasses == Right.ItemQuery.RequiredFragmentClasses
		&& Left.ItemQuery.bRequireUsableFromInventory == Right.ItemQuery.bRequireUsableFromInventory
		&& Left.AllowedInventoryComponentClasses == Right.AllowedInventoryComponentClasses
		&& AreQuickBarSlotIndicesEqual(Left.QuickBarSlotIndices, Right.QuickBarSlotIndices);
}

bool AreResolvedInventoryTargetsEqual(const FAOAIResolvedInventoryUseTarget& Left, const FAOAIResolvedInventoryUseTarget& Right)
{
	return Left.InventoryComponent == Right.InventoryComponent
		&& Left.ItemInstance == Right.ItemInstance
		&& Left.SlotIndex == Right.SlotIndex
		&& Left.bUsedQuickBarSlot == Right.bUsedQuickBarSlot
		&& Left.QuickBarSlotIndex == Right.QuickBarSlotIndex;
}

bool AreInventoryDecisionResultsEqual(const FAOAIInventoryDecisionResult& Left, const FAOAIInventoryDecisionResult& Right)
{
	if (Left.bHasAction != Right.bHasAction)
	{
		return false;
	}

	if (!Left.bHasAction && !Right.bHasAction)
	{
		return true;
	}

	return Left.ActionTag.MatchesTagExact(Right.ActionTag)
		&& Left.CandidateTag.MatchesTagExact(Right.CandidateTag)
		&& Left.CoordinationMode == Right.CoordinationMode
		&& FMath::IsNearlyEqual(Left.Desire, Right.Desire)
		&& FMath::IsNearlyEqual(Left.Score, Right.Score)
		&& Left.bHasResolvedTarget == Right.bHasResolvedTarget
		&& AreInventoryUseCommandsEqual(Left.UseCommand, Right.UseCommand)
		&& AreResolvedInventoryTargetsEqual(Left.ResolvedTarget, Right.ResolvedTarget);
}

}

UAOAIDecisionComponent::UAOAIDecisionComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	SyncRuntimeStatesWithDefinitions();
	SyncInventoryRuntimeStatesWithDefinitions();
}

void UAOAIDecisionComponent::BeginPlay()
{
	Super::BeginPlay();
	BindHealthAttributeEventsIfNeeded();
}

void UAOAIDecisionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnbindHealthAttributeEvents();
	Super::EndPlay(EndPlayReason);
}

UAOAIDecisionComponent* UAOAIDecisionComponent::FindAIDecisionComponent(const AActor* Actor)
{
	const AActor* ResolvedActor = ResolveDecisionOwnerActor(Actor);
	return ResolvedActor ? ResolvedActor->FindComponentByClass<UAOAIDecisionComponent>() : nullptr;
}

void UAOAIDecisionComponent::EnsureDecisionDefinitionsInitialized()
{
	EnsureIntentDefinitionsInitialized();
}

const TArray<FAOAIDecisionIntentDefinition>& UAOAIDecisionComponent::GetIntentDefinitions()
{
	EnsureIntentDefinitionsInitialized();
	return IntentDefinitions;
}

const TArray<FAOAIInventoryActionDefinition>& UAOAIDecisionComponent::GetInventoryActionDefinitions()
{
	EnsureIntentDefinitionsInitialized();
	return InventoryActionDefinitions;
}

void UAOAIDecisionComponent::RefreshObservationContext()
{
	CachedSelfAbilitySystemComponent = ResolveAbilitySystemComponentFromActor(GetOwner());
	BindHealthAttributeEventsIfNeeded();

	const UWorld* World = GetWorld();
	const float CurrentWorldTimeSeconds = World ? World->GetTimeSeconds() : -1.0f;
	PruneRecentDamageEntries(CurrentWorldTimeSeconds);
}

void UAOAIDecisionComponent::CacheCombatEvaluation(
	const float CurrentWorldTimeSeconds,
	const FAOAIDecisionCombatFacts& InCombatFacts,
	const TMap<FGameplayTag, FAOAIDecisionIntentRuntimeState>& InIntentRuntimeStates,
	const FGameplayTag& InSelectedIntentTag,
	const FAOAIDecisionTacticalState& InTacticalState)
{
	CombatFacts = InCombatFacts;
	IntentRuntimeStates = InIntentRuntimeStates;
	SelectedIntentTag = InSelectedIntentTag;
	TacticalState = InTacticalState;
	LastCombatEvaluationTimeSeconds = CurrentWorldTimeSeconds;

	if (FMath::IsNearlyEqual(LastInventoryEvaluationTimeSeconds, CurrentWorldTimeSeconds)
		&& !FMath::IsNearlyEqual(LastSubmissionEvaluationTimeSeconds, CurrentWorldTimeSeconds))
	{
		SubmitCurrentDecisionOutputs(CurrentWorldTimeSeconds);
		LastSubmissionEvaluationTimeSeconds = CurrentWorldTimeSeconds;
	}
}

void UAOAIDecisionComponent::CacheInventoryEvaluation(
	const float CurrentWorldTimeSeconds,
	const FAOAIInventoryDecisionFacts& InInventoryDecisionFacts,
	const TMap<FGameplayTag, FAOAIInventoryDecisionCandidateFacts>& InCandidateFactsByActionTag,
	const TMap<FGameplayTag, FAOAIInventoryDecisionRuntimeState>& InInventoryRuntimeStates,
	const FAOAIInventoryDecisionResult& InPendingInventoryDecisionResult)
{
	InventoryDecisionFacts = InInventoryDecisionFacts;
	InventoryCandidateFactsByActionTag = InCandidateFactsByActionTag;
	InventoryRuntimeStates = InInventoryRuntimeStates;
	SetPendingInventoryDecisionResult(InPendingInventoryDecisionResult);
	LastInventoryEvaluationTimeSeconds = CurrentWorldTimeSeconds;

	if (FMath::IsNearlyEqual(LastCombatEvaluationTimeSeconds, CurrentWorldTimeSeconds)
		&& !FMath::IsNearlyEqual(LastSubmissionEvaluationTimeSeconds, CurrentWorldTimeSeconds))
	{
		SubmitCurrentDecisionOutputs(CurrentWorldTimeSeconds);
		LastSubmissionEvaluationTimeSeconds = CurrentWorldTimeSeconds;
	}
}

bool UAOAIDecisionComponent::CommitExecutedIntent(FGameplayTag ExecutedIntentTag, float CurrentWorldTimeSeconds)
{
	EnsureIntentDefinitionsInitialized();

	if (!ExecutedIntentTag.IsValid() || !IntentRuntimeStates.Contains(ExecutedIntentTag))
	{
		return false;
	}

	if (LastExecutedIntentTag.IsValid() && LastExecutedIntentTag.MatchesTagExact(ExecutedIntentTag))
	{
		++RepeatedIntentCount;
	}
	else
	{
		LastExecutedIntentTag = ExecutedIntentTag;
		RepeatedIntentCount = 1;
	}

	if (FAOAIDecisionIntentRuntimeState* RuntimeState = IntentRuntimeStates.Find(ExecutedIntentTag))
	{
		RuntimeState->LastExecutedTime = CurrentWorldTimeSeconds;
	}

	return true;
}

bool UAOAIDecisionComponent::CommitExecutedInventoryAction(const FAOAIInventoryDecisionResult& ExecutedDecisionResult, float CurrentWorldTimeSeconds)
{
	if (!ExecutedDecisionResult.bHasAction || !ExecutedDecisionResult.ActionTag.IsValid())
	{
		return false;
	}

	if (!DoesInventoryDecisionResultMatchPending(ExecutedDecisionResult))
	{
		return false;
	}

	if (InventoryDecisionExecutionRecord.LastExecutedActionTag.IsValid()
		&& InventoryDecisionExecutionRecord.LastExecutedActionTag.MatchesTagExact(ExecutedDecisionResult.ActionTag))
	{
		++InventoryDecisionExecutionRecord.RepeatedActionCount;
	}
	else
	{
		InventoryDecisionExecutionRecord.LastExecutedActionTag = ExecutedDecisionResult.ActionTag;
		InventoryDecisionExecutionRecord.RepeatedActionCount = 1;
	}

	if (FAOAIInventoryDecisionRuntimeState* RuntimeState = InventoryRuntimeStates.Find(ExecutedDecisionResult.ActionTag))
	{
		RuntimeState->LastExecutedTime = CurrentWorldTimeSeconds;
	}

	ClearPendingInventoryDecision();
	return true;
}

void UAOAIDecisionComponent::ClearPendingInventoryDecision()
{
	SetPendingInventoryDecisionResult(FAOAIInventoryDecisionResult());
}

bool UAOAIDecisionComponent::EnqueueDecisionTag(FGameplayTag DecisionTag, float CurrentWorldTimeSeconds)
{
	FAOAIDecisionQueueItem DecisionItem;
	DecisionItem.DecisionTag = DecisionTag;
	DecisionItem.SourceTag = DecisionTag;
	DecisionItem.EnqueueWorldTimeSeconds = CurrentWorldTimeSeconds;
	return EnqueueDecisionItem(DecisionItem);
}

bool UAOAIDecisionComponent::EnqueueDecisionItem(const FAOAIDecisionQueueItem& DecisionItem)
{
	// 统一提交队列只在服务端维护。
	// 这里同时负责去重和固定容量约束，避免同一帧无限堆积重复条目。
	if (!HasDecisionQueueAuthority())
	{
		return false;
	}

	if (!DecisionItem.DecisionTag.IsValid())
	{
		return false;
	}

	for (const FAOAIDecisionQueueItem& ExistingItem : DecisionQueue)
	{
		if (IsQueuedDecisionEquivalent(ExistingItem, DecisionItem))
		{
			return false;
		}
	}

	if (DecisionQueue.Num() >= MaxQueuedAIDecisionCount)
	{
		return false;
	}

	DecisionQueue.Add(DecisionItem);

	if (NextDecisionSubmitTimeSeconds < 0.0f)
	{
		ScheduleNextDecisionSubmitTime(DecisionItem.EnqueueWorldTimeSeconds);
	}

	return true;
}

void UAOAIDecisionComponent::SubmitCurrentDecisionOutputs(float CurrentWorldTimeSeconds)
{
	// 把“评估结果”投影为“正式主链可消费的队列条目”。
	// 当前会同时尝试产出主战斗意图条目和库存动作条目，再按节流窗口提交一个队首结果。
	if (!HasDecisionQueueAuthority())
	{
		return;
	}

	const FGameplayTag IntentDecisionTag = BuildCurrentIntentDecisionTag();
	if (IntentDecisionTag.IsValid())
	{
		FAOAIDecisionQueueItem IntentDecisionItem;
		IntentDecisionItem.DecisionTag = IntentDecisionTag;
		IntentDecisionItem.SourceTag = SelectedIntentTag;
		IntentDecisionItem.EnqueueWorldTimeSeconds = CurrentWorldTimeSeconds;
		EnqueueDecisionItem(IntentDecisionItem);
	}

	FAOAIDecisionQueueItem InventoryDecisionItem;
	if (BuildCurrentInventoryDecisionItem(CurrentWorldTimeSeconds, InventoryDecisionItem))
	{
		EnqueueDecisionItem(InventoryDecisionItem);
	}

	FAOAIDecisionQueueItem SubmittedDecision;
	if (TrySubmitNextDecision(CurrentWorldTimeSeconds, SubmittedDecision))
	{
		CurrentSubmittedDecisionTag = SubmittedDecision.DecisionTag;
		LastSubmittedDecisionTag = SubmittedDecision.DecisionTag;

		FAOAIInventoryDecisionResult SubmittedInventoryDecisionResult;
		if (TryResolveInventoryDecisionResultForQueuedItem(SubmittedDecision, SubmittedInventoryDecisionResult))
		{
			CurrentSubmittedInventoryDecisionResult = SubmittedInventoryDecisionResult;
			LastSubmittedInventoryDecisionResult = SubmittedInventoryDecisionResult;
			SubmittedInventoryDecisionChangedEvent.Broadcast(CurrentSubmittedInventoryDecisionResult);
		}
		else
		{
			CurrentSubmittedInventoryDecisionResult = FAOAIInventoryDecisionResult();
			SubmittedInventoryDecisionChangedEvent.Broadcast(CurrentSubmittedInventoryDecisionResult);
		}
	}
	else if (!HasPendingDecisionTag())
	{
		CurrentSubmittedDecisionTag = FGameplayTag();
		CurrentSubmittedInventoryDecisionResult = FAOAIInventoryDecisionResult();
		SubmittedInventoryDecisionChangedEvent.Broadcast(CurrentSubmittedInventoryDecisionResult);
	}
}

bool UAOAIDecisionComponent::TrySubmitNextDecision(float CurrentWorldTimeSeconds, FAOAIDecisionQueueItem& OutSubmittedDecision)
{
	// 只有到了允许提交的时间点，才真正弹出队首条目。
	// 提交成功后会重新安排下一次时间窗，形成固定速率的逐条出队。
	if (!HasDecisionQueueAuthority() || DecisionQueue.IsEmpty())
	{
		return false;
	}

	if (NextDecisionSubmitTimeSeconds >= 0.0f && CurrentWorldTimeSeconds < NextDecisionSubmitTimeSeconds)
	{
		return false;
	}

	OutSubmittedDecision = DecisionQueue[0];
	DecisionQueue.RemoveAt(0);

	if (DecisionQueue.IsEmpty())
	{
		NextDecisionSubmitTimeSeconds = -1.0f;
	}
	else
	{
		ScheduleNextDecisionSubmitTime(CurrentWorldTimeSeconds);
	}

	return true;
}

bool UAOAIDecisionComponent::HasPendingDecisionTag() const
{
	return !DecisionQueue.IsEmpty() && DecisionQueue[0].DecisionTag.IsValid();
}

FGameplayTag UAOAIDecisionComponent::GetCurrentQueuedDecisionTag() const
{
	return DecisionQueue.IsEmpty() ? FGameplayTag() : DecisionQueue[0].DecisionTag;
}

bool UAOAIDecisionComponent::MatchesCurrentDecisionTag(FGameplayTag DecisionTag) const
{
	if (!DecisionTag.IsValid())
	{
		return false;
	}

	if (CurrentSubmittedDecisionTag.IsValid())
	{
		return CurrentSubmittedDecisionTag.MatchesTag(DecisionTag);
	}

	if (HasPendingDecisionTag())
	{
		return GetCurrentQueuedDecisionTag().MatchesTag(DecisionTag);
	}

	return HasSelectedIntent(DecisionTag);
}

void UAOAIDecisionComponent::ResetDecisionState()
{
	EnsureIntentDefinitionsInitialized();
	CombatFacts = FAOAIDecisionCombatFacts();
	InventoryDecisionFacts = FAOAIInventoryDecisionFacts();
	InventoryCandidateFactsByActionTag.Reset();
	TacticalState = FAOAIDecisionTacticalState();
	PendingActionDirection = FVector::ZeroVector;
	bHasPendingActionDirection = false;
	DecisionQueue.Reset();
	NextDecisionSubmitTimeSeconds = -1.0f;
	CurrentSubmittedDecisionTag = FGameplayTag();
	LastSubmittedDecisionTag = FGameplayTag();
	CurrentSubmittedInventoryDecisionResult = FAOAIInventoryDecisionResult();
	LastSubmittedInventoryDecisionResult = FAOAIInventoryDecisionResult();
	SetPendingInventoryDecisionResult(FAOAIInventoryDecisionResult());
	InventoryDecisionExecutionRecord = FAOAIInventoryDecisionExecutionRecord();
	SelectedIntentTag = FGameplayTag();
	LastExecutedIntentTag = FGameplayTag();
	RepeatedIntentCount = 0;
	LastCombatEvaluationTimeSeconds = -1.0f;
	LastInventoryEvaluationTimeSeconds = -1.0f;
	LastSubmissionEvaluationTimeSeconds = -1.0f;
	ResetRecentDamageTracking();

	for (TPair<FGameplayTag, FAOAIDecisionIntentRuntimeState>& Pair : IntentRuntimeStates)
	{
		Pair.Value = FAOAIDecisionIntentRuntimeState();
	}

	for (TPair<FGameplayTag, FAOAIInventoryDecisionRuntimeState>& Pair : InventoryRuntimeStates)
	{
		Pair.Value = FAOAIInventoryDecisionRuntimeState();
	}
}

bool UAOAIDecisionComponent::HasDecisionQueueAuthority() const
{
	const APawn* OwnerPawn = GetPawn<APawn>();
	return OwnerPawn != nullptr && OwnerPawn->HasAuthority();
}

void UAOAIDecisionComponent::ScheduleNextDecisionSubmitTime(float CurrentWorldTimeSeconds)
{
	const float IntervalMin = FMath::Max(0.0f, DecisionSubmitIntervalRangeSeconds.X);
	const float IntervalMax = FMath::Max(IntervalMin, DecisionSubmitIntervalRangeSeconds.Y);
	NextDecisionSubmitTimeSeconds = CurrentWorldTimeSeconds + FMath::FRandRange(IntervalMin, IntervalMax);
}

FGameplayTag UAOAIDecisionComponent::BuildCurrentIntentDecisionTag() const
{
	return SelectedIntentTag;
}

bool UAOAIDecisionComponent::BuildCurrentInventoryDecisionItem(float CurrentWorldTimeSeconds, FAOAIDecisionQueueItem& OutDecisionItem) const
{
	if (!PendingInventoryDecisionResult.bHasAction || !PendingInventoryDecisionResult.ActionTag.IsValid())
	{
		return false;
	}

	OutDecisionItem = FAOAIDecisionQueueItem();
	OutDecisionItem.DecisionTag = AOGameplayTags::AI_Decision_Inventory_UseItem;
	OutDecisionItem.SourceTag = PendingInventoryDecisionResult.ActionTag;
	OutDecisionItem.EnqueueWorldTimeSeconds = CurrentWorldTimeSeconds;
	return true;
}

bool UAOAIDecisionComponent::TryResolveInventoryDecisionResultForQueuedItem(
	const FAOAIDecisionQueueItem& DecisionItem,
	FAOAIInventoryDecisionResult& OutResult) const
{
	OutResult = FAOAIInventoryDecisionResult();

	if (!DecisionItem.DecisionTag.MatchesTagExact(AOGameplayTags::AI_Decision_Inventory_UseItem))
	{
		return false;
	}

	if (!PendingInventoryDecisionResult.bHasAction || !PendingInventoryDecisionResult.ActionTag.IsValid())
	{
		return false;
	}

	if (DecisionItem.SourceTag.IsValid() && !PendingInventoryDecisionResult.ActionTag.MatchesTagExact(DecisionItem.SourceTag))
	{
		return false;
	}

	OutResult = PendingInventoryDecisionResult;
	return true;
}

bool UAOAIDecisionComponent::IsQueuedDecisionEquivalent(const FAOAIDecisionQueueItem& ExistingItem, const FAOAIDecisionQueueItem& CandidateItem) const
{
	return ExistingItem.DecisionTag.IsValid()
		&& ExistingItem.DecisionTag.MatchesTagExact(CandidateItem.DecisionTag)
		&& ExistingItem.SourceTag.MatchesTagExact(CandidateItem.SourceTag);
}

bool UAOAIDecisionComponent::GetCurrentSubmittedInventoryDecisionResult(FAOAIInventoryDecisionResult& OutResult) const
{
	OutResult = CurrentSubmittedInventoryDecisionResult;
	return CurrentSubmittedInventoryDecisionResult.bHasAction;
}

bool UAOAIDecisionComponent::GetLastSubmittedInventoryDecisionResult(FAOAIInventoryDecisionResult& OutResult) const
{
	OutResult = LastSubmittedInventoryDecisionResult;
	return LastSubmittedInventoryDecisionResult.bHasAction;
}

bool UAOAIDecisionComponent::HasSelectedIntent(FGameplayTag IntentTag) const
{
	return IntentTag.IsValid() && SelectedIntentTag.IsValid() && SelectedIntentTag.MatchesTag(IntentTag);
}

bool UAOAIDecisionComponent::GetIntentMetrics(FGameplayTag IntentTag, float& OutDesire, float& OutScore) const
{
	const FGameplayTag ResolvedIntentTag = IntentTag.IsValid() ? IntentTag : SelectedIntentTag;
	const FAOAIDecisionIntentRuntimeState* RuntimeState = IntentRuntimeStates.Find(ResolvedIntentTag);
	if (RuntimeState == nullptr)
	{
		OutDesire = 0.0f;
		OutScore = 0.0f;
		return false;
	}

	OutDesire = RuntimeState->Desire;
	OutScore = RuntimeState->Score;
	return true;
}

void UAOAIDecisionComponent::SetPendingActionDirection(FVector InWorldDirection)
{
	const FVector PlanarDirection = FVector(InWorldDirection.X, InWorldDirection.Y, 0.0f).GetSafeNormal();
	PendingActionDirection = PlanarDirection;
	bHasPendingActionDirection = !PlanarDirection.IsNearlyZero();
}

void UAOAIDecisionComponent::ClearPendingActionDirection()
{
	PendingActionDirection = FVector::ZeroVector;
	bHasPendingActionDirection = false;
}

bool UAOAIDecisionComponent::GetPendingActionDirection(FVector& OutWorldDirection) const
{
	OutWorldDirection = PendingActionDirection;
	return bHasPendingActionDirection;
}

bool UAOAIDecisionComponent::GetPendingInventoryDecisionResult(FAOAIInventoryDecisionResult& OutResult) const
{
	OutResult = PendingInventoryDecisionResult;
	return PendingInventoryDecisionResult.bHasAction;
}

float UAOAIDecisionComponent::GetSelectedIntentDesire() const
{
	float OutDesire = 0.0f;
	float OutScore = 0.0f;
	GetIntentMetrics(SelectedIntentTag, OutDesire, OutScore);
	return OutDesire;
}

float UAOAIDecisionComponent::GetSelectedIntentScore() const
{
	float OutDesire = 0.0f;
	float OutScore = 0.0f;
	GetIntentMetrics(SelectedIntentTag, OutDesire, OutScore);
	return OutScore;
}

void UAOAIDecisionComponent::EnsureIntentDefinitionsInitialized()
{
	ApplyDecisionProfileIfNeeded();
	SyncRuntimeStatesWithDefinitions();
	SyncInventoryRuntimeStatesWithDefinitions();
}

void UAOAIDecisionComponent::ApplyDecisionProfileIfNeeded()
{
	if (DecisionProfile == nullptr)
	{
		return;
	}

	IntentDefinitions = DecisionProfile->IntentDefinitions;
	InventoryActionDefinitions = DecisionProfile->InventoryActionDefinitions;
}

void UAOAIDecisionComponent::SyncRuntimeStatesWithDefinitions()
{
	TSet<FGameplayTag> ValidIntentTags;

	for (const FAOAIDecisionIntentDefinition& Definition : IntentDefinitions)
	{
		if (!Definition.IntentTag.IsValid() || ValidIntentTags.Contains(Definition.IntentTag))
		{
			continue;
		}

		ValidIntentTags.Add(Definition.IntentTag);
		IntentRuntimeStates.FindOrAdd(Definition.IntentTag);
	}

	TArray<FGameplayTag> ExistingIntentTags;
	IntentRuntimeStates.GenerateKeyArray(ExistingIntentTags);

	for (const FGameplayTag& ExistingIntentTag : ExistingIntentTags)
	{
		if (!ValidIntentTags.Contains(ExistingIntentTag))
		{
			IntentRuntimeStates.Remove(ExistingIntentTag);
		}
	}
}

void UAOAIDecisionComponent::SyncInventoryRuntimeStatesWithDefinitions()
{
	TSet<FGameplayTag> ValidActionTags;

	for (const FAOAIInventoryActionDefinition& Definition : InventoryActionDefinitions)
	{
		if (!Definition.ActionTag.IsValid() || ValidActionTags.Contains(Definition.ActionTag))
		{
			continue;
		}

		ValidActionTags.Add(Definition.ActionTag);
		InventoryRuntimeStates.FindOrAdd(Definition.ActionTag);
	}

	TArray<FGameplayTag> ExistingActionTags;
	InventoryRuntimeStates.GenerateKeyArray(ExistingActionTags);

	for (const FGameplayTag& ExistingActionTag : ExistingActionTags)
	{
		if (!ValidActionTags.Contains(ExistingActionTag))
		{
			InventoryRuntimeStates.Remove(ExistingActionTag);
		}
	}
}

void UAOAIDecisionComponent::SetPendingInventoryDecisionResult(const FAOAIInventoryDecisionResult& NewResult)
{
	if (AreInventoryDecisionResultsEqual(PendingInventoryDecisionResult, NewResult))
	{
		return;
	}

	PendingInventoryDecisionResult = NewResult;
	PendingInventoryDecisionChangedEvent.Broadcast(PendingInventoryDecisionResult);
}

void UAOAIDecisionComponent::ResetRecentDamageTracking()
{
	LastHealthEventWorldTimeSeconds = -1.0f;
	RecentDamageEntries.Reset();
	CombatFacts.RecentDamageRatio = 0.0f;
}

void UAOAIDecisionComponent::BindHealthAttributeEventsIfNeeded()
{
	UAbilitySystemComponent* AbilitySystemComponent = ResolveAbilitySystemComponentFromActor(GetOwner());
	if (AbilitySystemComponent == nullptr)
	{
		UnbindHealthAttributeEvents();
		return;
	}

	const UAOHealthAttributeSet* HealthAttributeSet = Cast<UAOHealthAttributeSet>(AbilitySystemComponent->GetAttributeSet(UAOHealthAttributeSet::StaticClass()));
	if (CachedSelfAbilitySystemComponent == AbilitySystemComponent && CachedSelfHealthAttributeSet == HealthAttributeSet)
	{
		return;
	}

	UnbindHealthAttributeEvents();

	CachedSelfAbilitySystemComponent = AbilitySystemComponent;
	CachedSelfHealthAttributeSet = const_cast<UAOHealthAttributeSet*>(HealthAttributeSet);
	if (CachedSelfHealthAttributeSet == nullptr)
	{
		return;
	}

	CachedSelfHealthAttributeSet->OnHealthChange.AddUObject(this, &ThisClass::HandleSelfHealthChanged);
}

void UAOAIDecisionComponent::UnbindHealthAttributeEvents()
{
	if (CachedSelfHealthAttributeSet != nullptr)
	{
		CachedSelfHealthAttributeSet->OnHealthChange.RemoveAll(this);
	}

	CachedSelfAbilitySystemComponent = nullptr;
	CachedSelfHealthAttributeSet = nullptr;
}

void UAOAIDecisionComponent::HandleSelfHealthChanged(AActor* EffectInstigator, AActor* EffectCauser, const FGameplayEffectSpec* EffectSpec, float EffectMagnitude, float OldValue, float NewValue)
{
	const float LostHealth = FMath::Max(0.0f, OldValue - NewValue);
	if (LostHealth <= KINDA_SMALL_NUMBER)
	{
		return;
	}

	const UWorld* World = GetWorld();
	const float CurrentWorldTimeSeconds = World ? World->GetTimeSeconds() : -1.0f;
	LastHealthEventWorldTimeSeconds = CurrentWorldTimeSeconds;

	FAOAIDecisionRecentDamageEntry& Entry = RecentDamageEntries.AddDefaulted_GetRef();
	Entry.WorldTimeSeconds = CurrentWorldTimeSeconds;
	Entry.DamageAmount = LostHealth;
}

float UAOAIDecisionComponent::ResolveAIAttackRangeFromActor(const AActor* Actor, float DefaultRange) const
{
	const AActor* ResolvedActor = ResolveDecisionOwnerActor(Actor);
	const APawn* OwnerPawn = Cast<APawn>(ResolvedActor);
	if (OwnerPawn != nullptr)
	{
		if (const UAOWeaponManagerComponent* WeaponManagerComponent = OwnerPawn->FindComponentByClass<UAOWeaponManagerComponent>())
		{
			if (const UAOWeaponInstance* WeaponInstance = Cast<UAOWeaponInstance>(WeaponManagerComponent->GetCurrentWeaponInstance()))
			{
				if (const UAOWeaponDefinition* WeaponDefinition = Cast<UAOWeaponDefinition>(WeaponInstance->GetItemCDO()))
				{
					return FMath::Max(1.0f, WeaponDefinition->GetAIAttackRange());
				}
			}
		}
	}

	return FMath::Max(1.0f, DefaultRange);
}

UAbilitySystemComponent* UAOAIDecisionComponent::ResolveAbilitySystemComponentFromActor(const AActor* Actor) const
{
	const AActor* ResolvedActor = ResolveDecisionOwnerActor(Actor);
	return ResolvedActor ? UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(const_cast<AActor*>(ResolvedActor)) : nullptr;
}

bool UAOAIDecisionComponent::HasMatchingTag(const AActor* Actor, const FGameplayTag& Tag, bool bExactMatch) const
{
	if (!Actor || !Tag.IsValid())
	{
		return false;
	}

	if (UAbilitySystemComponent* AbilitySystemComponent = ResolveAbilitySystemComponentFromActor(Actor))
	{
		if (bExactMatch)
		{
			FGameplayTagContainer OwnedTags;
			AbilitySystemComponent->GetOwnedGameplayTags(OwnedTags);
			return OwnedTags.HasTagExact(Tag);
		}

		return AbilitySystemComponent->HasMatchingGameplayTag(Tag);
	}

	return false;
}

float UAOAIDecisionComponent::GetAttributeValue(const FGameplayAttribute& Attribute) const
{
	if (CachedSelfAbilitySystemComponent == nullptr || !Attribute.IsValid())
	{
		return 0.0f;
	}

	return CachedSelfAbilitySystemComponent->GetNumericAttribute(Attribute);
}

float UAOAIDecisionComponent::GetRecentDamageRatio(float CurrentWorldTimeSeconds, float WindowSeconds) const
{
	if (CachedSelfHealthAttributeSet == nullptr)
	{
		return 0.0f;
	}

	const float MaxHealth = CachedSelfHealthAttributeSet->GetMaxHealth();
	if (MaxHealth <= KINDA_SMALL_NUMBER)
	{
		return 0.0f;
	}

	const float SafeWindowSeconds = FMath::Max(0.0f, WindowSeconds);
	float TotalRecentDamage = 0.0f;

	for (const FAOAIDecisionRecentDamageEntry& Entry : RecentDamageEntries)
	{
		if (Entry.DamageAmount <= 0.0f)
		{
			continue;
		}

		if (SafeWindowSeconds > 0.0f && CurrentWorldTimeSeconds >= 0.0f && Entry.WorldTimeSeconds >= 0.0f)
		{
			if ((CurrentWorldTimeSeconds - Entry.WorldTimeSeconds) > SafeWindowSeconds)
			{
				continue;
			}
		}

		TotalRecentDamage += Entry.DamageAmount;
	}

	return TotalRecentDamage / MaxHealth;
}

float UAOAIDecisionComponent::GetRecentDamageDebugWindowSeconds() const
{
	float MaxWindowSeconds = 0.0f;
	bool bFoundWindowSetting = false;

	for (const FAOAIDecisionIntentDefinition& Definition : IntentDefinitions)
	{
		if (!Definition.IntentTag.IsValid())
		{
			continue;
		}

		if (Definition.RecentDamageWindowSeconds <= 0.0f)
		{
			return 0.0f;
		}

		MaxWindowSeconds = FMath::Max(MaxWindowSeconds, Definition.RecentDamageWindowSeconds);
		bFoundWindowSetting = true;
	}

	for (const FAOAIInventoryActionDefinition& Definition : InventoryActionDefinitions)
	{
		if (!Definition.ActionTag.IsValid())
		{
			continue;
		}

		if (Definition.RecentDamageWindowSeconds <= 0.0f)
		{
			return 0.0f;
		}

		MaxWindowSeconds = FMath::Max(MaxWindowSeconds, Definition.RecentDamageWindowSeconds);
		bFoundWindowSetting = true;
	}

	return bFoundWindowSetting ? MaxWindowSeconds : 0.0f;
}

void UAOAIDecisionComponent::PruneRecentDamageEntries(float CurrentWorldTimeSeconds)
{
	if (RecentDamageEntries.IsEmpty() || CurrentWorldTimeSeconds < 0.0f)
	{
		return;
	}

	const float MaxWindowSeconds = GetRecentDamageDebugWindowSeconds();
	if (MaxWindowSeconds <= 0.0f)
	{
		return;
	}

	for (int32 Index = RecentDamageEntries.Num() - 1; Index >= 0; --Index)
	{
		const FAOAIDecisionRecentDamageEntry& Entry = RecentDamageEntries[Index];
		if (Entry.WorldTimeSeconds < 0.0f || (CurrentWorldTimeSeconds - Entry.WorldTimeSeconds) > MaxWindowSeconds)
		{
			RecentDamageEntries.RemoveAtSwap(Index);
		}
	}
}

bool UAOAIDecisionComponent::DoesInventoryDecisionResultMatchPending(const FAOAIInventoryDecisionResult& ExecutedDecisionResult) const
{
	if (!PendingInventoryDecisionResult.bHasAction || !PendingInventoryDecisionResult.ActionTag.IsValid())
	{
		return false;
	}

	if (!PendingInventoryDecisionResult.ActionTag.MatchesTagExact(ExecutedDecisionResult.ActionTag))
	{
		return false;
	}

	if (PendingInventoryDecisionResult.CandidateTag.IsValid() || ExecutedDecisionResult.CandidateTag.IsValid())
	{
		if (!PendingInventoryDecisionResult.CandidateTag.MatchesTagExact(ExecutedDecisionResult.CandidateTag))
		{
			return false;
		}
	}

	return PendingInventoryDecisionResult.UseCommand.CommandType == ExecutedDecisionResult.UseCommand.CommandType
		&& PendingInventoryDecisionResult.UseCommand.QuickBarSlotIndex == ExecutedDecisionResult.UseCommand.QuickBarSlotIndex
		&& PendingInventoryDecisionResult.UseCommand.QuickBarSlotIndices == ExecutedDecisionResult.UseCommand.QuickBarSlotIndices
		&& PendingInventoryDecisionResult.UseCommand.AllowedInventoryComponentClasses == ExecutedDecisionResult.UseCommand.AllowedInventoryComponentClasses
		&& PendingInventoryDecisionResult.UseCommand.ItemQuery.SemanticTag.MatchesTagExact(ExecutedDecisionResult.UseCommand.ItemQuery.SemanticTag)
		&& PendingInventoryDecisionResult.UseCommand.ItemQuery.RequiredItemInstanceClass == ExecutedDecisionResult.UseCommand.ItemQuery.RequiredItemInstanceClass
		&& PendingInventoryDecisionResult.UseCommand.ItemQuery.RequiredItemDefinitionClass == ExecutedDecisionResult.UseCommand.ItemQuery.RequiredItemDefinitionClass
		&& PendingInventoryDecisionResult.UseCommand.ItemQuery.RequiredFragmentClasses == ExecutedDecisionResult.UseCommand.ItemQuery.RequiredFragmentClasses
		&& PendingInventoryDecisionResult.UseCommand.ItemQuery.bRequireUsableFromInventory == ExecutedDecisionResult.UseCommand.ItemQuery.bRequireUsableFromInventory;
}
