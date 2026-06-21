#pragma once

#include "CoreMinimal.h"
#include "AegisOdyssey/Character/Enemies/AI/Decision/AOAIDecisionTypes.h"
#include "AegisOdyssey/Character/Enemies/AI/Decision/Inventory/AOAIInventoryDecisionTypes.h"
#include "StateTreeEvaluatorBase.h"
#include "STE_UpdateInventoryDecision.generated.h"

class AActor;

USTRUCT()
struct FUpdateInventoryDecisionInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<AActor> CurrentTarget = nullptr;

	UPROPERTY(EditAnywhere, Category = "Input")
	float DistanceToTarget = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Input")
	bool bHasTarget = false;

	UPROPERTY(EditAnywhere, Category = "Input")
	FAOAIDecisionTacticalState TacticalState;

	UPROPERTY(EditAnywhere, Category = "Output")
	bool bHasCurrentSubmittedInventoryDecision = false;

	UPROPERTY(EditAnywhere, Category = "Output")
	FAOAIInventoryDecisionResult CurrentSubmittedInventoryDecision;
};

USTRUCT(DisplayName = "Update Inventory Decision", Category = "AegisOdyssey|AI")
struct AEGISODYSSEY_API FSTE_UpdateInventoryDecision : public FStateTreeEvaluatorCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FUpdateInventoryDecisionInstanceData;

	FSTE_UpdateInventoryDecision() = default;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	// STE only decides the desired inventory semantic tag; STT owns lookup and execution.
	virtual void Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
};
