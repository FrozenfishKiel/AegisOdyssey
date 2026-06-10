#pragma once

#include "CoreMinimal.h"
#include "AegisOdyssey/Character/AOHeroComponent.h"
#include "GameplayAbilitySpec.h"
#include "StateTreeTaskBase.h"
#include "STT_SendCombatCommand.generated.h"

class UAbilitySystemComponent;

UENUM()
enum class ESTTCommandWaitMode : uint8
{
	None UMETA(ToolTip = "只负责发送指令，发送完成后立即结束。"),
	WaitFixedDuration UMETA(ToolTip = "发送后固定等待一段时长，再继续往下走。不关心技能是否激活，也不关心它什么时候结束。"),
	WaitForActivation UMETA(ToolTip = "等待命中的能力真正进入激活态。"),
	WaitForCompletion UMETA(ToolTip = "等待命中的能力激活，并持续等到它结束。"),
	WaitForCompletionIfActivated UMETA(ToolTip = "如果命中的能力激活了，就继续等到它结束；如果直到超时都没有激活，则按成功结束。")
};

USTRUCT()
struct FSTT_SendCombatCommandInstanceData
{
	GENERATED_BODY()

	// 这次 Task 想要发出的输入标签。
	// 它会沿着 HeroComponent 的统一输入分发入口被注入到当前角色。
	UPROPERTY(EditAnywhere, Category = "Config", meta = (ToolTip = "要通过 Hero 指令入口发送出去的输入标签。"))
	FGameplayTag InputTag;

	// 这次要模拟哪一种输入事件。
	// 不同技能可能依赖 Press / Release / Trigger 等不同输入语义。
	UPROPERTY(EditAnywhere, Category = "Config", meta = (ToolTip = "本次要注入的输入事件类型。"))
	TEnumAsByte<EInputType> InputType = EInputType::Trigger;

	// 发送命令之后，这个 Task 还要继续等待哪一种阶段。
	// 它决定这个 Task 是立刻返回，还是要短等一下，还是要观察能力生命周期。
	UPROPERTY(EditAnywhere, Category = "Config", meta = (ToolTip = "决定命令发出后，Task 还要继续等待到哪个阶段。"))
	ESTTCommandWaitMode WaitMode = ESTTCommandWaitMode::None;

	// 只用于 WaitFixedDuration。
	// 这是专门给“技能输入窗口”准备的固定短等待：
	// 即使技能本身很墨迹，这个 Task 也只占用这段时间，然后就继续往后走，
	// 不会因为技能实际执行很长而把状态树一直卡在输入阶段。
	UPROPERTY(EditAnywhere, Category = "Config", meta = (ToolTip = "仅用于 WaitFixedDuration。无论技能有没有激活，Task 都会在等待这段时长后成功结束。", ClampMin = "0.0", UIMin = "0.0"))
	float FixedWaitSeconds = 0.25f;

	// 愿意等待“能力激活出现”的最长时间。
	// 小于等于 0 表示不启用这段超时。
	UPROPERTY(EditAnywhere, Category = "Config", meta = (ToolTip = "等待命中能力激活的超时时长。小于等于 0 表示不启用超时。", ClampMin = "0.0", UIMin = "0.0"))
	float ActivationTimeoutSeconds = 0.35f;

	// 愿意等待“已观察到的能力结束”的最长时间。
	// 小于等于 0 表示不启用这段超时。
	UPROPERTY(EditAnywhere, Category = "Config", meta = (ToolTip = "等待已观察到能力结束的超时时长。小于等于 0 表示不启用超时。", ClampMin = "0.0", UIMin = "0.0"))
	float CompletionTimeoutSeconds = 2.0f;

	// 这次命令是否真的成功注入到了 Hero 的输入分发链路里。
	UPROPERTY(EditAnywhere, Category = "Output", meta = (ToolTip = "这次命令是否成功发送。"))
	bool bCommandSent = false;

	// 如果任务是因为等待超时而失败，这里会被置为 true。
	UPROPERTY(EditAnywhere, Category = "Output", meta = (ToolTip = "任务是否发生了超时。"))
	bool bTimedOut = false;

