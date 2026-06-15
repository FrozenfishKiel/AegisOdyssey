#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Widgets/SCompoundWidget.h"

class UMVVM_AIDecisionDebug;

// AI 决策调试专用 Slate 面板。
// 它只消费 HUD 已经整理好的调试 ViewModel，不直接参与任何正式游戏流程。
class SAOAIDecisionDebugPanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SAOAIDecisionDebugPanel)
		: _DebugViewModel(nullptr)
	{
	}
		SLATE_ARGUMENT(UMVVM_AIDecisionDebug*, DebugViewModel)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	FText GetTrackedActorText() const;
	FText GetSelectedIntentText() const;
	FText GetEvaluationInventoryText() const;
	FText GetQueueStateText() const;
	FText GetSubmittedStateText() const;

	FText BuildTagText(const FGameplayTag& InGameplayTag, const TCHAR* EmptyLabel) const;

private:
	TWeakObjectPtr<UMVVM_AIDecisionDebug> DebugViewModel;
};
