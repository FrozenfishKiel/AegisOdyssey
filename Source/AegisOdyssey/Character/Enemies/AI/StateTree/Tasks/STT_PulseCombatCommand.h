#pragma once

#include "CoreMinimal.h"
#include "AegisOdyssey/Character/AOHeroComponent.h"
#include "StateTreeTaskBase.h"
#include "STT_PulseCombatCommand.generated.h"

USTRUCT()
struct FSTT_PulseCombatCommandInstanceData
{
	GENERATED_BODY()

	// 这就是要模拟的那一次输入脉冲，本质上等价于玩家按下某个已经映射好的战斗按键。
	UPROPERTY(EditAnywhere, Category = "Config")
	FGameplayTag InputTag;

	UPROPERTY(EditAnywhere, Category = "Config")
	TEnumAsByte<EInputType> InputType = EInputType::Trigger;

	// 状态刚激活时是否立刻先发一次输入，用来避免进入状态后还要先傻等一段随机时间。
	UPROPERTY(EditAnywhere, Category = "Config")
	bool bSendImmediatelyOnEnter = true;

	// 如果不想一进状态就立刻发输入，可以在这里配置首次发送前的随机延迟。
	UPROPERTY(EditAnywhere, Category = "Config", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float InitialDelayMin = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Config", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float InitialDelayMax = 0.0f;

	// 状态持续激活期间，后续每次输入脉冲的随机间隔。
	// 这里故意做成通用节奏参数，这样以后不管是随机规则、PPO 还是 LLM，
	// 最后都只是决定“下一次什么时候再按一下”，不用改这个 Task 的职责。
	UPROPERTY(EditAnywhere, Category = "Config", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float PulseIntervalMin = 0.35f;

	UPROPERTY(EditAnywhere, Category = "Config", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float PulseIntervalMax = 0.75f;

	// 可选的内部随机持续时长区间。
	// 进入状态时会从这个区间里抽一次“本次最多持续多久”。
	// 如果两边都小于等于 0，就表示不启用这个限制，完全交给外部 Condition/Transition 决定何时退出。
	UPROPERTY(EditAnywhere, Category = "Config", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float DurationMin = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Config", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float DurationMax = 0.0f;

	// 可选的最大脉冲次数。
	// 大于 0 时，累计发送到指定次数后自动结束；
	// 小于等于 0 时，表示不限制发送次数。
	UPROPERTY(EditAnywhere, Category = "Config", meta = (ClampMin = "0", UIMin = "0"))
	int32 MaxPulseCount = 0;

	UPROPERTY(EditAnywhere, Category = "Output")
	bool bLastPulseSucceeded = false;

	UPROPERTY(EditAnywhere, Category = "Output")
	int32 PulseCount = 0;

	// 记录本次进入状态后，实际抽到的持续时长上限，方便直接在调试里看到结果。
	UPROPERTY(EditAnywhere, Category = "Output")
	float DurationLimit = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Output")
	float ElapsedTime = 0.0f;

	// 这个倒计时由 Task 实例自己维护，所以同一个 Attack 状态只进一次，
	// 但在状态存活期间可以不断重复发输入，而不是靠重进状态来模拟连点。
	UPROPERTY(Transient)
	float TimeUntilNextPulse = 0.0f;

	// 用世界时间记录进入时刻，让 ElapsedTime 更接近“现实里已经过去多久”，
	// 而不是只统计到 Task 被 Tick 的累计 DeltaTime。
	UPROPERTY(Transient)
	float StartTimeSeconds = -1.0f;
};

USTRUCT(DisplayName="Pulse Combat Command", Category="AegisOdyssey|AI")
struct AEGISODYSSEY_API FSTT_PulseCombatCommand : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FSTT_PulseCombatCommandInstanceData;

	FSTT_PulseCombatCommand();

	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;

private:
	AActor* ResolveCommandTarget(const FStateTreeExecutionContext& Context) const;
	bool SendCombatCommand(AActor* CommandTarget, const FGameplayTag& InputTag, TEnumAsByte<EInputType> InputType) const;
	bool SendPulse(FStateTreeExecutionContext& Context) const;
	float GetRandomizedDelay(float MinDelay, float MaxDelay) const;
	bool ShouldFinishPulsing(const FInstanceDataType& InstanceData) const;
	float GetCurrentWorldTimeSeconds(const FStateTreeExecutionContext& Context) const;
};
