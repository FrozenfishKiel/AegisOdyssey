#pragma once
#include "CoreMinimal.h"
#include "GameplayAbilitySpecHandle.h"
#include "StateTreeTaskBase.h"
#include "STT_PlayBlockAnimation.generated.h"

enum EInputType : uint8;
class UGameplayAbility;
class UAbilitySystemComponent;

USTRUCT()
struct FPlayBlockAnimationMontageInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Config")
	FGameplayTag InputTag;

	UPROPERTY()
	TEnumAsByte<EInputType> InputType;
	
	UPROPERTY(EditAnywhere, Category = "Config")
	USkeletalMeshComponent* SkeletalMesh;

	UPROPERTY(EditAnywhere, Category = "Config")
	UAnimMontage* StartBlockMontage;
	
	UPROPERTY(EditAnywhere, Category = "Config")
	UAnimMontage* LoopBlockMontage;

	UPROPERTY(EditAnywhere, Category = "Config")
	UAnimMontage* EndBlockMontage;

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
