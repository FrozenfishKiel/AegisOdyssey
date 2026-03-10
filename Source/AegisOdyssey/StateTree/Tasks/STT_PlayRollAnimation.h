#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySpecHandle.h"
#include "StateTreeTaskBase.h"
#include "STT_PlayRollAnimation.generated.h"

class UGameplayAbility;
class UAbilitySystemComponent;

USTRUCT()
struct FPlayRollAnimationMontageInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Config")
	FGameplayTag InputTag;

	UPROPERTY(EditAnywhere, Category = "Config")
	USkeletalMeshComponent* SkeletalMesh;

	UPROPERTY(EditAnywhere, Category = "Config")
	UAnimMontage* ForwardMontage;

	UPROPERTY(EditAnywhere, Category = "Config")
	UAnimMontage* BackwardMontage;

	UPROPERTY(EditAnywhere, Category = "Config")
	UAnimMontage* LeftMontage;

	UPROPERTY(EditAnywhere, Category = "Config")
	UAnimMontage* RightMontage;

	UPROPERTY(EditAnywhere, Category = "Config")
	UAnimMontage* ForwardLeftMontage;

	UPROPERTY(EditAnywhere, Category = "Config")
	UAnimMontage* ForwardRightMontage;

	UPROPERTY(EditAnywhere, Category = "Config")
	UAnimMontage* BackwardLeftMontage;

	UPROPERTY(EditAnywhere, Category = "Config")
	UAnimMontage* BackwardRightMontage;

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

USTRUCT(DisplayName="Play Roll Animation Montage", Category="AegisOdyssey")
struct AEGISODYSSEY_API FSTT_PlayRollAnimation : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FPlayRollAnimationMontageInstanceData;

	FSTT_PlayRollAnimation() = default;

	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual void StateCompleted(FStateTreeExecutionContext& Context, const EStateTreeRunStatus CompletionStatus, const FStateTreeActiveStates& CompletedActiveStates) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
};