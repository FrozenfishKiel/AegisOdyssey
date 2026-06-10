// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "STT_ClearPersistentStateTags.generated.h"

class AActor;

USTRUCT()
struct FSTT_ClearPersistentStateTagsInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<AActor> TargetActor = nullptr;

	UPROPERTY(EditAnywhere, Category = "Config")
	FName SourceId = NAME_None;

	UPROPERTY(EditAnywhere, Category = "Output")
	bool bCleared = false;
};

USTRUCT(DisplayName = "Clear Persistent State Tags", Category = "AegisOdyssey")
struct AEGISODYSSEY_API FSTT_ClearPersistentStateTags : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FSTT_ClearPersistentStateTagsInstanceData;

	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

private:
	AActor* ResolveTargetActor(const FStateTreeExecutionContext& Context, const FInstanceDataType& InstanceData) const;
};
