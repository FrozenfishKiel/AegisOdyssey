#pragma once
#include "CoreMinimal.h"
#include "AegisOdyssey/Character/AOHeroComponent.h"
#include "GameplayAbilitySpecHandle.h"
#include "StateTreeTaskBase.h"
#include "STT_PlayBlockAnimation.generated.h"

class UGameplayAbility;
class UAbilitySystemComponent;

USTRUCT()
struct FPlayBlockAnimationMontageInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Config")
	FGameplayTag InputTag;

	// 传给格挡能力的输入阶段，None 表示本节点未指定输入阶段。
	UPROPERTY()
	TEnumAsByte<EInputType> InputType = EInputType::None;
	
	// 格挡动画节点的可选角色网格绑定，空值表示节点未配置网格。
	UPROPERTY(EditAnywhere, Category = "Config")
	USkeletalMeshComponent* SkeletalMesh = nullptr;

	// 格挡起手的蒙太奇，空值使能力直接进入持续格挡。
	UPROPERTY(EditAnywhere, Category = "Config")
	UAnimMontage* StartBlockMontage = nullptr;
	
	// 持续格挡的蒙太奇，空值使能力结束格挡。
	UPROPERTY(EditAnywhere, Category = "Config")
	UAnimMontage* LoopBlockMontage = nullptr;

	// 格挡收尾的蒙太奇，空值时能力会请求结束。
	UPROPERTY(EditAnywhere, Category = "Config")
	UAnimMontage* EndBlockMontage = nullptr;

	UPROPERTY(EditAnywhere, Category = "Config")
	float PlayRate = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Config")
	FName StartSection = NAME_None;

	UPROPERTY(EditAnywhere, Category = "Config")
	float StartTime = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Config")
	bool bStopAllMontages = true;

	UPROPERTY(EditAnywhere, Category = "Config")
	float InBlendOutTime = 0.0f;

	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	bool bActivated = false;

	UPROPERTY()
	FGameplayAbilitySpecHandle AbilitySpecHandle;
};

USTRUCT(DisplayName="Play Block Animation Montage", Category="AegisOdyssey")
struct FSTT_PlayBlockAnimation:public FStateTreeTaskCommonBase
{
	GENERATED_BODY()
	using FInstanceDataType = FPlayBlockAnimationMontageInstanceData;

	FSTT_PlayBlockAnimation() = default;

	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual void StateCompleted(FStateTreeExecutionContext& Context, const EStateTreeRunStatus CompletionStatus, const FStateTreeActiveStates& CompletedActiveStates) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
};
