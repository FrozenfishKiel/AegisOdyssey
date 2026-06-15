#include "AegisOdyssey/UI/AIDebug/SAOAIDecisionDebugPanel.h"

#include "AegisOdyssey/UI/ViewModel/AIDebug/MVVM_AIDecisionDebug.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

namespace AOAIDebugPanelPrivate
{
	static TSharedRef<STextBlock> MakeValueText(TAttribute<FText> InText, const int32 FontSize = 11)
	{
		return SNew(STextBlock)
			.Text(InText)
			.AutoWrapText(true)
			.Font(FCoreStyle::GetDefaultFontStyle("Regular", FontSize))
			.ColorAndOpacity(FLinearColor(0.86f, 0.91f, 0.95f));
	}

	static TSharedRef<STextBlock> MakeHeaderText(const FText& InText)
	{
		return SNew(STextBlock)
			.Text(InText)
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 13))
			.ColorAndOpacity(FLinearColor(0.96f, 0.84f, 0.41f));
	}
}

void SAOAIDecisionDebugPanel::Construct(const FArguments& InArgs)
{
	DebugViewModel = InArgs._DebugViewModel;

	ChildSlot
	[
		SNew(SBox)
		.WidthOverride(420.0f)
		[
			SNew(SBorder)
			.Padding(FMargin(14.0f))
			.BorderBackgroundColor(FLinearColor(0.02f, 0.03f, 0.05f, 0.92f))
			[
				SNew(SVerticalBox)

				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					AOAIDebugPanelPrivate::MakeHeaderText(FText::FromString(TEXT("AI Decision Debug")))
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.0f, 8.0f, 0.0f, 0.0f)
				[
					AOAIDebugPanelPrivate::MakeValueText(
						TAttribute<FText>::CreateSP(this, &SAOAIDecisionDebugPanel::GetTrackedActorText))
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.0f, 10.0f, 0.0f, 6.0f)
				[
					SNew(SSeparator)
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					AOAIDebugPanelPrivate::MakeHeaderText(FText::FromString(TEXT("Evaluation")))
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.0f, 6.0f, 0.0f, 0.0f)
				[
					AOAIDebugPanelPrivate::MakeValueText(
						TAttribute<FText>::CreateSP(this, &SAOAIDecisionDebugPanel::GetSelectedIntentText))
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.0f, 4.0f, 0.0f, 0.0f)
				[
					AOAIDebugPanelPrivate::MakeValueText(
						TAttribute<FText>::CreateSP(this, &SAOAIDecisionDebugPanel::GetEvaluationInventoryText))
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.0f, 10.0f, 0.0f, 6.0f)
				[
					SNew(SSeparator)
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					AOAIDebugPanelPrivate::MakeHeaderText(FText::FromString(TEXT("Queue")))
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.0f, 6.0f, 0.0f, 0.0f)
				[
					AOAIDebugPanelPrivate::MakeValueText(
						TAttribute<FText>::CreateSP(this, &SAOAIDecisionDebugPanel::GetQueueStateText))
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.0f, 10.0f, 0.0f, 6.0f)
				[
					SNew(SSeparator)
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					AOAIDebugPanelPrivate::MakeHeaderText(FText::FromString(TEXT("Submitted")))
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.0f, 6.0f, 0.0f, 0.0f)
				[
					AOAIDebugPanelPrivate::MakeValueText(
						TAttribute<FText>::CreateSP(this, &SAOAIDecisionDebugPanel::GetSubmittedStateText))
				]
			]
		]
	];
}

FText SAOAIDecisionDebugPanel::GetTrackedActorText() const
{
	const UMVVM_AIDecisionDebug* ViewModel = DebugViewModel.Get();
	if (ViewModel == nullptr || !ViewModel->IsTrackingAI())
	{
		return FText::FromString(TEXT("Tracked AI: None"));
	}

	return FText::FromString(FString::Printf(TEXT("Tracked AI: %s"), *ViewModel->GetTrackedActorName().ToString()));
}

FText SAOAIDecisionDebugPanel::GetSelectedIntentText() const
{
	const UMVVM_AIDecisionDebug* ViewModel = DebugViewModel.Get();
	if (ViewModel == nullptr)
	{
		return FText::FromString(TEXT("Selected Intent: None"));
	}

	return FText::FromString(FString::Printf(
		TEXT("Selected Intent: %s"),
		*BuildTagText(ViewModel->GetSelectedIntentTag(), TEXT("None")).ToString()));
}

FText SAOAIDecisionDebugPanel::GetEvaluationInventoryText() const
{
	const UMVVM_AIDecisionDebug* ViewModel = DebugViewModel.Get();
	if (ViewModel == nullptr || !ViewModel->HasCurrentEvaluationInventoryDecision())
	{
		return FText::FromString(TEXT("Inventory Eval: None"));
	}

	return FText::FromString(FString::Printf(
		TEXT("Inventory Eval: %s"),
		*BuildTagText(ViewModel->GetCurrentEvaluationInventoryActionTag(), TEXT("None")).ToString()));
}

FText SAOAIDecisionDebugPanel::GetQueueStateText() const
{
	const UMVVM_AIDecisionDebug* ViewModel = DebugViewModel.Get();
	if (ViewModel == nullptr)
	{
		return FText::FromString(TEXT("Queue: unavailable"));
	}

	return FText::FromString(FString::Printf(
		TEXT("Queue Count: %d\nQueue Head: %s\nSubmit Delay: %.2fs"),
		ViewModel->GetDecisionQueueCount(),
		*BuildTagText(ViewModel->GetCurrentQueuedDecisionTag(), TEXT("None")).ToString(),
		ViewModel->GetPendingSubmitDelaySeconds()));
}

FText SAOAIDecisionDebugPanel::GetSubmittedStateText() const
{
	const UMVVM_AIDecisionDebug* ViewModel = DebugViewModel.Get();
	if (ViewModel == nullptr)
	{
		return FText::FromString(TEXT("Submitted: unavailable"));
	}

	return FText::FromString(FString::Printf(
		TEXT("Current Submitted: %s\nLast Submitted: %s\nInventory Submitted: %s"),
		*BuildTagText(ViewModel->GetCurrentSubmittedDecisionTag(), TEXT("None")).ToString(),
		*BuildTagText(ViewModel->GetLastSubmittedDecisionTag(), TEXT("None")).ToString(),
		*BuildTagText(ViewModel->GetCurrentSubmittedInventoryActionTag(), TEXT("None")).ToString()));
}

FText SAOAIDecisionDebugPanel::BuildTagText(const FGameplayTag& InGameplayTag, const TCHAR* EmptyLabel) const
{
	return InGameplayTag.IsValid() ? FText::FromName(InGameplayTag.GetTagName()) : FText::FromString(EmptyLabel);
}
