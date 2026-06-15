#include "STE_UpdateInventoryDecision.h"

#include "AbilitySystemComponent.h"
#include "AegisOdyssey/AbilitySystem/Attributes/Combat/AOCombatAttributeSet.h"
#include "AegisOdyssey/AbilitySystem/Attributes/Core/AOHealthAttributeSet.h"
#include "AegisOdyssey/Character/Enemies/AI/AOAIInventoryRuntimeUseLibrary.h"
#include "AegisOdyssey/Character/Enemies/AI/Decision/AOAIDecisionComponent.h"
#include "AegisOdyssey/Equipment/Weapons/AOWeaponDefinition.h"
#include "AegisOdyssey/Inventory/AOInventoryComponent.h"
#include "AegisOdyssey/Inventory/AOInventoryItemDefinition.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "StateTreeExecutionContext.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(STE_UpdateInventoryDecision)

namespace
{
int32 ResolveInventorySlotIndexFromResolvedTarget(const FAOAIResolvedInventoryUseTarget& ResolvedTarget)
{
	return ResolvedTarget.bUsedQuickBarSlot ? ResolvedTarget.QuickBarSlotIndex : ResolvedTarget.SlotIndex;
}

APawn* ResolveInventoryDecisionOwnerPawn(const FStateTreeExecutionContext& Context)
{
	if (AActor* OwnerActor = Cast<AActor>(Context.GetOwner()))
	{
		if (APawn* OwnerPawn = Cast<APawn>(OwnerActor))
		{
			return OwnerPawn;
		}

		if (AController* OwnerController = Cast<AController>(OwnerActor))
		{
			return OwnerController->GetPawn();
		}
	}

	return nullptr;
}

void ResetInventoryDecisionOutputs(FUpdateInventoryDecisionInstanceData& InstanceData)
{
	InstanceData.bHasCurrentSubmittedInventoryDecision = false;
	InstanceData.CurrentSubmittedInventoryDecision = FAOAIInventoryDecisionResult();
	InstanceData.bHasAdditiveInventoryWindow = false;
}

bool DoesInventoryDecisionCurrentIntentMatchAnyTag(const FGameplayTag& CurrentIntentTag, const FGameplayTagContainer& IntentTags)
{
	return CurrentIntentTag.IsValid() && !IntentTags.IsEmpty() && IntentTags.HasTag(CurrentIntentTag);
}

float EvaluateInventoryDecisionResponseCurveFactor(const FAOAIDecisionResponseCurveFactor& Factor, const float RawInput)
{
	if (!Factor.bEnabled || FMath::IsNearlyZero(Factor.Weight))
	{
		return 0.0f;
	}

	float EvaluatedInput = (RawInput * Factor.InputScale) + Factor.InputBias;
	if (Factor.bClampInput)
	{
		const float ClampMin = FMath::Min(Factor.InputClampRange.X, Factor.InputClampRange.Y);
		const float ClampMax = FMath::Max(Factor.InputClampRange.X, Factor.InputClampRange.Y);
		EvaluatedInput = FMath::Clamp(EvaluatedInput, ClampMin, ClampMax);
	}

	const FRichCurve* RichCurve = Factor.ResponseCurve.GetRichCurveConst();
	const bool bHasValidCurve = (RichCurve != nullptr) && (RichCurve->GetNumKeys() > 0);
	const float FinalValue = bHasValidCurve ? RichCurve->Eval(EvaluatedInput, 0.0f) : EvaluatedInput;
	return FinalValue * Factor.Weight;
}

float EvaluateInventoryDecisionAttributeIntervalFactor(const UAOAIDecisionComponent& DecisionComponent, const FAOAIDecisionAttributeIntervalFactor& Factor)
{
	if (!Factor.bEnabled || FMath::IsNearlyZero(Factor.Weight) || !Factor.NumeratorAttribute.IsValid())
	{
		return 0.0f;
	}

	const float NumeratorValue = DecisionComponent.GetAttributeValue(Factor.NumeratorAttribute);
	float DenominatorValue = Factor.ManualDenominator;
	if (Factor.DenominatorAttribute.IsValid())
	{
		const float RuntimeDenominatorValue = DecisionComponent.GetAttributeValue(Factor.DenominatorAttribute);
		if (RuntimeDenominatorValue > KINDA_SMALL_NUMBER)
		{
			DenominatorValue = RuntimeDenominatorValue;
		}
	}

	DenominatorValue = FMath::Max(KINDA_SMALL_NUMBER, DenominatorValue);
	const float NormalizedValue = NumeratorValue / DenominatorValue;

	const float MinValue = FMath::Min(Factor.RangeMin, Factor.RangeMax);
	const float MaxValue = FMath::Max(Factor.RangeMin, Factor.RangeMax);
	if (NormalizedValue < MinValue || NormalizedValue > MaxValue)
	{
		return 0.0f;
	}

	float Alpha = 1.0f;
	if (!FMath::IsNearlyEqual(MinValue, MaxValue))
	{
		Alpha = FMath::GetRangePct(MinValue, MaxValue, NormalizedValue);
	}

	Alpha = FMath::Clamp(Alpha, 0.0f, 1.0f);
	const float ProximityRatio = Factor.bHigherScoreNearMin ? (1.0f - Alpha) : Alpha;

	const FRichCurve* RichCurve = Factor.ResponseCurve.GetRichCurveConst();
	const bool bHasValidCurve = (RichCurve != nullptr) && (RichCurve->GetNumKeys() > 0);
	const float FinalRatio = bHasValidCurve ? RichCurve->Eval(ProximityRatio, ProximityRatio) : ProximityRatio;
	return FinalRatio * Factor.Weight;
}

bool CanInventoryActionCooperateWithTacticalState(
	const FAOAIDecisionTacticalState& TacticalState,
	const FAOAIInventoryActionDefinition& Definition)
{
	if (Definition.CoordinationMode == EAOAIInventoryActionCoordinationMode::Additive
		&& !TacticalState.bHasAdditiveInventoryWindow)
	{
		return false;
	}

	if (!Definition.RequiredMainIntentTags.IsEmpty()
		&& !DoesInventoryDecisionCurrentIntentMatchAnyTag(TacticalState.CurrentMainIntentTag, Definition.RequiredMainIntentTags))
	{
		return false;
	}

	if (!Definition.BlockedMainIntentTags.IsEmpty()
		&& DoesInventoryDecisionCurrentIntentMatchAnyTag(TacticalState.CurrentMainIntentTag, Definition.BlockedMainIntentTags))
	{
		return false;
	}

	if (!Definition.RequiredTacticalTags.IsEmpty()
		&& !TacticalState.TacticalTags.HasAll(Definition.RequiredTacticalTags))
	{
		return false;
	}

	if (!Definition.BlockedTacticalTags.IsEmpty()
		&& TacticalState.TacticalTags.HasAny(Definition.BlockedTacticalTags))
	{
		return false;
	}

	return true;
}

bool CanEvaluateInventoryAction(
	const FAOAIInventoryDecisionFacts& InventoryDecisionFacts,
	const FAOAIDecisionTacticalState& TacticalState,
	const TMap<FGameplayTag, FAOAIInventoryDecisionCandidateFacts>& CandidateFactsByActionTag,
	const FAOAIInventoryActionDefinition& Definition)
{
	if (Definition.bRequireTarget && !InventoryDecisionFacts.bHasTarget)
	{
		return false;
	}

	const FAOAIInventoryDecisionCandidateFacts* CandidateFacts = CandidateFactsByActionTag.Find(Definition.ActionTag);
	return CandidateFacts != nullptr
		&& CandidateFacts->bHasUsableCandidate
		&& CanInventoryActionCooperateWithTacticalState(TacticalState, Definition);
}

float ComputeInventoryActionDesire(
	const UAOAIDecisionComponent& DecisionComponent,
	const FAOAIDecisionCombatFacts& CombatFacts,
	const FAOAIInventoryDecisionFacts& InventoryDecisionFacts,
	const FAOAIInventoryActionDefinition& Definition,
	const float TimeSinceLastExecution,
	const float DistanceRatio,
	const float CurrentWorldTimeSeconds)
{
	const float SafeCadenceSeconds = FMath::Max(KINDA_SMALL_NUMBER, Definition.CadenceSeconds);
	const float CadenceAlpha = FMath::Clamp(TimeSinceLastExecution / SafeCadenceSeconds, 0.0f, 1.0f);
	const float RecentDamageRatio = DecisionComponent.GetRecentDamageRatio(CurrentWorldTimeSeconds, Definition.RecentDamageWindowSeconds);

	float Desire = Definition.BaseDesire;
	Desire += CadenceAlpha * Definition.CadenceWeight;
	Desire += EvaluateInventoryDecisionResponseCurveFactor(Definition.HealthRatioFactor, InventoryDecisionFacts.HealthRatio);
	Desire += EvaluateInventoryDecisionResponseCurveFactor(Definition.StaminaRatioFactor, InventoryDecisionFacts.StaminaRatio);
	Desire += EvaluateInventoryDecisionResponseCurveFactor(Definition.DistanceFactor, DistanceRatio);
	Desire += EvaluateInventoryDecisionResponseCurveFactor(Definition.RecentDamageFactor, RecentDamageRatio);

	for (const FAOAIDecisionAttributeIntervalFactor& AttributeFactor : Definition.AttributeIntervalFactors)
	{
		Desire += EvaluateInventoryDecisionAttributeIntervalFactor(DecisionComponent, AttributeFactor);
	}

	for (const FAOAIDecisionTagScoreFactor& TagFactor : Definition.TargetStateTagFactors)
	{
		if (!TagFactor.Tag.IsValid())
		{
			continue;
		}

		if (DecisionComponent.HasMatchingTag(CombatFacts.CurrentTarget, TagFactor.Tag, TagFactor.bExactMatch))
		{
			Desire += TagFactor.ScoreDelta;
		}
	}

	return Desire;
}

float ComputeInventoryActionScore(
	const FAOAIInventoryActionDefinition& Definition,
	const FGameplayTag& ActionTag,
	const float Desire,
	const float TimeSinceLastExecution,
	const FAOAIInventoryDecisionExecutionRecord& ExecutionRecord)
{
	float Score = Desire;

	if (TimeSinceLastExecution < Definition.CooldownSeconds)
	{
		Score *= Definition.CooldownPenaltyMultiplier;
	}

	if (ExecutionRecord.LastExecutedActionTag.IsValid())
	{
		if (ExecutionRecord.LastExecutedActionTag.MatchesTagExact(ActionTag))
		{
			Score -= Definition.RepeatPenalty * ExecutionRecord.RepeatedActionCount;
		}
		else
		{
			Score += Definition.SwitchBonus;
		}
	}

	return FMath::Max(0.0f, Score);
}

const FAOAIInventoryActionDefinition* FindInventoryActionDefinitionByTag(
	const TArray<FAOAIInventoryActionDefinition>& InventoryActionDefinitions,
	const FGameplayTag& ActionTag)
{
	for (const FAOAIInventoryActionDefinition& Definition : InventoryActionDefinitions)
	{
		if (Definition.ActionTag.IsValid() && Definition.ActionTag.MatchesTagExact(ActionTag))
		{
			return &Definition;
		}
	}

	return nullptr;
}

FGameplayTag ResolveFallbackInventoryActionTag(
	const FAOAIInventoryDecisionFacts& InventoryDecisionFacts,
	const FAOAIDecisionTacticalState& TacticalState,
	const TMap<FGameplayTag, FAOAIInventoryDecisionCandidateFacts>& CandidateFactsByActionTag,
	const TArray<FAOAIInventoryActionDefinition>& InventoryActionDefinitions)
{
	float BestFallbackWeight = -1.0f;
	FGameplayTag FallbackActionTag;

	for (const FAOAIInventoryActionDefinition& Definition : InventoryActionDefinitions)
	{
		if (!Definition.ActionTag.IsValid()
			|| !CanEvaluateInventoryAction(InventoryDecisionFacts, TacticalState, CandidateFactsByActionTag, Definition))
		{
			continue;
		}

		if (Definition.FallbackSelectionWeight > BestFallbackWeight)
		{
			BestFallbackWeight = Definition.FallbackSelectionWeight;
			FallbackActionTag = Definition.ActionTag;
		}
	}

	return FallbackActionTag;
}

FAOAIInventoryDecisionCandidateFacts BuildInventoryCandidateFacts(APawn* OwnerPawn, const FAOAIInventoryUseCommand& UseCommand)
{
	FAOAIInventoryDecisionCandidateFacts Facts;
	if (OwnerPawn == nullptr)
	{
		return Facts;
	}

	Facts.CandidateCount = UAOAIInventoryRuntimeUseLibrary::CountMatchingInventoryEntries(OwnerPawn, UseCommand, false);
	Facts.bHasAnyCandidate = Facts.CandidateCount > 0;
	const int32 UsableCount = UAOAIInventoryRuntimeUseLibrary::CountMatchingInventoryEntries(OwnerPawn, UseCommand, true);
	Facts.bHasUsableCandidate = UsableCount > 0;
	return Facts;
}

const UAOInventoryItemDefinition* ResolveItemDefinitionFromResolvedTarget(const FAOAIResolvedInventoryUseTarget& ResolvedTarget)
{
	if (ResolvedTarget.ItemInstance != nullptr)
	{
		return ResolvedTarget.ItemInstance->GetItemCDO();
	}

	const int32 ResolvedSlotIndex = ResolveInventorySlotIndexFromResolvedTarget(ResolvedTarget);
	if (ResolvedTarget.InventoryComponent != nullptr && ResolvedSlotIndex != INDEX_NONE)
	{
		if (const FAOInventoryEntry* InventoryEntry = ResolvedTarget.InventoryComponent->GetInventoryEntryAtSlot(ResolvedSlotIndex))
		{
			if (InventoryEntry->Instance != nullptr)
			{
				return InventoryEntry->Instance->GetItemCDO();
			}
		}
	}

	return nullptr;
}

FAOAIInventoryConcreteCandidateFacts BuildConcreteCandidateFactFromResolvedTarget(
	const FAOAIInventoryDecisionFacts& InventoryDecisionFacts,
	const FAOAIResolvedInventoryUseTarget& ResolvedTarget)
{
	FAOAIInventoryConcreteCandidateFacts ConcreteCandidate;
	ConcreteCandidate.ResolvedTarget = ResolvedTarget;

	if (ResolvedTarget.InventoryComponent == nullptr)
	{
		return ConcreteCandidate;
	}

	const int32 ResolvedSlotIndex = ResolveInventorySlotIndexFromResolvedTarget(ResolvedTarget);
	if (ResolvedSlotIndex != INDEX_NONE)
	{
		if (const FAOInventoryEntry* InventoryEntry = ResolvedTarget.InventoryComponent->GetInventoryEntryAtSlot(ResolvedSlotIndex))
		{
			ConcreteCandidate.StackCount = InventoryEntry->StackCount;
		}
	}

	if (const UAOInventoryItemDefinition* ItemDefinition = ResolveItemDefinitionFromResolvedTarget(ResolvedTarget))
	{
		if (const UAOWeaponDefinition* WeaponDefinition = Cast<UAOWeaponDefinition>(ItemDefinition))
		{
			ConcreteCandidate.WeaponAIAttackRange = WeaponDefinition->GetAIAttackRange();

			if (!InventoryDecisionFacts.bHasTarget)
			{
				ConcreteCandidate.bMatchesCurrentCombatDistance = true;
			}
			else
			{
				const float CandidateDistanceRatio =
					InventoryDecisionFacts.TargetDistance / FMath::Max(1.0f, WeaponDefinition->GetAIAttackRange());
				ConcreteCandidate.bMatchesCurrentCombatDistance = CandidateDistanceRatio <= 1.0f;
			}
		}
	}

	return ConcreteCandidate;
}

void BuildInventoryConcreteCandidateFacts(
	APawn* OwnerPawn,
	const FAOAIInventoryDecisionFacts& InventoryDecisionFacts,
	const FAOAIInventoryUseCommand& UseCommand,
	TArray<FAOAIInventoryConcreteCandidateFacts>& OutConcreteCandidates)
{
	OutConcreteCandidates.Reset();
	if (OwnerPawn == nullptr)
	{
		return;
	}

	TArray<FAOAIResolvedInventoryUseTarget> ResolvedTargets;
	UAOAIInventoryRuntimeUseLibrary::GatherMatchingInventoryTargets(OwnerPawn, UseCommand, true, ResolvedTargets);
	OutConcreteCandidates.Reserve(ResolvedTargets.Num());

	for (const FAOAIResolvedInventoryUseTarget& ResolvedTarget : ResolvedTargets)
	{
		OutConcreteCandidates.Add(BuildConcreteCandidateFactFromResolvedTarget(InventoryDecisionFacts, ResolvedTarget));
	}
}

float ComputeInventoryCandidateScore(
	const FAOAIInventoryDecisionFacts& InventoryDecisionFacts,
	const TMap<FGameplayTag, FAOAIInventoryDecisionCandidateFacts>& CandidateFactsByActionTag,
	const FAOAIInventoryActionDefinition& ActionDefinition,
	const FAOAIInventoryDecisionCandidateDefinition& CandidateDefinition,
	const FAOAIInventoryConcreteCandidateFacts& ConcreteCandidate)
{
	float Score = CandidateDefinition.BaseScore;
	Score += (1.0f - InventoryDecisionFacts.HealthRatio) * CandidateDefinition.DesiredHealthDeficitWeight;
	Score += (1.0f - InventoryDecisionFacts.StaminaRatio) * CandidateDefinition.DesiredStaminaDeficitWeight;
	Score += InventoryDecisionFacts.RecentDamageRatio * CandidateDefinition.DesiredRecentDamageWeight;

	const float DistanceRatio =
		InventoryDecisionFacts.TargetDistance / FMath::Max(1.0f, InventoryDecisionFacts.SelfAIAttackRange);
	Score += FMath::Max(0.0f, DistanceRatio - 1.0f) * CandidateDefinition.DesiredOutOfRangeWeight;
	Score += InventoryDecisionFacts.bTargetInCombatWindow ? CandidateDefinition.DesireCombatWindowWeight : 0.0f;
	Score += InventoryDecisionFacts.bTargetInRecovery ? CandidateDefinition.DesireTargetRecoveryWeight : 0.0f;
	Score += !InventoryDecisionFacts.bCurrentWeaponFitsCombatDistance ? CandidateDefinition.DesireNeedWeaponSwapWeight : 0.0f;
	Score += ConcreteCandidate.StackCount * CandidateDefinition.DesiredHighStackCountWeight;
	Score += ConcreteCandidate.bMatchesCurrentCombatDistance ? CandidateDefinition.DesiredDistanceFitWeight : 0.0f;

	const int32 ResolvedSlotIndex = ResolveInventorySlotIndexFromResolvedTarget(ConcreteCandidate.ResolvedTarget);
	if (ResolvedSlotIndex != INDEX_NONE)
	{
		Score -= CandidateDefinition.PreferLowerSlotIndexWeight * static_cast<float>(ResolvedSlotIndex);
	}

	if (const FAOAIInventoryDecisionCandidateFacts* CandidateFacts = CandidateFactsByActionTag.Find(ActionDefinition.ActionTag))
	{
		if (!CandidateFacts->bHasUsableCandidate)
		{
			return 0.0f;
		}
	}

	return FMath::Max(0.0f, Score);
}

// 在某个库存动作内部，从所有具体候选物品策略里挑出当前最值得执行的那一个。
// 这里比较的是“候选分”，不是动作层 Desire/Score。
const FAOAIInventoryDecisionCandidateDefinition* FindBestInventoryCandidate(
	APawn* OwnerPawn,
	const FAOAIInventoryDecisionFacts& InventoryDecisionFacts,
	const TMap<FGameplayTag, FAOAIInventoryDecisionCandidateFacts>& CandidateFactsByActionTag,
	const FAOAIInventoryActionDefinition& ActionDefinition,
	float& OutCandidateScore,
	FAOAIInventoryConcreteCandidateFacts& OutResolvedConcreteCandidate)
{
	OutCandidateScore = 0.0f;
	OutResolvedConcreteCandidate = FAOAIInventoryConcreteCandidateFacts();
	const FAOAIInventoryDecisionCandidateDefinition* BestCandidate = nullptr;

	for (const FAOAIInventoryDecisionCandidateDefinition& CandidateDefinition : ActionDefinition.CandidateDefinitions)
	{
		const FAOAIInventoryDecisionCandidateFacts CandidateFacts =
			BuildInventoryCandidateFacts(OwnerPawn, CandidateDefinition.UseCommand);
		if (!CandidateFacts.bHasUsableCandidate)
		{
			continue;
		}

		TArray<FAOAIInventoryConcreteCandidateFacts> ConcreteCandidates;
		BuildInventoryConcreteCandidateFacts(
			OwnerPawn,
			InventoryDecisionFacts,
			CandidateDefinition.UseCommand,
			ConcreteCandidates);

		for (const FAOAIInventoryConcreteCandidateFacts& ConcreteCandidate : ConcreteCandidates)
		{
			const float CandidateScore = ComputeInventoryCandidateScore(
				InventoryDecisionFacts,
				CandidateFactsByActionTag,
				ActionDefinition,
				CandidateDefinition,
				ConcreteCandidate);
			if (CandidateScore > OutCandidateScore)
			{
				OutCandidateScore = CandidateScore;
				OutResolvedConcreteCandidate = ConcreteCandidate;
				BestCandidate = &CandidateDefinition;
			}
		}
	}

	return BestCandidate;
}
}

