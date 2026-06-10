// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/EnvQueryContext.h"
#include "AOEnvQueryContext_CurrentTarget.generated.h"

struct FEnvQueryContextData;
struct FEnvQueryInstance;

UCLASS(EditInlineNew, BlueprintType)
class AEGISODYSSEY_API UAOEnvQueryContext_CurrentTarget : public UEnvQueryContext
{
	GENERATED_BODY()

public:
	UAOEnvQueryContext_CurrentTarget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const override;
};
