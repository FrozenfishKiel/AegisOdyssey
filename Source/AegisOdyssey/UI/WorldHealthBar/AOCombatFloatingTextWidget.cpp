#include "AOCombatFloatingTextWidget.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOCombatFloatingTextWidget)

void UAOCombatFloatingTextWidget::HandleWorldCombatFeedback(
	const FAOCombatFeedbackViewData& FeedbackViewData,
	const FText& RecommendedText,
	const FVector& WorldAnchorLocation)
{
	OnWorldCombatFeedbackReceived(FeedbackViewData, RecommendedText, WorldAnchorLocation);
}
