#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "StateTreePropertyRef.h"
#include "StateTreeTaskBase.h"
#include "STT_RunEQSSelectLocation.generated.h"

class AAIController;
class AActor;
class UEnvQuery;

USTRUCT()
struct FSTT_RunEQSSelectLocationInstanceData
{
	GENERATED_BODY()

	// 可选的 Querier 绑定。
	// 如果不显式绑定，就优先回退到 Controller 的 Pawn，其次回退到 StateTree Owner。
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<AActor> QuerierActor = nullptr;

	UPROPERTY(EditAnywhere, Category = "Config")
	TObjectPtr<UEnvQuery> QueryTemplate = nullptr;

	// 保留 RunMode，方便后续继续复用这个节点。
	UPROPERTY(EditAnywhere, Category = "Config")
	TEnumAsByte<EEnvQueryRunMode::Type> RunMode = EEnvQueryRunMode::SingleResult;

	// 传给 EQS 的命名参数。
	UPROPERTY(EditAnywhere, Category = "Config")
	TArray<FEnvNamedValue> NamedParams;

	// 将本次 EQS 结果写入父状态或全局共享参数，方便兄弟状态继续使用。
	UPROPERTY(EditAnywhere, Category = "Output", meta = (RefType = "/Script/CoreUObject.Vector", Optional))
	FStateTreePropertyRef Result;

	// 如果查询失败或没有有效点位，是否直接让 Task 失败。
	UPROPERTY(EditAnywhere, Category = "Config")
	bool bFailIfNoValidResult = true;

	UPROPERTY(EditAnywhere, Category = "Output")
	FVector SelectedLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, Category = "Output")
	bool bHasValidLocation = false;

	UPROPERTY(EditAnywhere, Category = "Output")
	TEnumAsByte<EEnvQueryStatus::Type> QueryStatus = EEnvQueryStatus::Processing;

	UPROPERTY(Transient)
	int32 RequestId = INDEX_NONE;
};

USTRUCT(DisplayName = "Run EQS Select Location", Category = "AegisOdyssey|AI")
struct AEGISODYSSEY_API FSTT_RunEQSSelectLocation : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FSTT_RunEQSSelectLocationInstanceData;

	FSTT_RunEQSSelectLocation();

	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

private:
	AActor* ResolveQuerierActor(const FStateTreeExecutionContext& Context, const FInstanceDataType& InstanceData) const;
};
