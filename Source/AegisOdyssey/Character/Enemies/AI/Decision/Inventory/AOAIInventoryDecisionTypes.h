// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AttributeSet.h"
#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "AegisOdyssey/Character/Enemies/AI/AOAIInventoryUseTypes.h"
#include "AegisOdyssey/Character/Enemies/AI/Decision/AOAIDecisionTypes.h"
#include "AOAIInventoryDecisionTypes.generated.h"

class UAOInventoryComponent;

UENUM(BlueprintType)
enum class EAOAIInventoryActionCoordinationMode : uint8
{
	Exclusive UMETA(DisplayName = "Exclusive"),
	Additive UMETA(DisplayName = "Additive")
};

USTRUCT(BlueprintType)
struct FAOAIInventoryDecisionCandidateFacts
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	int32 CandidateCount = 0;

	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	bool bHasAnyCandidate = false;

	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	bool bHasUsableCandidate = false;
};

USTRUCT(BlueprintType)
struct FAOAIInventoryConcreteCandidateFacts
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	FAOAIResolvedInventoryUseTarget ResolvedTarget;

	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	int32 StackCount = 0;

	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	float WeaponAIAttackRange = 0.0f;

	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	bool bMatchesCurrentCombatDistance = false;
};

USTRUCT(BlueprintType)
struct FAOAIInventoryDecisionFacts
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	float HealthRatio = 1.0f;

	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	float StaminaRatio = 1.0f;

	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	float RecentDamageRatio = 0.0f;

	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	bool bHasTarget = false;

	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	float TargetDistance = 0.0f;

	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	float SelfAIAttackRange = 0.0f;

	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	bool bCurrentWeaponFitsCombatDistance = false;

	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	bool bTargetInCombatWindow = false;

	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	bool bTargetCombating = false;

	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	bool bTargetInRecovery = false;
};

USTRUCT(BlueprintType)
struct FAOAIInventoryDecisionCandidateDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Config")
	FGameplayTag CandidateTag;

	UPROPERTY(EditAnywhere, Category = "Config")
	FAOAIInventoryUseCommand UseCommand;

	UPROPERTY(EditAnywhere, Category = "Config")
	float BaseScore = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Config")
	float DesiredHealthDeficitWeight = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Config")
	float DesiredStaminaDeficitWeight = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Config")
	float DesiredRecentDamageWeight = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Config")
	float DesiredOutOfRangeWeight = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Config")
	float DesireCombatWindowWeight = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Config")
	float DesireTargetRecoveryWeight = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Config")
	float DesireNeedWeaponSwapWeight = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Config")
	float DesiredHighStackCountWeight = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Config")
	float DesiredDistanceFitWeight = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Config")
	float PreferLowerSlotIndexWeight = 0.0f;
};

USTRUCT(BlueprintType)
struct FAOAIInventoryActionDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Config")
	FGameplayTag ActionTag;

	UPROPERTY(EditAnywhere, Category = "Config")
	EAOAIInventoryActionCoordinationMode CoordinationMode = EAOAIInventoryActionCoordinationMode::Exclusive;

	UPROPERTY(EditAnywhere, Category = "Config")
	bool bRequireTarget = false;

	UPROPERTY(EditAnywhere, Category = "Config")
	FGameplayTagContainer RequiredMainIntentTags;

	UPROPERTY(EditAnywhere, Category = "Config")
	FGameplayTagContainer BlockedMainIntentTags;

	UPROPERTY(EditAnywhere, Category = "Config")
	FGameplayTagContainer RequiredTacticalTags;

	UPROPERTY(EditAnywhere, Category = "Config")
	FGameplayTagContainer BlockedTacticalTags;

	UPROPERTY(EditAnywhere, Category = "Config")
	float BaseDesire = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Config")
	float CadenceWeight = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Config", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float CadenceSeconds = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Config")
	FAOAIDecisionResponseCurveFactor HealthRatioFactor;

	UPROPERTY(EditAnywhere, Category = "Config")
	FAOAIDecisionResponseCurveFactor StaminaRatioFactor;

	UPROPERTY(EditAnywhere, Category = "Config")
	FAOAIDecisionResponseCurveFactor DistanceFactor;

	UPROPERTY(EditAnywhere, Category = "Config")
	FAOAIDecisionResponseCurveFactor RecentDamageFactor;

	UPROPERTY(EditAnywhere, Category = "Config", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float RecentDamageWindowSeconds = 2.0f;

	UPROPERTY(EditAnywhere, Category = "Config")
	TArray<FAOAIDecisionAttributeIntervalFactor> AttributeIntervalFactors;

	UPROPERTY(EditAnywhere, Category = "Config")
	TArray<FAOAIDecisionTagScoreFactor> TargetStateTagFactors;

	UPROPERTY(EditAnywhere, Category = "Config", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float CooldownSeconds = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Config", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float CooldownPenaltyMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Config")
	float RepeatPenalty = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Config")
	float SwitchBonus = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Config")
	float FallbackSelectionWeight = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Config")
	TArray<FAOAIInventoryDecisionCandidateDefinition> CandidateDefinitions;
};

USTRUCT(BlueprintType)
struct FAOAIInventoryDecisionRuntimeState
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	float Desire = 0.0f;

	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	float Score = 0.0f;

	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	float LastExecutedTime = -1.0f;
};

USTRUCT(BlueprintType)
struct FAOAIInventoryDecisionResult
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	bool bHasAction = false;

	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	FGameplayTag ActionTag;

	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	FGameplayTag CandidateTag;

	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	EAOAIInventoryActionCoordinationMode CoordinationMode = EAOAIInventoryActionCoordinationMode::Exclusive;

	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	float Desire = 0.0f;

	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	float Score = 0.0f;

	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	FAOAIInventoryUseCommand UseCommand;

	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	bool bHasResolvedTarget = false;

	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	FAOAIResolvedInventoryUseTarget ResolvedTarget;
};

USTRUCT(BlueprintType)
struct FAOAIInventoryDecisionExecutionRecord
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	FGameplayTag LastExecutedActionTag;

	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	int32 RepeatedActionCount = 0;
};
