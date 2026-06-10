#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySpecHandle.h"
#include "StateTreeTaskBase.h"
#include "STT_PlayHarvest.generated.h"

class AActor;
class UAnimMontage;
class UAbilitySystemComponent;
class UAOHarvestToolDefinition;
class UAOHarvestToolFragment;
class UAOHarvestToolInstance;

USTRUCT()
struct FPlayHarvestInstanceData
{
	GENERATED_BODY()

	// 采集动作通过哪个输入标签去匹配已经授予到 ASC 上的采集 Ability。
	UPROPERTY(EditAnywhere, Category = "Config", meta = (Categories = "InputTag"))
	FGameplayTag InputTag;

	// 这次采集动作具体播哪段蒙太奇，由状态树像战斗任务一样直接配置。
	// 这样采集动作的前摇、命中窗、后摇都由状态树资产决定，而不是写死在 GA 或工具定义里。
	UPROPERTY(EditAnywhere, Category = "Config")
	TObjectPtr<UAnimMontage> Montage = nullptr;

	UPROPERTY(EditAnywhere, Category = "Config")
	float PlayRate = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Config")
	FName StartSection = NAME_None;

	UPROPERTY(EditAnywhere, Category = "Config", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float StartTime = 0.0f;

	UPROPERTY(Transient)
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent = nullptr;

	UPROPERTY(Transient)
	FGameplayAbilitySpecHandle AbilitySpecHandle;

	UPROPERTY(Transient)
	TObjectPtr<const UAOHarvestToolDefinition> HarvestToolDefinition = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<const UAOHarvestToolInstance> HarvestToolInstance = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<const UAOHarvestToolFragment> HarvestToolFragment = nullptr;

	// 这里只记录“这次采集 Ability 是否已经成功发起”。
	// 真正的命中时序与结算由 GA_Harvest 和状态树下发蒙太奇里的 HarvestWindow 负责。
	bool bActivated = false;
};

USTRUCT(DisplayName = "Play Harvest", Category = "AegisOdyssey|Harvest")
struct AEGISODYSSEY_API FSTT_PlayHarvest : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FPlayHarvestInstanceData;

	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

private:
	AActor* ResolveOwningActor(const FStateTreeExecutionContext& Context) const;
	UAbilitySystemComponent* ResolveAbilitySystemComponent(AActor* OwnerActor) const;
	bool ResolveCurrentHarvestTool(FInstanceDataType& InstanceData, AActor* OwnerActor) const;
	bool GatherMatchingAbilitySpecHandle(const UAbilitySystemComponent& AbilitySystemComponent, const FGameplayTag& InputTag, FGameplayAbilitySpecHandle& OutHandle) const;
};
