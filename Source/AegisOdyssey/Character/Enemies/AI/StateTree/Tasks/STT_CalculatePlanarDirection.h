#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "STT_CalculatePlanarDirection.generated.h"

class AActor;

USTRUCT()
struct FSTT_CalculatePlanarDirectionInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Input", meta = (ToolTip = "可选的起点 Actor。不绑定时会自动回退到 StateTree Owner；如果 Owner 是 AIController，则会继续取它控制的 Pawn。"))
	TObjectPtr<AActor> SourceActor = nullptr;

	UPROPERTY(EditAnywhere, Category = "Input", meta = (ToolTip = "当你不想依赖 Actor 位置时，可以直接填写起点坐标。只有在 bUseSourceActorLocation 为 false 时才会真正使用它。"))
	FVector SourceLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, Category = "Config", meta = (ToolTip = "true 表示优先使用 SourceActor 的位置做起点；false 表示直接使用 SourceLocation。"))
	bool bUseSourceActorLocation = true;

	UPROPERTY(EditAnywhere, Category = "Input", meta = (ToolTip = "目标点。最常见的用法是绑定 EQS 选出来的位置。这个值决定最终方向指向哪里。"))
	FVector TargetLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, Category = "Config", meta = (ToolTip = "是否忽略 Z，只计算平面方向。true 时更适合地面走位、闪避、翻滚；false 时会保留上下高度差。"))
	bool bProjectToPlane = true;

	UPROPERTY(EditAnywhere, Category = "Config", meta = (ToolTip = "是否把结果归一化成单位方向向量。true 时 CalculatedDirection 的长度通常接近 1，只保留方向；false 时会保留原始距离长度。"))
	bool bNormalizeDirection = true;

	UPROPERTY(EditAnywhere, Category = "Config", meta = (ToolTip = "当起点和目标点太近时，是否直接让 Task 失败。true 表示距离太近就失败；false 表示即使没有有效方向也返回成功。"))
	bool bFailIfDirectionIsNearlyZero = true;

	UPROPERTY(EditAnywhere, Category = "Config", meta = (ToolTip = "近零阈值。起点到目标点距离小于这个值时，会认为方向太小、不稳定或没有实际意义。数值越大，越容易把短距离判成无效方向。", ClampMin = "0.0", UIMin = "0.0"))
	float NearlyZeroTolerance = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Output", meta = (ToolTip = "最终算出来的方向结果。若 bNormalizeDirection 为 true，它通常是单位向量；X/Y/Z 的正负分别表示对应轴上的朝向。"))
	FVector CalculatedDirection = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, Category = "Output", meta = (ToolTip = "起点到目标点的距离。数值越大表示目标离起点越远，数值越小表示越近。"))
	float CalculatedDistance = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Output", meta = (ToolTip = "这次计算是否得到了有效方向。true 表示 CalculatedDirection 当前可用，false 表示当前结果无效。"))
	bool bHasValidDirection = false;
};

USTRUCT(DisplayName = "Calculate Planar Direction", Category = "AegisOdyssey|AI")
struct AEGISODYSSEY_API FSTT_CalculatePlanarDirection : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FSTT_CalculatePlanarDirectionInstanceData;

	FSTT_CalculatePlanarDirection();

	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}

	// 计算从起点指向目标点的方向，并把结果缓存到决策组件，供后续动作或技能消费。
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

private:
	AActor* ResolveSourceActor(const FStateTreeExecutionContext& Context, const FInstanceDataType& InstanceData) const;
};
