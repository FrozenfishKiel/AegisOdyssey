#pragma once
#include "CoreMinimal.h"
#include "GameplayAbilitySpecHandle.h"
#include "StateTreeTaskBase.h"
#include "STT_StartToSprint.generated.h"

class UGameplayAbility;
class UAbilitySystemComponent;


USTRUCT()
struct FStartToSprintInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Config")
	FGameplayTag InputTag;

	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;
	
	UPROPERTY()
	FGameplayAbilitySpecHandle AbilitySpecHandle;

	bool bActivated = false;
};
USTRUCT(DisplayName="Play Start To Sprint", Category="AegisOdyssey")
struct AEGISODYSSEY_API FSTT_StartToSprint : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FStartToSprintInstanceData;

	FSTT_StartToSprint() = default;

	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual void StateCompleted(FStateTreeExecutionContext& Context, const EStateTreeRunStatus CompletionStatus, const FStateTreeActiveStates& CompletedActiveStates) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
};
