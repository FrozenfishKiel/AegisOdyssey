// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "AegisOdyssey/Character/Enemies/AI/Decision/AOAIDecisionTypes.h"
#include "AOAIInventoryDecisionTypes.generated.h"

USTRUCT(BlueprintType)
struct FAOAIInventoryDecisionFacts
{
	GENERATED_BODY()

	// Self health ratio observed by the inventory desire evaluator.
	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	float HealthRatio = 1.0f;

	// Self stamina ratio observed by the inventory desire evaluator.
	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	float StaminaRatio = 1.0f;

	// Recent received damage ratio observed by the inventory desire evaluator.
	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	float RecentDamageRatio = 0.0f;

	// Whether the current decision frame has a valid target.
	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	bool bHasTarget = false;

	// Current target distance. Meaningful only when bHasTarget is true.
	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	float TargetDistance = 0.0f;

	// Whether the target is currently inside a useful combat window.
	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	bool bTargetInCombatWindow = false;

	// Whether the target is currently performing stable combat.
	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	bool bTargetCombating = false;

	// Whether the target is currently in recovery.
	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	bool bTargetInRecovery = false;
};

USTRUCT(BlueprintType)
struct FAOAIInventoryDesireDefinition
{
	GENERATED_BODY()

	// The single semantic tag STE wants to submit, usually an item semantic tag such as Item.Semantic.Weapon.
	UPROPERTY(EditAnywhere, Category = "Config")
	FGameplayTag DesiredTag;

	// If true, this desire is evaluated only when a target exists.
	UPROPERTY(EditAnywhere, Category = "Config")
	bool bRequireTarget = false;

	// Base desire before runtime factors are added.
	UPROPERTY(EditAnywhere, Category = "Config")
	float BaseDesire = 0.0f;

	// Desire recovered by time since this desired tag was last executed.
	UPROPERTY(EditAnywhere, Category = "Config")
	float CadenceWeight = 0.0f;

	// Time span used to normalize cadence recovery.
	UPROPERTY(EditAnywhere, Category = "Config", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float CadenceSeconds = 1.0f;

	// Health-ratio response factor.
	UPROPERTY(EditAnywhere, Category = "Config")
	FAOAIDecisionResponseCurveFactor HealthRatioFactor;

	// Stamina-ratio response factor.
	UPROPERTY(EditAnywhere, Category = "Config")
	FAOAIDecisionResponseCurveFactor StaminaRatioFactor;

	// Distance response factor. The raw input is current target distance.
	UPROPERTY(EditAnywhere, Category = "Config")
	FAOAIDecisionResponseCurveFactor DistanceFactor;

	// Recent-damage response factor.
	UPROPERTY(EditAnywhere, Category = "Config")
	FAOAIDecisionResponseCurveFactor RecentDamageFactor;

	// Time window used when sampling recent damage for this desire.
	UPROPERTY(EditAnywhere, Category = "Config", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float RecentDamageWindowSeconds = 2.0f;

	// Attribute interval factors that affect desire.
	UPROPERTY(EditAnywhere, Category = "Config")
	TArray<FAOAIDecisionAttributeIntervalFactor> AttributeIntervalFactors;

	// Target-state tag factors that affect desire.
	UPROPERTY(EditAnywhere, Category = "Config")
	TArray<FAOAIDecisionTagScoreFactor> TargetStateTagFactors;

	// Cooldown after this desired tag has been executed.
	UPROPERTY(EditAnywhere, Category = "Config", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float CooldownSeconds = 0.0f;

	// Score multiplier while this desired tag is still cooling down.
	UPROPERTY(EditAnywhere, Category = "Config", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float CooldownPenaltyMultiplier = 1.0f;

	// Repeated execution penalty for the same desired tag.
	UPROPERTY(EditAnywhere, Category = "Config")
	float RepeatPenalty = 0.0f;

	// Bonus when switching away from the previously executed desired tag.
	UPROPERTY(EditAnywhere, Category = "Config")
	float SwitchBonus = 0.0f;
};

USTRUCT(BlueprintType)
struct FAOAIInventoryDecisionRuntimeState
{
	GENERATED_BODY()

	// Desire calculated for this tag in the current frame.
	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	float Desire = 0.0f;

	// Final score calculated for this tag in the current frame.
	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	float Score = 0.0f;

	// Last time this tag was formally executed.
	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	float LastExecutedTime = -1.0f;
};

USTRUCT(BlueprintType)
struct FAOAIInventoryDecisionResult
{
	GENERATED_BODY()

	// Whether STE currently has a desired inventory semantic tag.
	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	bool bHasAction = false;

	// The single semantic tag submitted by STE.
	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	FGameplayTag ActionTag;

	// Desire snapshot for debugging and tuning.
	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	float Desire = 0.0f;

	// Score snapshot for debugging and tuning.
	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	float Score = 0.0f;
};

USTRUCT(BlueprintType)
struct FAOAIInventoryDecisionExecutionRecord
{
	GENERATED_BODY()

	// Most recent inventory semantic tag executed by STT.
	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	FGameplayTag LastExecutedActionTag;

	// Repeated execution count for the same semantic tag.
	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	int32 RepeatedActionCount = 0;
};
