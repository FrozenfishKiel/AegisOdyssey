#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "STT_FindNearestTarget.generated.h"

class AActor;
// 状态任务：：搜索范围内的对象
USTRUCT()
struct FFindNearestTargetInstanceData
{
	GENERATED_BODY()

	// [输入/配置参数]
	UPROPERTY(EditAnywhere, Category = "Config")
	float SearchRadius = 1500.0f;

	// 你可以通过 Tag 来过滤玩家，或者后续改成你的 CombatInterface
	UPROPERTY(EditAnywhere, Category = "Config")
	FName TargetTag = FName("Player"); 

	// [输出参数] 会在 StateTree 编辑器中暴露出来，供其他节点绑定
	UPROPERTY(EditAnywhere, Category = "Output")
	TObjectPtr<AActor> TargetActor = nullptr; 
};

USTRUCT(DisplayName="Find Nearest Target", Category="AegisOdyssey|AI")
struct AEGISODYSSEY_API FSTT_FindNearestTarget : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FFindNearestTargetInstanceData;

	FSTT_FindNearestTarget() = default;

	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	// 寻找目标瞬间就能完成，通常不需要 Tick，所以可以省略 Tick 和 ExitState
};