// 库存决策主评估循环。
// 它负责：
// 1. 收集当前库存决策事实；
// 2. 汇总每个库存动作是否存在可用候选；
// 3. 先做动作层 Desire/Score 评估；
// 4. 再为胜出的动作挑选具体候选物品；
// 5. 最后把评估结果同步回 UAOAIDecisionComponent。
void FSTE_UpdateInventoryDecision::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	APawn* OwnerPawn = ResolveInventoryDecisionOwnerPawn(Context);
	UAOAIDecisionComponent* DecisionComponent = OwnerPawn ? UAOAIDecisionComponent::FindAIDecisionComponent(OwnerPawn) : nullptr;

	if (DecisionComponent == nullptr)
	{
		ResetInventoryDecisionOutputs(InstanceData);
		return;
	}

	DecisionComponent->EnsureDecisionDefinitionsInitialized();
	DecisionComponent->RefreshObservationContext();

	const UWorld* World = OwnerPawn ? OwnerPawn->GetWorld() : nullptr;
	const float CurrentWorldTimeSeconds = World ? World->GetTimeSeconds() : -1.0f;

	const FAOAIDecisionCombatFacts& CombatFacts = DecisionComponent->GetCombatFacts();

	FAOAIInventoryDecisionFacts InventoryDecisionFacts;
	InventoryDecisionFacts.HealthRatio = FMath::Clamp(
		DecisionComponent->GetAttributeValue(UAOHealthAttributeSet::GetHealthAttribute()) /
			FMath::Max(KINDA_SMALL_NUMBER, DecisionComponent->GetAttributeValue(UAOHealthAttributeSet::GetMaxHealthAttribute())),
		0.0f,
		1.0f);
	InventoryDecisionFacts.StaminaRatio = FMath::Clamp(
		DecisionComponent->GetAttributeValue(UAOCombatAttributeSet::GetStaminaAttribute()) /
			FMath::Max(KINDA_SMALL_NUMBER, DecisionComponent->GetAttributeValue(UAOCombatAttributeSet::GetMaxStaminaAttribute())),
		0.0f,
		1.0f);
	InventoryDecisionFacts.RecentDamageRatio = CombatFacts.RecentDamageRatio;
	InventoryDecisionFacts.bHasTarget = InstanceData.bHasTarget && InstanceData.CurrentTarget != nullptr;
	InventoryDecisionFacts.TargetDistance = InventoryDecisionFacts.bHasTarget ? FMath::Max(0.0f, InstanceData.DistanceToTarget) : 0.0f;
	InventoryDecisionFacts.SelfAIAttackRange = DecisionComponent->ResolveAIAttackRangeFromActor(OwnerPawn, 200.0f);
	InventoryDecisionFacts.bTargetInCombatWindow = CombatFacts.bTargetInCombatWindow;
	InventoryDecisionFacts.bTargetCombating = CombatFacts.bTargetCombating;
	InventoryDecisionFacts.bTargetInRecovery = CombatFacts.bTargetInRecovery;

	const float DistanceRatio =
		InventoryDecisionFacts.TargetDistance / FMath::Max(1.0f, InventoryDecisionFacts.SelfAIAttackRange);
	InventoryDecisionFacts.bCurrentWeaponFitsCombatDistance = !InventoryDecisionFacts.bHasTarget || DistanceRatio <= 1.0f;

	const TArray<FAOAIInventoryActionDefinition>& InventoryActionDefinitions = DecisionComponent->GetInventoryActionDefinitions();
	TMap<FGameplayTag, FAOAIInventoryDecisionCandidateFacts> CandidateFactsByActionTag;

	for (const FAOAIInventoryActionDefinition& ActionDefinition : InventoryActionDefinitions)
	{
		if (!ActionDefinition.ActionTag.IsValid())
		{
			continue;
		}

		FAOAIInventoryDecisionCandidateFacts MergedFacts;
		for (const FAOAIInventoryDecisionCandidateDefinition& CandidateDefinition : ActionDefinition.CandidateDefinitions)
		{
			const FAOAIInventoryDecisionCandidateFacts CandidateFacts =
				BuildInventoryCandidateFacts(OwnerPawn, CandidateDefinition.UseCommand);
			MergedFacts.CandidateCount += CandidateFacts.CandidateCount;
			MergedFacts.bHasAnyCandidate = MergedFacts.bHasAnyCandidate || CandidateFacts.bHasAnyCandidate;
			MergedFacts.bHasUsableCandidate = MergedFacts.bHasUsableCandidate || CandidateFacts.bHasUsableCandidate;
		}

		CandidateFactsByActionTag.Add(ActionDefinition.ActionTag, MergedFacts);
	}

	TMap<FGameplayTag, FAOAIInventoryDecisionRuntimeState> InventoryRuntimeStates = DecisionComponent->GetInventoryRuntimeStates();
	float BestScore = -1.0f;
	float BestFallbackWeight = -1.0f;
	FGameplayTag BestActionTag;

	for (const FAOAIInventoryActionDefinition& Definition : InventoryActionDefinitions)
	{
		if (!Definition.ActionTag.IsValid())
		{
			continue;
		}

		FAOAIInventoryDecisionRuntimeState& RuntimeState = InventoryRuntimeStates.FindOrAdd(Definition.ActionTag);
		RuntimeState.Desire = 0.0f;
		RuntimeState.Score = 0.0f;

		if (!CanEvaluateInventoryAction(InventoryDecisionFacts, InstanceData.TacticalState, CandidateFactsByActionTag, Definition))
		{
			continue;
		}

		const float TimeSinceLastExecution = (RuntimeState.LastExecutedTime < 0.0f || CurrentWorldTimeSeconds < 0.0f)
			? TNumericLimits<float>::Max()
			: FMath::Max(0.0f, CurrentWorldTimeSeconds - RuntimeState.LastExecutedTime);
		const float Desire = ComputeInventoryActionDesire(
			*DecisionComponent,
			CombatFacts,
			InventoryDecisionFacts,
			Definition,
			TimeSinceLastExecution,
			DistanceRatio,
			CurrentWorldTimeSeconds);
		const float Score = ComputeInventoryActionScore(
			Definition,
			Definition.ActionTag,
			Desire,
			TimeSinceLastExecution,
			DecisionComponent->GetInventoryDecisionExecutionRecord());
		RuntimeState.Desire = Desire;
		RuntimeState.Score = Score;

		if (Score > BestScore || (FMath::IsNearlyEqual(Score, BestScore) && Definition.FallbackSelectionWeight > BestFallbackWeight))
		{
			BestScore = Score;
			BestFallbackWeight = Definition.FallbackSelectionWeight;
			BestActionTag = Definition.ActionTag;
		}
	}

	FAOAIInventoryDecisionResult EvaluationInventoryDecisionResult;
	const FGameplayTag SelectedActionTag = BestScore <= 0.0f
		? ResolveFallbackInventoryActionTag(
			InventoryDecisionFacts,
			InstanceData.TacticalState,
			CandidateFactsByActionTag,
			InventoryActionDefinitions)
		: BestActionTag;
	const FAOAIInventoryActionDefinition* SelectedActionDefinition =
		FindInventoryActionDefinitionByTag(InventoryActionDefinitions, SelectedActionTag);

	if (SelectedActionDefinition != nullptr)
	{
		const FAOAIInventoryDecisionRuntimeState* RuntimeState = InventoryRuntimeStates.Find(SelectedActionTag);
		float CandidateScore = 0.0f;
		FAOAIInventoryConcreteCandidateFacts ResolvedConcreteCandidate;
		const FAOAIInventoryDecisionCandidateDefinition* CandidateDefinition = FindBestInventoryCandidate(
			OwnerPawn,
			InventoryDecisionFacts,
			CandidateFactsByActionTag,
			*SelectedActionDefinition,
			CandidateScore,
			ResolvedConcreteCandidate);

		if (RuntimeState != nullptr && CandidateDefinition != nullptr && CandidateScore > 0.0f)
		{
			EvaluationInventoryDecisionResult.bHasAction = true;
			EvaluationInventoryDecisionResult.ActionTag = SelectedActionDefinition->ActionTag;
			EvaluationInventoryDecisionResult.CandidateTag = CandidateDefinition->CandidateTag;
			EvaluationInventoryDecisionResult.CoordinationMode = SelectedActionDefinition->CoordinationMode;
			EvaluationInventoryDecisionResult.Desire = RuntimeState->Desire;
			EvaluationInventoryDecisionResult.Score = RuntimeState->Score;
			EvaluationInventoryDecisionResult.UseCommand = CandidateDefinition->UseCommand;

			if (ResolvedConcreteCandidate.ResolvedTarget.bUsedQuickBarSlot)
			{
				EvaluationInventoryDecisionResult.UseCommand.QuickBarSlotIndex =
					ResolvedConcreteCandidate.ResolvedTarget.QuickBarSlotIndex;
				EvaluationInventoryDecisionResult.UseCommand.QuickBarSlotIndices.Reset();
				EvaluationInventoryDecisionResult.UseCommand.QuickBarSlotIndices.Add(
					ResolvedConcreteCandidate.ResolvedTarget.QuickBarSlotIndex);
			}

			EvaluationInventoryDecisionResult.bHasResolvedTarget = true;
			EvaluationInventoryDecisionResult.ResolvedTarget = ResolvedConcreteCandidate.ResolvedTarget;
		}
	}

	DecisionComponent->CacheInventoryEvaluation(
		CurrentWorldTimeSeconds,
		InventoryDecisionFacts,
		CandidateFactsByActionTag,
		InventoryRuntimeStates,
		EvaluationInventoryDecisionResult);

	InstanceData.bHasCurrentSubmittedInventoryDecision =
		DecisionComponent->GetCurrentSubmittedInventoryDecisionResult(InstanceData.CurrentSubmittedInventoryDecision);
	InstanceData.bHasAdditiveInventoryWindow = InstanceData.TacticalState.bHasAdditiveInventoryWindow;
}
