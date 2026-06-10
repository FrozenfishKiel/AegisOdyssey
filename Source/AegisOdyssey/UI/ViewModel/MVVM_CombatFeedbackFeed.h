#pragma once

#include "CoreMinimal.h"
#include "AOMVVMViewModelBase.h"
#include "AOCombatFeedbackViewData.h"
#include "MVVM_CombatFeedbackFeed.generated.h"

// 本地战斗反馈流 ViewModel。
// 它只负责“已完成本地过滤的反馈流”作为 ViewModel 数据对外暴露，
// 不再承担事件式分发职责，也不承接资源条或状态条。
UCLASS(Blueprintable)
class AEGISODYSSEY_API UMVVM_CombatFeedbackFeed : public UAOMVVMViewModelBase
{
	GENERATED_BODY()

public:
	UMVVM_CombatFeedbackFeed(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	void ApplyCombatFeedbackViewData(const FAOCombatFeedbackViewData& FeedbackViewData);

	UFUNCTION(BlueprintPure, FieldNotify, Category = "AO|Combat Feedback")
	const FAOCombatFeedbackViewData& GetLatestCombatFeedback() const { return LatestCombatFeedback; }

	UFUNCTION(BlueprintPure, FieldNotify, Category = "AO|Combat Feedback")
	TArray<FAOCombatFeedbackViewData> GetPendingCombatFeedbackList() const { return PendingCombatFeedbackList; }

	UFUNCTION(BlueprintCallable, Category = "AO|Combat Feedback")
	TArray<FAOCombatFeedbackViewData> ConsumePendingCombatFeedbackList();

	UFUNCTION(BlueprintCallable, Category = "AO|Combat Feedback")
	void ClearPendingCombatFeedbackList();

private:
	// 最近一次已进入本地反馈流的战斗反馈。
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter = GetLatestCombatFeedback, Category = "AO|Combat Feedback", meta = (AllowPrivateAccess))
	FAOCombatFeedbackViewData LatestCombatFeedback;

	// 待一次性消费的瞬时反馈列表。
	// 表现层应通过绑定或主动消费这份 ViewModel 数据工作，而不是再回退到事件驱动。
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter = GetPendingCombatFeedbackList, Category = "AO|Combat Feedback", meta = (AllowPrivateAccess))
	TArray<FAOCombatFeedbackViewData> PendingCombatFeedbackList;
};