	// 发送命令后，是否至少观察到过一次“新的匹配能力激活”。
	UPROPERTY(EditAnywhere, Category = "Output", meta = (ToolTip = "发送命令后，是否观察到匹配能力被激活。"))
	bool bObservedAbilityActivation = false;

	// 只用于 WaitForCompletionIfActivated。
	// 它表示这次 Task 是“按设计允许地”在没有观察到任何激活的情况下结束的，
	// 不是失败，也不是异常，而是当前模式允许这种结果。
	UPROPERTY(EditAnywhere, Category = "Output", meta = (ToolTip = "仅用于 WaitForCompletionIfActivated。true 表示任务在没有观察到激活的情况下按设计成功结束。"))
	bool bFinishedWithoutActivation = false;

	// 自从 EnterState 以来，这个 Task 已经累计等待了多久。
	UPROPERTY(EditAnywhere, Category = "Output", meta = (ToolTip = "任务当前已经等待的时长。"))
	float ElapsedWaitTime = 0.0f;

	UPROPERTY(Transient)
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent = nullptr;

	UPROPERTY(Transient)
	TArray<FGameplayAbilitySpecHandle> MatchingAbilitySpecHandles;

	UPROPERTY(Transient)
	TArray<FGameplayAbilitySpecHandle> ActiveAbilitySpecHandlesBeforeCommand;

	UPROPERTY(Transient)
	FGameplayAbilitySpecHandle ObservedAbilitySpecHandle;

	UPROPERTY(Transient)
	float EnterWorldTimeSeconds = -1.0f;
};

USTRUCT(DisplayName = "Send Combat Command", Category = "AegisOdyssey|AI")
struct AEGISODYSSEY_API FSTT_SendCombatCommand : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FSTT_SendCombatCommandInstanceData;

	FSTT_SendCombatCommand() = default;

	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;

private:
	// 解析这次命令真正应该发给谁。
	// 对 AI 控制的状态树来说，Owner 往往是 Controller，但真正持有 HeroComponent 的通常是 Pawn，
	// 所以这里会把 Controller 解析回它正在控制的 Pawn。
	AActor* ResolveCommandTarget(const FStateTreeExecutionContext& Context) const;

	// 通过 HeroComponent 的统一输入分发口发送命令。
	// 这样无论上层是状态树、技能系统还是别的监听者，都仍然走同一条正式入口。
	bool SendCombatCommand(AActor* CommandTarget, const FGameplayTag& InputTag, TEnumAsByte<EInputType> InputType) const;

	// 当等待模式需要观察能力激活/结束状态时，
	// 这里负责解析应该观察哪一个 ASC。
	UAbilitySystemComponent* ResolveAbilitySystemComponent(AActor* CommandTarget) const;

	// 收集所有命中这个 InputTag 的 AbilitySpec，
	// 同时记录在发送命令之前就已经处于激活态的那些 Spec，
	// 这样后面才能判断“这次是不是新激活出来的能力”。
	void GatherMatchingAbilitySpecHandles(const UAbilitySystemComponent& AbilitySystemComponent, const FGameplayTag& InputTag, TArray<FGameplayAbilitySpecHandle>& OutMatchingHandles, TArray<FGameplayAbilitySpecHandle>& OutActiveHandles) const;

	// 返回第一个“现在是 Active，但发送命令之前并不在激活快照里”的能力。
	// 也就是这次输入之后新进入激活态的候选能力。
	FGameplayAbilitySpec* FindNewlyActivatedAbility(const UAbilitySystemComponent& AbilitySystemComponent, const TArray<FGameplayAbilitySpecHandle>& MatchingHandles, const TArray<FGameplayAbilitySpecHandle>& PreviouslyActiveHandles) const;

	// 优先使用世界时间来计算等待时长。
	// 这样比单纯累加 DeltaTime 更稳，不容易因为 Tick 波动导致等待误差。
	float GetCurrentWorldTimeSeconds(const FStateTreeExecutionContext& Context) const;
};
