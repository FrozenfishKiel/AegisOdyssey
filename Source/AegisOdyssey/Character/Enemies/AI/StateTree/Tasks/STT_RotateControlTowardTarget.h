#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "STT_RotateControlTowardTarget.generated.h"

class AActor;
class AAIController;

USTRUCT()
struct FSTT_RotateControlTowardTargetInstanceData
{
	GENERATED_BODY()

	// 可选目标绑定。
	// 如果外部没有显式绑定，就回退到 AIController 的 CurrentTarget。
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<AActor> TargetActor = nullptr;

	// 基础旋转速度，单位是“每秒多少度”。
	// 我们用恒定插值而不是弹簧插值，这样更容易像“玩家手动转鼠标”的固定速度。
	UPROPERTY(EditAnywhere, Category = "Config", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float RotationSpeed = 540.0f;

	// 额外倍率，方便外部状态直接约束旋转快慢。
	// 例如蓄力时可以传一个更小的倍率，让 AI 和玩家一样转得更慢。
	UPROPERTY(EditAnywhere, Category = "Config", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float RotationSpeedMultiplier = 1.0f;

	// Yaw 误差小于这个值时，就认为“基本已经对准目标”。
	// 单次转向模式下会以此作为结束条件。
	UPROPERTY(EditAnywhere, Category = "Config", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float YawTolerance = 5.0f;

	// 允许在目标正前方基础上再偏移一点朝向，方便以后做出更像人的非绝对正对效果。
	UPROPERTY(EditAnywhere, Category = "Config")
	float AimOffsetYaw = 0.0f;

	// 是否同时处理 Pitch。
	// 近战大多只关心 Yaw，因此默认关闭。
	UPROPERTY(EditAnywhere, Category = "Config")
	bool bUsePitch = false;

	// 是否持续追踪目标。
	// 为 true 时，Task 会一直 Running 并持续修正朝向；
	// 为 false 时，只要进入容差就 Succeeded。
	UPROPERTY(EditAnywhere, Category = "Config")
	bool bContinuous = true;

	// 可选的内部结束时长。
	// 大于 0 时，到时自动成功结束；小于等于 0 表示不限制，完全交给外部状态树控制退出。
	UPROPERTY(EditAnywhere, Category = "Config", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float MaxDuration = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Output")
	float ElapsedTime = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Output")
	float CurrentYawDifference = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Output")
	bool bReachedTargetYaw = false;

	UPROPERTY(Transient)
	float StartTimeSeconds = -1.0f;

	UPROPERTY(Transient)
	TObjectPtr<AAIController> AIController = nullptr;
};

USTRUCT(DisplayName="Rotate Control Toward Target", Category="AegisOdyssey|AI")
struct AEGISODYSSEY_API FSTT_RotateControlTowardTarget : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FSTT_RotateControlTowardTargetInstanceData;

	FSTT_RotateControlTowardTarget();

	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;

private:
	AAIController* ResolveAIController(const FStateTreeExecutionContext& Context) const;
	AActor* ResolveTargetActor(FStateTreeExecutionContext& Context) const;
	float GetCurrentWorldTimeSeconds(const FStateTreeExecutionContext& Context) const;
};
