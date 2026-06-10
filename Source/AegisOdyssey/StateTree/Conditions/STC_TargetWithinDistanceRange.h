#pragma once

#include "CoreMinimal.h"
#include "StateTreeConditionBase.h"
#include "STC_TargetWithinDistanceRange.generated.h"

class AActor;

USTRUCT()
struct FTargetWithinDistanceRangeInstanceData
{
	GENERATED_BODY()

	// Bind this from the evaluator's CurrentTarget output.
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<AActor> TargetActor = nullptr;

	// Bind this from the evaluator's DistanceToTarget output.
	UPROPERTY(EditAnywhere, Category = "Input", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float DistanceToTarget = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Config", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float MinDistance = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Config", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float MaxDistance = 250.0f;
};

USTRUCT(DisplayName = "Target Within Distance Range", Category = "AegisOdyssey|AI")
struct AEGISODYSSEY_API FSTC_TargetWithinDistanceRange : public FStateTreeConditionCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FTargetWithinDistanceRangeInstanceData;

	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}

	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;

	UPROPERTY(EditAnywhere, Category = "Config")
	bool bInvert = false;
};
