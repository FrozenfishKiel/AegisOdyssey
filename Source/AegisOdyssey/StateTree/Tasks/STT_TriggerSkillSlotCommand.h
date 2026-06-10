#pragma once

#include "CoreMinimal.h"
#include "AegisOdyssey/Character/AOHeroComponent.h"
#include "AegisOdyssey/Character/Enemies/AI/StateTree/Tasks/STT_SendCombatCommand.h"
#include "AegisOdyssey/SkillSystem/Components/AOSkillComponent.h"
#include "StateTreeTaskBase.h"
#include "STT_TriggerSkillSlotCommand.generated.h"

USTRUCT()
struct FSTT_TriggerSkillSlotCommandInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Config", meta = (ToolTip = "优先使用的技能槽索引。大于等于 0 时，Task 会直接按槽位执行。"))
	int32 SlotIndex = INDEX_NONE;

	UPROPERTY(EditAnywhere, Category = "Config", meta = (Categories = "InputTag", ToolTip = "可选的技能槽输入标签。仅当 SlotIndex 无效时才会用它反查槽位。"))
	FGameplayTag SkillSlotInputTag;

	UPROPERTY(EditAnywhere, Category = "Config", meta = (ToolTip = "本次要向技能槽执行层注入的输入语义。"))
	TEnumAsByte<EInputType> InputType = EInputType::Trigger;

	UPROPERTY(EditAnywhere, Category = "Config", meta = (ToolTip = "这里只支持 None 和 WaitFixedDuration。技能是否激活、何时结束由技能自身决定。"))
	ESTTCommandWaitMode WaitMode = ESTTCommandWaitMode::None;

	UPROPERTY(EditAnywhere, Category = "Config", meta = (ToolTip = "仅用于 WaitFixedDuration。Task 只在固定时长后结束，不观察技能生命周期。", ClampMin = "0.0", UIMin = "0.0"))
	float FixedWaitSeconds = 0.25f;

	UPROPERTY(EditAnywhere, Category = "Output")
	bool bCommandAccepted = false;

	UPROPERTY(EditAnywhere, Category = "Output")
	float ElapsedWaitTime = 0.0f;

	UPROPERTY(Transient)
	TObjectPtr<UAOSkillComponent> SkillComponent = nullptr;

	UPROPERTY(Transient)
	float EnterWorldTimeSeconds = -1.0f;
};

USTRUCT(DisplayName = "Trigger Skill Slot Command", Category = "AegisOdyssey|Skill")
struct AEGISODYSSEY_API FSTT_TriggerSkillSlotCommand : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FSTT_TriggerSkillSlotCommandInstanceData;

	FSTT_TriggerSkillSlotCommand() = default;

	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;

private:
	AActor* ResolveCommandTarget(const FStateTreeExecutionContext& Context) const;
	UAOSkillComponent* ResolveSkillComponent(AActor* CommandTarget) const;
	int32 ResolveSlotIndex(const UAOSkillComponent& SkillComponent, const FInstanceDataType& InstanceData) const;
	float GetCurrentWorldTimeSeconds(const FStateTreeExecutionContext& Context) const;
};
