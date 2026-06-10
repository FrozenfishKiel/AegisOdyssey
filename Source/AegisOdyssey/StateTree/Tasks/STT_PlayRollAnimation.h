#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySpecHandle.h"
#include "StateTreeTaskBase.h"
#include "STT_PlayRollAnimation.generated.h"

class AAOCharacter;
class UAbilitySystemComponent;

USTRUCT()
struct FPlayRollAnimationMontageInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Config", meta = (ToolTip = "要激活的翻滚能力标签。这个标签会被用来定位并尝试激活对应能力。"))
	FGameplayTag InputTag;

	UPROPERTY(EditAnywhere, Category = "Config", meta = (ToolTip = "要播放翻滚动画的骨骼网格。通常应绑定角色自己的 Mesh。"))
	USkeletalMeshComponent* SkeletalMesh = nullptr;

	UPROPERTY(EditAnywhere, Category = "Config", meta = (ToolTip = "前翻使用的蒙太奇。当前方向更接近前方时会优先使用它。"))
	UAnimMontage* ForwardMontage = nullptr;

	UPROPERTY(EditAnywhere, Category = "Config", meta = (ToolTip = "后翻使用的蒙太奇。当前方向更接近后方时会优先使用它。"))
	UAnimMontage* BackwardMontage = nullptr;

	UPROPERTY(EditAnywhere, Category = "Config", meta = (ToolTip = "左翻使用的蒙太奇。当前方向更接近左方时会优先使用它。"))
	UAnimMontage* LeftMontage = nullptr;

	UPROPERTY(EditAnywhere, Category = "Config", meta = (ToolTip = "右翻使用的蒙太奇。当前方向更接近右方时会优先使用它。"))
	UAnimMontage* RightMontage = nullptr;

	UPROPERTY(EditAnywhere, Category = "Config", meta = (ToolTip = "左前翻使用的蒙太奇。当前方向更接近左前时会优先使用它。"))
	UAnimMontage* ForwardLeftMontage = nullptr;

	UPROPERTY(EditAnywhere, Category = "Config", meta = (ToolTip = "右前翻使用的蒙太奇。当前方向更接近右前时会优先使用它。"))
	UAnimMontage* ForwardRightMontage = nullptr;

	UPROPERTY(EditAnywhere, Category = "Config", meta = (ToolTip = "左后翻使用的蒙太奇。当前方向更接近左后时会优先使用它。"))
	UAnimMontage* BackwardLeftMontage = nullptr;

	UPROPERTY(EditAnywhere, Category = "Config", meta = (ToolTip = "右后翻使用的蒙太奇。当前方向更接近右后时会优先使用它。"))
	UAnimMontage* BackwardRightMontage = nullptr;

	UPROPERTY(EditAnywhere, Category = "Config", meta = (ToolTip = "播放速率。大于 1 表示动画更快，0 到 1 表示动画更慢。"))
	float PlayRate = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Config", meta = (ToolTip = "起播 Section 名。填写后会从该 Section 开始播放；留空则按默认起点播放。"))
	FName StartSection = NAME_None;

	UPROPERTY(EditAnywhere, Category = "Config", meta = (ToolTip = "起播时间。数值越大表示从蒙太奇更靠后的位置开始播放。"))
	float StartTime = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Config", meta = (ToolTip = "是否在播放当前翻滚前停止其它蒙太奇。true 表示先停掉其它蒙太奇，false 表示允许并存。"))
	bool bStopAllMontages = true;

	UPROPERTY(EditAnywhere, Category = "Config", meta = (ToolTip = "混出时间。数值越大表示退出当前蒙太奇时混出越柔和，数值越小表示退出越干脆。"))
	float InBlendOutTime = 0.0f;

	UPROPERTY(Transient)
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent = nullptr;

	UPROPERTY(Transient)
	FGameplayAbilitySpecHandle AbilitySpecHandle;

	UPROPERTY(EditAnywhere, Category = "Output", meta = (ToolTip = "当前对应能力是否处于激活状态。true 表示翻滚能力当前还在执行，false 表示当前没有激活。"))
	bool bActivated = false;
};

USTRUCT(DisplayName = "Play Roll Animation Montage", Category = "AegisOdyssey")
struct AEGISODYSSEY_API FSTT_PlayRollAnimation : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FPlayRollAnimationMontageInstanceData;

	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual void StateCompleted(FStateTreeExecutionContext& Context, const EStateTreeRunStatus CompletionStatus, const FStateTreeActiveStates& CompletedActiveStates) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;

private:
	AAOCharacter* ResolveCharacter(const FStateTreeExecutionContext& Context) const;
};
