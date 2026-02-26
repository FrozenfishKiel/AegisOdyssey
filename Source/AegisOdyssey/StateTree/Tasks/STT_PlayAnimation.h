// STT_PlayAnimation.h
#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "STT_PlayAnimation.generated.h"

/**
 * 播放连招动画的任务
 * 进入状态时添加Tag，动画结束/中断/BlendOut/取消时移除Tag
 */
USTRUCT()
struct FPlayAnimationMontageInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Config")
	FGameplayTag StateTag;

	UPROPERTY(EditAnywhere, Category = "Config")
	USkeletalMeshComponent* SkeletalMesh;

	UPROPERTY(EditAnywhere, Category = "Config")
	UAnimMontage* Montage;

	UPROPERTY(EditAnywhere, Category = "Config")
	float PlayRate = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Config")
	FName StartSection = NAME_None;

	UPROPERTY(EditAnywhere, Category = "Config")
	float StartTime = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Config")
	bool bStopAllMontages = true;
};

USTRUCT(DisplayName="Play Animation Montage", Category="AegisOdyssey")
struct AEGISODYSSEY_API FSTT_PlayAnimation : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FPlayAnimationMontageInstanceData;

	FSTT_PlayAnimation() = default;

	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
};