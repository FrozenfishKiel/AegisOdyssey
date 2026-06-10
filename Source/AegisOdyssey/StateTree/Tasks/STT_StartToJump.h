#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySpecHandle.h"
#include "StateTreeTaskBase.h"
#include "STT_StartToJump.generated.h"
class UGameplayAbility;
class UAbilitySystemComponent;
USTRUCT()
struct FStartToJumpInstanceData
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere ,Category = "Config")
	FGameplayTag InputTag;
	
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	bool bActivated = false;

	UPROPERTY()
	FGameplayAbilitySpecHandle AbilitySpecHandle;
};

USTRUCT(DisplayName="Play Start To Jump", Category="AegisOdyssey")
struct AEGISODYSSEY_API FSTT_StartToJump : public FStateTreeTaskCommonBase 
{
	GENERATED_BODY()
public:
	using FInstanceDataType = FStartToJumpInstanceData;

	FSTT_StartToJump() = default;

	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual void StateCompleted(FStateTreeExecutionContext& Context, const EStateTreeRunStatus CompletionStatus, const FStateTreeActiveStates& CompletedActiveStates) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
};
