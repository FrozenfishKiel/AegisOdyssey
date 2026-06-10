// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AegisOdyssey/AbilitySystem/Abilities/AOGameplayAbility.h"
#include "AegisOdyssey/Interaction/InteractionOption.h"
#include "AOGameplayAbility_Interact.generated.h"

class UAT_WaitForInteractable_LineTrace;
class UAbilityTask_WaitInputPress;
class UUserWidget;

/**
 * 通用交互能力。
 * 它只负责扫描当前命中的可交互对象、缓存对象暴露出的交互选项，
 * 并在输入确认时把当前选中的选项转发给对象侧执行。
 */
UCLASS()
class AEGISODYSSEY_API UAOGameplayAbility_Interact : public UAOGameplayAbility
{
	GENERATED_BODY()

public:
	UAOGameplayAbility_Interact(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	/** 写入当前扫描得到的交互选项列表。 */
	UFUNCTION(BlueprintCallable, Category = "AO|Interaction")
	void UpdateInteractOptions(const TArray<FInteractionOption>& InteractionOptions);

	/** 触发默认交互，等价于执行当前列表中的第 0 个交互选项。 */
	UFUNCTION(BlueprintCallable, Category = "AO|Interaction")
	void TriggerInteraction();

	/** 触发指定索引的交互选项。 */
	UFUNCTION(BlueprintCallable, Category = "AO|Interaction")
	void TriggerInteractionByIndex(int32 OptionIndex);

	/** 返回当前缓存的交互选项，供 HUD/ViewModel 读取。 */
	UFUNCTION(BlueprintPure, Category = "AO|Interaction")
	const TArray<FInteractionOption>& GetCurrentInteractionOptions() const { return CurrentOptions; }

protected:
	/** 开始等待下一次交互输入确认。 */
	void BeginWaitForInteractionInput();

	/** 收到一次交互输入确认后的回调。 */
	UFUNCTION()
	void HandleInteractionInputPressed(float TimeWaited);

	/** 把当前交互选项同步到 HUD ViewModel。 */
	void PushInteractionOptionsToHUDViewModel() const;

protected:
	/** 当前扫描命中的交互选项列表。 */
	UPROPERTY(BlueprintReadWrite, Category = "AO|Interaction")
	TArray<FInteractionOption> CurrentOptions;

	/** 当前正在运行的交互扫描任务。 */
	UPROPERTY(Transient)
	TObjectPtr<UAT_WaitForInteractable_LineTrace> ActiveTraceTask = nullptr;

	/** 当前正在等待的交互输入任务。 */
	UPROPERTY(Transient)
	TObjectPtr<UAbilityTask_WaitInputPress> WaitInputPressTask = nullptr;

protected:
	/** 交互扫描频率。 */
	UPROPERTY(EditDefaultsOnly, Category = "AO|Interaction")
	float InteractionScanRate = 0.1f;

	/** 交互扫描距离。 */
	UPROPERTY(EditDefaultsOnly, Category = "AO|Interaction")
	float InteractionScanRange = 500.f;

	/** 默认交互提示 UI 类，作为后续扩展保留。 */
	UPROPERTY(EditDefaultsOnly, Category = "AO|Interaction")
	TSoftClassPtr<UUserWidget> DefaultInteractionWidgetClass;
};
