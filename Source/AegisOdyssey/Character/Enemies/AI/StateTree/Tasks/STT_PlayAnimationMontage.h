#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySpecHandle.h"
#include "StateTreeTaskBase.h"
#include "STT_PlayAnimationMontage.generated.h"

class UAbilitySystemComponent;
class UAnimMontage;
class UGameplayAbility;

USTRUCT()
struct FSTT_PlayAnimationMontageInstanceData
{
	GENERATED_BODY()

	// [配置] 要播放的动画蒙太奇
	UPROPERTY(EditAnywhere, Category = "Config")
	TSoftObjectPtr<UAnimMontage> Montage;

	// [配置] 播放速率（未来会传递给GA）
	UPROPERTY(EditAnywhere, Category = "Config")
	float PlayRate = 1.0f;

	// [配置] 起始分段名称（未来会传递给GA）
	UPROPERTY(EditAnywhere, Category = "Config")
	FName StartSection = NAME_None;

	// [配置] 是否等待动画播放完成
	// true = Task 会保持 Running 状态直到动画结束
	// false = 激活动画后立即返回 Succeeded
	UPROPERTY(EditAnywhere, Category = "Config")
	bool bWaitForAnimation = true;

	// [配置] GameplayAbility 类（用于播放动画）
	// 如果不设置，则默认使用 UGA_PlayAnimationMontage
	UPROPERTY(EditAnywhere, Category = "Config")
	TSubclassOf<UGameplayAbility> AnimationAbilityClass;

	// [输出] 动画是否成功开始播放
	UPROPERTY(EditAnywhere, Category = "Output")
	bool bAnimationStarted = false;

	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY()
	FGameplayAbilitySpecHandle AbilitySpecHandle;
};

USTRUCT(DisplayName="Play Animation Montage", Category="AegisOdyssey|Animation")
struct AEGISODYSSEY_API FSTT_PlayAnimationMontage : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FSTT_PlayAnimationMontageInstanceData;

	FSTT_PlayAnimationMontage() = default;

	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
};
