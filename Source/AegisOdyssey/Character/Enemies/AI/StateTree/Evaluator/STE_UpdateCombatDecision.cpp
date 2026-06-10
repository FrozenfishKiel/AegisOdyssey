#include "STE_UpdateCombatDecision.h"

#include "AbilitySystemComponent.h"
#include "AegisOdyssey/AOGameplayTags.h"
#include "AegisOdyssey/AOStateTags.h"
#include "AegisOdyssey/Character/Enemies/AI/Decision/AOAIDecisionComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "StateTreeExecutionContext.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(STE_UpdateCombatDecision)

namespace
{
void ResetDecisionOutputs(FUpdateCombatDecisionInstanceData& InstanceData)
{
	InstanceData.SelectedIntentTag = FGameplayTag();
	InstanceData.SelectedIntentDesire = 0.0f;
	InstanceData.SelectedIntentScore = 0.0f;
	InstanceData.ResolvedObservedIntentTag = FGameplayTag();
	InstanceData.ObservedIntentDesire = 0.0f;
	InstanceData.ObservedIntentScore = 0.0f;
	InstanceData.bHasObservedIntentMetrics = false;
	InstanceData.LastExecutedIntentTag = FGameplayTag();
	InstanceData.RepeatedIntentCount = 0;
	InstanceData.TacticalState = FAOAIDecisionTacticalState();
}

APawn* ResolveOwnerPawn(const FStateTreeExecutionContext& Context)
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

bool DoesCurrentIntentMatchAnyTag(const FGameplayTag& CurrentIntentTag, const FGameplayTagContainer& IntentTags)
{
	return CurrentIntentTag.IsValid() && !IntentTags.IsEmpty() && IntentTags.HasTag(CurrentIntentTag);
}

bool ShouldOpenAdditiveInventoryWindow(const FGameplayTag& CurrentIntentTag)
{
	if (!CurrentIntentTag.IsValid())
	{
		return true;
	}

	if (CurrentIntentTag.MatchesTag(AOGameplayTags::AI_Intent_Strafe))
	{
		return true;
	}

	if (CurrentIntentTag.MatchesTag(AOGameplayTags::AI_Intent_Attack)
		|| CurrentIntentTag.MatchesTag(AOGameplayTags::AI_Intent_Roll))
	{
		return false;
	}

	return false;
}

float EvaluateResponseCurveFactor(const FAOAIDecisionResponseCurveFactor& Factor, const float RawInput)
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

float EvaluateAttributeIntervalFactor(const UAOAIDecisionComponent& DecisionComponent, const FAOAIDecisionAttributeIntervalFactor& Factor)
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

float ComputeIntentDesire(
	const UAOAIDecisionComponent& DecisionComponent,
	const FAOAIDecisionCombatFacts& CombatFacts,
	const FAOAIDecisionIntentDefinition& Definition,
	const float TimeSinceLastExecution,
	const float DistanceRatio,
	const float CurrentWorldTimeSeconds)
{
	const float SafeCadenceSeconds = FMath::Max(KINDA_SMALL_NUMBER, Definition.CadenceSeconds);
	const float CadenceAlpha = FMath::Clamp(TimeSinceLastExecution / SafeCadenceSeconds, 0.0f, 1.0f);
	const float RecentDamageRatio = DecisionComponent.GetRecentDamageRatio(CurrentWorldTimeSeconds, Definition.RecentDamageWindowSeconds);

	float Desire = Definition.BaseDesire;
	Desire += CadenceAlpha * Definition.CadenceWeight;
	Desire += EvaluateResponseCurveFactor(Definition.DistanceFactor, DistanceRatio);
	Desire += EvaluateResponseCurveFactor(Definition.RecentDamageFactor, RecentDamageRatio);

	for (const FAOAIDecisionAttributeIntervalFactor& AttributeFactor : Definition.AttributeIntervalFactors)
	{
		Desire += EvaluateAttributeIntervalFactor(DecisionComponent, AttributeFactor);
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

float ComputeIntentScore(
	const FAOAIDecisionIntentDefinition& Definition,
	const FGameplayTag& IntentTag,
	const float Desire,
	const float TimeSinceLastExecution,
	const FGameplayTag& LastExecutedIntentTag,
	const int32 RepeatedIntentCount)
{
	float Score = Desire;

	if (TimeSinceLastExecution < Definition.CooldownSeconds)
	{
		Score *= Definition.CooldownPenaltyMultiplier;
	}

	if (LastExecutedIntentTag.IsValid())
	{
		if (LastExecutedIntentTag.MatchesTagExact(IntentTag))
		{
			Score -= Definition.RepeatPenalty * RepeatedIntentCount;
		}
		else
		{
			Score += Definition.SwitchBonus;
		}
	}

	return FMath::Max(0.0f, Score);
}

bool CanEvaluateIntent(const FAOAIDecisionCombatFacts& CombatFacts, const FAOAIDecisionIntentDefinition& Definition)
{
	if (Definition.bRequireTarget && !CombatFacts.bHasTarget)
	{
		return false;
	}

	return true;
}

FGameplayTag ResolveFallbackIntentTag(
	const FAOAIDecisionCombatFacts& CombatFacts,
	const TArray<FAOAIDecisionIntentDefinition>& IntentDefinitions)
{
	float BestFallbackWeight = -1.0f;
	FGameplayTag FallbackIntentTag;

	for (const FAOAIDecisionIntentDefinition& Definition : IntentDefinitions)
	{
		if (!Definition.IntentTag.IsValid() || !CanEvaluateIntent(CombatFacts, Definition))
		{
			continue;
		}

		if (Definition.FallbackSelectionWeight > BestFallbackWeight)
		{
			BestFallbackWeight = Definition.FallbackSelectionWeight;
			FallbackIntentTag = Definition.IntentTag;
		}
	}

	return FallbackIntentTag;
}

FAOAIDecisionTacticalState BuildTacticalState(
	const FAOAIDecisionCombatFacts& CombatFacts,
	const FGameplayTag& SelectedIntentTag)
{
	FAOAIDecisionTacticalState TacticalState;
	TacticalState.CurrentMainIntentTag = SelectedIntentTag;

	if (CombatFacts.bHasTarget)
	{
		TacticalState.TacticalTags.AddTag(AOGameplayTags::AI_Tactical_HasTarget);
	}
	else
	{
		TacticalState.TacticalTags.AddTag(AOGameplayTags::AI_Tactical_NoTarget);
	}

	if (CombatFacts.bIsInAttackRange)
	{
		TacticalState.TacticalTags.AddTag(AOGameplayTags::AI_Tactical_Target_InRange);
	}
	else
	{
		TacticalState.TacticalTags.AddTag(AOGameplayTags::AI_Tactical_Target_OutOfRange);
	}

	if (CombatFacts.bTargetInPreparation)
	{
		TacticalState.TacticalTags.AddTag(AOGameplayTags::AI_Tactical_Target_Preparation);
	}

	if (CombatFacts.bTargetInCombatWindow)
	{
		TacticalState.TacticalTags.AddTag(AOGameplayTags::AI_Tactical_Target_CombatWindow);
	}

	if (CombatFacts.bTargetCombating)
	{
		TacticalState.TacticalTags.AddTag(AOGameplayTags::AI_Tactical_Target_Combating);
	}

	if (CombatFacts.bTargetInRecovery)
	{
		TacticalState.TacticalTags.AddTag(AOGameplayTags::AI_Tactical_Target_Recovery);
	}

	const float DistanceRatio = CombatFacts.TargetDistance / FMath::Max(1.0f, CombatFacts.SelfAIAttackRange);
	if (!CombatFacts.bHasTarget || DistanceRatio <= 1.0f)
	{
		TacticalState.TacticalTags.AddTag(AOGameplayTags::AI_Tactical_Self_WeaponFitsCombatDistance);
	}
	else
	{
		TacticalState.TacticalTags.AddTag(AOGameplayTags::AI_Tactical_Self_NeedWeaponSwap);
	}

	TacticalState.bHasAdditiveInventoryWindow = ShouldOpenAdditiveInventoryWindow(SelectedIntentTag);
	if (TacticalState.bHasAdditiveInventoryWindow)
	{
		TacticalState.TacticalTags.AddTag(AOGameplayTags::AI_Tactical_Inventory_AdditiveWindow);
	}

	return TacticalState;
}
}

void FSTE_UpdateCombatDecision::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	APawn* OwnerPawn = ResolveOwnerPawn(Context);
	UAOAIDecisionComponent* DecisionComponent = OwnerPawn ? UAOAIDecisionComponent::FindAIDecisionComponent(OwnerPawn) : nullptr;

	if (DecisionComponent == nullptr)
	{
		ResetDecisionOutputs(InstanceData);
		return;
	}

	DecisionComponent->EnsureDecisionDefinitionsInitialized();
	DecisionComponent->RefreshObservationContext();

	const float CurrentWorldTimeSeconds = OwnerPawn && OwnerPawn->GetWorld() ? OwnerPawn->GetWorld()->GetTimeSeconds() : -1.0f;

	FAOAIDecisionCombatFacts CombatFacts;
	CombatFacts.CurrentTarget = InstanceData.CurrentTarget;
	CombatFacts.bHasTarget = InstanceData.bHasTarget && InstanceData.CurrentTarget != nullptr;
	CombatFacts.TargetDistance = CombatFacts.bHasTarget ? FMath::Max(0.0f, InstanceData.DistanceToTarget) : 0.0f;
	CombatFacts.bIsInAttackRange = CombatFacts.bHasTarget && InstanceData.bIsInAttackRange;
	CombatFacts.SelfAIAttackRange = DecisionComponent->ResolveAIAttackRangeFromActor(OwnerPawn, 200.0f);
	CombatFacts.TargetAIAttackRange = DecisionComponent->ResolveAIAttackRangeFromActor(InstanceData.CurrentTarget, 200.0f);
	CombatFacts.RecentDamageRatio = DecisionComponent->GetRecentDamageRatio(CurrentWorldTimeSeconds, 0.0f);
	CombatFacts.bTargetInPreparation = DecisionComponent->HasMatchingTag(InstanceData.CurrentTarget, AOStateTags::State_Combat_Preparation, false);
	CombatFacts.bTargetInCombatWindow = DecisionComponent->HasMatchingTag(InstanceData.CurrentTarget, AOStateTags::State_Combat_CombatWindow, false);
	CombatFacts.bTargetCombating = DecisionComponent->HasMatchingTag(InstanceData.CurrentTarget, AOStateTags::State_Combat_Combating, false);
	CombatFacts.bTargetInRecovery = DecisionComponent->HasMatchingTag(InstanceData.CurrentTarget, AOStateTags::State_Combat_Recovery, false);

	TMap<FGameplayTag, FAOAIDecisionIntentRuntimeState> IntentRuntimeStates = DecisionComponent->GetIntentRuntimeStates();

	const float DistanceRatio = CombatFacts.TargetDistance / FMath::Max(1.0f, CombatFacts.SelfAIAttackRange);
	float BestScore = -1.0f;
	float BestFallbackWeight = -1.0f;
	FGameplayTag BestIntentTag;

	const TArray<FAOAIDecisionIntentDefinition>& IntentDefinitions = DecisionComponent->GetIntentDefinitions();
	for (const FAOAIDecisionIntentDefinition& Definition : IntentDefinitions)
	{
		if (!Definition.IntentTag.IsValid())
		{
			continue;
		}

		FAOAIDecisionIntentRuntimeState& RuntimeState = IntentRuntimeStates.FindOrAdd(Definition.IntentTag);
		RuntimeState.Desire = 0.0f;
		RuntimeState.Score = 0.0f;

		if (!CanEvaluateIntent(CombatFacts, Definition))
		{
			continue;
		}

		const float TimeSinceLastExecution = (RuntimeState.LastExecutedTime < 0.0f || CurrentWorldTimeSeconds < 0.0f)
			? TNumericLimits<float>::Max()
			: FMath::Max(0.0f, CurrentWorldTimeSeconds - RuntimeState.LastExecutedTime);
		const float Desire = ComputeIntentDesire(
			*DecisionComponent,
			CombatFacts,
			Definition,
			TimeSinceLastExecution,
			DistanceRatio,
			CurrentWorldTimeSeconds);
		const float Score = ComputeIntentScore(
			Definition,
			Definition.IntentTag,
			Desire,
			TimeSinceLastExecution,
			DecisionComponent->GetLastExecutedIntentTag(),
			DecisionComponent->GetRepeatedIntentCount());
		RuntimeState.Desire = Desire;
		RuntimeState.Score = Score;

		if (Score > BestScore || (FMath::IsNearlyEqual(Score, BestScore) && Definition.FallbackSelectionWeight > BestFallbackWeight))
		{
			BestScore = Score;
			BestFallbackWeight = Definition.FallbackSelectionWeight;
			BestIntentTag = Definition.IntentTag;
		}
	}

	const FGameplayTag SelectedIntentTag =
		BestScore <= 0.0f ? ResolveFallbackIntentTag(CombatFacts, IntentDefinitions) : BestIntentTag;
	const FAOAIDecisionTacticalState TacticalState = BuildTacticalState(CombatFacts, SelectedIntentTag);

	DecisionComponent->CacheCombatEvaluation(
		CurrentWorldTimeSeconds,
		CombatFacts,
		IntentRuntimeStates,
		SelectedIntentTag,
		TacticalState);

	InstanceData.SelectedIntentTag = SelectedIntentTag;
	if (const FAOAIDecisionIntentRuntimeState* SelectedRuntimeState = IntentRuntimeStates.Find(SelectedIntentTag))
	{
		InstanceData.SelectedIntentDesire = SelectedRuntimeState->Desire;
		InstanceData.SelectedIntentScore = SelectedRuntimeState->Score;
	}
	else
	{
		InstanceData.SelectedIntentDesire = 0.0f;
		InstanceData.SelectedIntentScore = 0.0f;
	}

	InstanceData.LastExecutedIntentTag = DecisionComponent->GetLastExecutedIntentTag();
	InstanceData.RepeatedIntentCount = DecisionComponent->GetRepeatedIntentCount();
	InstanceData.TacticalState = TacticalState;

	const FGameplayTag ObservedIntentTag = InstanceData.ObservedIntentTag.IsValid()
		? InstanceData.ObservedIntentTag
		: SelectedIntentTag;
	InstanceData.ResolvedObservedIntentTag = ObservedIntentTag;

	if (const FAOAIDecisionIntentRuntimeState* ObservedRuntimeState = IntentRuntimeStates.Find(ObservedIntentTag))
	{
		InstanceData.bHasObservedIntentMetrics = true;
		InstanceData.ObservedIntentDesire = ObservedRuntimeState->Desire;
		InstanceData.ObservedIntentScore = ObservedRuntimeState->Score;
	}
	else
	{
		InstanceData.bHasObservedIntentMetrics = false;
		InstanceData.ObservedIntentDesire = 0.0f;
		InstanceData.ObservedIntentScore = 0.0f;
	}
}
