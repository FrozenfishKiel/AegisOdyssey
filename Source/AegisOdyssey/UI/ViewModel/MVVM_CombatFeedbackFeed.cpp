#include "MVVM_CombatFeedbackFeed.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MVVM_CombatFeedbackFeed)

namespace MVVMCombatFeedbackFeedPrivate
{
	constexpr int32 MaxPendingCombatFeedbackEntries = 16;
}

UMVVM_CombatFeedbackFeed::UMVVM_CombatFeedbackFeed(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UMVVM_CombatFeedbackFeed::ApplyCombatFeedbackViewData(const FAOCombatFeedbackViewData& FeedbackViewData)
{
	// 这里统一把一条“已完成本地过滤的战斗反馈”写进 ViewModel 数据面。
	// UI / 蓝图应通过绑定字段或主动消费队列来读取，而不是依赖事件式回调。
	LatestCombatFeedback = FeedbackViewData;
	PendingCombatFeedbackList.Add(LatestCombatFeedback);
	if (PendingCombatFeedbackList.Num() > MVVMCombatFeedbackFeedPrivate::MaxPendingCombatFeedbackEntries)
	{
		PendingCombatFeedbackList.RemoveAt(0, PendingCombatFeedbackList.Num() - MVVMCombatFeedbackFeedPrivate::MaxPendingCombatFeedbackEntries);
	}

	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetLatestCombatFeedback);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetPendingCombatFeedbackList);
}

TArray<FAOCombatFeedbackViewData> UMVVM_CombatFeedbackFeed::ConsumePendingCombatFeedbackList()
{
	TArray<FAOCombatFeedbackViewData> Result = PendingCombatFeedbackList;
	ClearPendingCombatFeedbackList();
	return Result;
}

void UMVVM_CombatFeedbackFeed::ClearPendingCombatFeedbackList()
{
	if (PendingCombatFeedbackList.IsEmpty())
	{
		return;
	}

	PendingCombatFeedbackList.Reset();
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetPendingCombatFeedbackList);
}
