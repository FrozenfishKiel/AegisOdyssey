// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "AegisOdyssey/Character/Enemies/AI/Decision/Inventory/AOAIInventoryDecisionTypes.h"
#include "StateTreeConditionBase.h"
#include "STC_AIPendingInventoryDecisionMatches.generated.h"

USTRUCT()
struct FAIPendingInventoryDecisionMatchesInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Input")
	bool bHasCurrentSubmittedInventoryDecision = false;

	UPROPERTY(EditAnywhere, Category = "Input")
	FAOAIInventoryDecisionResult CurrentSubmittedInventoryDecision;

	UPROPERTY(EditAnywhere, Category = "Config")
	bool bRequirePendingInventoryDecision = true;

	UPROPERTY(EditAnywhere, Category = "Config")
	FGameplayTag ExpectedActionTag;
};

USTRUCT(DisplayName = "AI Pending Inventory Decision Matches", Category = "AegisOdyssey|AI")
struct AEGISODYSSEY_API FSTC_AIPendingInventoryDecisionMatches : public FStateTreeConditionCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FAIPendingInventoryDecisionMatchesInstanceData;

	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}

	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;

	UPROPERTY(EditAnywhere, Category = "Config")
	bool bInvert = false;
};
