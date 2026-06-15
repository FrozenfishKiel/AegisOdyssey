// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "AegisOdyssey/Character/Enemies/AI/Decision/AOAIDecisionComponent.h"
#include "AegisOdyssey/UI/ViewModel/AOMVVMViewModelBase.h"
#include "MVVM_AIDecisionDebug.generated.h"

/**
 * AI 决策调试 ViewModel。
 * 它只负责把运行时调试快照翻译成 HUD / UMG 可直接绑定的只读字段。
 */
UCLASS()
class AEGISODYSSEY_API UMVVM_AIDecisionDebug : public UAOMVVMViewModelBase
{
	GENERATED_BODY()

public:
	/** 把一份 AI 决策调试快照应用到当前 ViewModel。 */
	void ApplyDebugSnapshot(const FAOAIDecisionDebugSnapshot& InDebugSnapshot);

	/** 当前调试面板是否已经在跟踪某个 AI。 */
	UFUNCTION(BlueprintPure, FieldNotify, Category = "AO|AI|Debug")
	bool IsTrackingAI() const { return bTrackingAI; }

	/** 当前被跟踪 AI 的显示名。 */
	UFUNCTION(BlueprintPure, FieldNotify, Category = "AO|AI|Debug")
	FName GetTrackedActorName() const { return TrackedActorName; }

	/** 当前队列内待提交决策条目数量。 */
	UFUNCTION(BlueprintPure, FieldNotify, Category = "AO|AI|Debug")
	int32 GetDecisionQueueCount() const { return DecisionQueueCount; }
	UFUNCTION(BlueprintPure, FieldNotify, Category = "AO|AI|Debug")
	FGameplayTag GetSelectedIntentTag() const { return SelectedIntentTag; }
	UFUNCTION(BlueprintPure, FieldNotify, Category = "AO|AI|Debug")
	bool HasCurrentEvaluationInventoryDecision() const { return bHasCurrentEvaluationInventoryDecision; }
	UFUNCTION(BlueprintPure, FieldNotify, Category = "AO|AI|Debug")
	FGameplayTag GetCurrentEvaluationInventoryActionTag() const { return CurrentEvaluationInventoryActionTag; }

	/** 当前队首待提交决策标签。 */
	UFUNCTION(BlueprintPure, FieldNotify, Category = "AO|AI|Debug")
	FGameplayTag GetCurrentQueuedDecisionTag() const { return CurrentQueuedDecisionTag; }
	UFUNCTION(BlueprintPure, FieldNotify, Category = "AO|AI|Debug")
	FGameplayTag GetCurrentSubmittedDecisionTag() const { return CurrentSubmittedDecisionTag; }
	UFUNCTION(BlueprintPure, FieldNotify, Category = "AO|AI|Debug")
	FGameplayTag GetLastSubmittedDecisionTag() const { return LastSubmittedDecisionTag; }

	/** 当前是否存在已经正式提交的库存决策结果。 */
	UFUNCTION(BlueprintPure, FieldNotify, Category = "AO|AI|Debug")
	bool HasCurrentSubmittedInventoryDecision() const { return bHasCurrentSubmittedInventoryDecision; }

	/** 当前已经正式提交的库存动作标签。 */
	UFUNCTION(BlueprintPure, FieldNotify, Category = "AO|AI|Debug")
	FGameplayTag GetCurrentSubmittedInventoryActionTag() const { return CurrentSubmittedInventoryActionTag; }
	UFUNCTION(BlueprintPure, FieldNotify, Category = "AO|AI|Debug")
	float GetPendingSubmitDelaySeconds() const { return PendingSubmitDelaySeconds; }

private:
	/** 当前调试面板是否正在跟踪某个 AI。 */
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter = IsTrackingAI, Category = "AO|AI|Debug", meta = (AllowPrivateAccess))
	bool bTrackingAI = false;

	/** 当前被跟踪 AI 的显示名。 */
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter = GetTrackedActorName, Category = "AO|AI|Debug", meta = (AllowPrivateAccess))
	FName TrackedActorName = NAME_None;

	/** 当前统一决策队列中的条目数量。 */
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter = GetDecisionQueueCount, Category = "AO|AI|Debug", meta = (AllowPrivateAccess))
	int32 DecisionQueueCount = 0;
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter = GetSelectedIntentTag, Category = "AO|AI|Debug", meta = (AllowPrivateAccess))
	FGameplayTag SelectedIntentTag;
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter = HasCurrentEvaluationInventoryDecision, Category = "AO|AI|Debug", meta = (AllowPrivateAccess))
	bool bHasCurrentEvaluationInventoryDecision = false;
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter = GetCurrentEvaluationInventoryActionTag, Category = "AO|AI|Debug", meta = (AllowPrivateAccess))
	FGameplayTag CurrentEvaluationInventoryActionTag;

	/** 当前队首待提交决策标签。 */
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter = GetCurrentQueuedDecisionTag, Category = "AO|AI|Debug", meta = (AllowPrivateAccess))
	FGameplayTag CurrentQueuedDecisionTag;
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter = GetCurrentSubmittedDecisionTag, Category = "AO|AI|Debug", meta = (AllowPrivateAccess))
	FGameplayTag CurrentSubmittedDecisionTag;
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter = GetLastSubmittedDecisionTag, Category = "AO|AI|Debug", meta = (AllowPrivateAccess))
	FGameplayTag LastSubmittedDecisionTag;

	/** 当前是否存在已经正式提交的库存决策结果。 */
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter = HasCurrentSubmittedInventoryDecision, Category = "AO|AI|Debug", meta = (AllowPrivateAccess))
	bool bHasCurrentSubmittedInventoryDecision = false;

	/** 当前已经正式提交的库存动作标签。 */
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter = GetCurrentSubmittedInventoryActionTag, Category = "AO|AI|Debug", meta = (AllowPrivateAccess))
	FGameplayTag CurrentSubmittedInventoryActionTag;
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter = GetPendingSubmitDelaySeconds, Category = "AO|AI|Debug", meta = (AllowPrivateAccess))
	float PendingSubmitDelaySeconds = -1.0f;
};
