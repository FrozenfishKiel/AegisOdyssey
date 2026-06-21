#include "STE_UpdateInventoryDecision.h"

#include "AbilitySystemComponent.h"
#include "AegisOdyssey/AbilitySystem/Attributes/Combat/AOCombatAttributeSet.h"
#include "AegisOdyssey/AbilitySystem/Attributes/Core/AOHealthAttributeSet.h"
#include "AegisOdyssey/Character/Enemies/AI/Decision/AOAIDecisionComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "StateTreeExecutionContext.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(STE_UpdateInventoryDecision)

namespace
{
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

float EvaluateInventoryDecisionAttributeIntervalFactor(
	const UAOAIDecisionComponent& DecisionComponent,
	const FAOAIDecisionAttributeIntervalFactor& Factor)
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

float ComputeInventoryDesire(
	const UAOAIDecisionComponent& DecisionComponent,
	const FAOAIDecisionCombatFacts& CombatFacts,
	const FAOAIInventoryDecisionFacts& InventoryDecisionFacts,
	const FAOAIInventoryDesireDefinition& Definition,
	const float TimeSinceLastExecution,
	const float CurrentWorldTimeSeconds)
{
	const float SafeCadenceSeconds = FMath::Max(KINDA_SMALL_NUMBER, Definition.CadenceSeconds);
	const float CadenceAlpha = FMath::Clamp(TimeSinceLastExecution / SafeCadenceSeconds, 0.0f, 1.0f);
	const float RecentDamageRatio = DecisionComponent.GetRecentDamageRatio(
		CurrentWorldTimeSeconds,
		Definition.RecentDamageWindowSeconds);

	float Desire = Definition.BaseDesire;
	Desire += CadenceAlpha * Definition.CadenceWeight;
	Desire += EvaluateInventoryDecisionResponseCurveFactor(Definition.HealthRatioFactor, InventoryDecisionFacts.HealthRatio);
	Desire += EvaluateInventoryDecisionResponseCurveFactor(Definition.StaminaRatioFactor, InventoryDecisionFacts.StaminaRatio);
	Desire += EvaluateInventoryDecisionResponseCurveFactor(Definition.DistanceFactor, InventoryDecisionFacts.TargetDistance);
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

float ComputeInventoryScore(
	const FAOAIInventoryDesireDefinition& Definition,
	const FGameplayTag& DesiredTag,
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
		if (ExecutionRecord.LastExecutedActionTag.MatchesTagExact(DesiredTag))
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
}

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
	InventoryDecisionFacts.bTargetInCombatWindow = CombatFacts.bTargetInCombatWindow;
	InventoryDecisionFacts.bTargetCombating = CombatFacts.bTargetCombating;
	InventoryDecisionFacts.bTargetInRecovery = CombatFacts.bTargetInRecovery;

	const TArray<FAOAIInventoryDesireDefinition>& InventoryDesireDefinitions =
		DecisionComponent->GetInventoryDesireDefinitions();
	TMap<FGameplayTag, FAOAIInventoryDecisionRuntimeState> InventoryRuntimeStates =
		DecisionComponent->GetInventoryRuntimeStates();

	TSet<FGameplayTag> EvaluatedDesiredTags;
	float BestScore = 0.0f;
	FGameplayTag BestDesiredTag;

	for (const FAOAIInventoryDesireDefinition& Definition : InventoryDesireDefinitions)
	{
		if (!Definition.DesiredTag.IsValid() || EvaluatedDesiredTags.Contains(Definition.DesiredTag))
		{
			continue;
		}

		EvaluatedDesiredTags.Add(Definition.DesiredTag);

		FAOAIInventoryDecisionRuntimeState& RuntimeState = InventoryRuntimeStates.FindOrAdd(Definition.DesiredTag);
		RuntimeState.Desire = 0.0f;
		RuntimeState.Score = 0.0f;

		if (Definition.bRequireTarget && !InventoryDecisionFacts.bHasTarget)
		{
			continue;
		}

		const float TimeSinceLastExecution = (RuntimeState.LastExecutedTime < 0.0f || CurrentWorldTimeSeconds < 0.0f)
			? TNumericLimits<float>::Max()
			: FMath::Max(0.0f, CurrentWorldTimeSeconds - RuntimeState.LastExecutedTime);
		RuntimeState.Desire = ComputeInventoryDesire(
			*DecisionComponent,
			CombatFacts,
			InventoryDecisionFacts,
			Definition,
			TimeSinceLastExecution,
			CurrentWorldTimeSeconds);
		RuntimeState.Score = ComputeInventoryScore(
			Definition,
			Definition.DesiredTag,
			RuntimeState.Desire,
			TimeSinceLastExecution,
			DecisionComponent->GetInventoryDecisionExecutionRecord());

		UE_LOG(
			LogStateTree,
			Log,
			TEXT("FSTE_UpdateInventoryDecision::Tick DesireTag=%s Desire=%.3f Score=%.3f TimeSinceLastExecution=%.3f HasTarget=%s"),
			*Definition.DesiredTag.ToString(),
			RuntimeState.Desire,
			RuntimeState.Score,
			TimeSinceLastExecution,
			InventoryDecisionFacts.bHasTarget ? TEXT("true") : TEXT("false"));

		if (RuntimeState.Score > BestScore)
		{
			BestScore = RuntimeState.Score;
			BestDesiredTag = Definition.DesiredTag;
		}
	}

	FAOAIInventoryDecisionResult EvaluationInventoryDecisionResult;
	if (BestDesiredTag.IsValid() && BestScore > 0.0f)
	{
		const FAOAIInventoryDecisionRuntimeState* RuntimeState = InventoryRuntimeStates.Find(BestDesiredTag);
		EvaluationInventoryDecisionResult.bHasAction = true;
		EvaluationInventoryDecisionResult.ActionTag = BestDesiredTag;
		EvaluationInventoryDecisionResult.Desire = RuntimeState ? RuntimeState->Desire : 0.0f;
		EvaluationInventoryDecisionResult.Score = RuntimeState ? RuntimeState->Score : BestScore;
	}

	UE_LOG(
		LogStateTree,
		Log,
		TEXT("FSTE_UpdateInventoryDecision::Tick EvaluationResult HasAction=%s ActionTag=%s Score=%.3f Desire=%.3f"),
		EvaluationInventoryDecisionResult.bHasAction ? TEXT("true") : TEXT("false"),
		*EvaluationInventoryDecisionResult.ActionTag.ToString(),
		EvaluationInventoryDecisionResult.Score,
		EvaluationInventoryDecisionResult.Desire);

	DecisionComponent->CacheInventoryEvaluation(
		CurrentWorldTimeSeconds,
		InventoryDecisionFacts,
		InventoryRuntimeStates,
		EvaluationInventoryDecisionResult);

	InstanceData.bHasCurrentSubmittedInventoryDecision =
		DecisionComponent->GetCurrentSubmittedInventoryDecisionResult(InstanceData.CurrentSubmittedInventoryDecision);
}
