#pragma once

#include "CoreMinimal.h"
#include "AegisOdyssey/UI/ViewModel/AOCombatFeedbackViewData.h"
#include "Blueprint/UserWidget.h"
#include "AOCombatFloatingTextWidget.generated.h"

// 目标侧世界跳字的基础 Widget。
// C++ 只负责把已经完成本地路由的反馈和锚点位置送进来，具体字效交给蓝图。
UCLASS(Abstract, Blueprintable)
class AEGISODYSSEY_API UAOCombatFloatingTextWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "AO|Combat UI")
	void HandleWorldCombatFeedback(
		const FAOCombatFeedbackViewData& FeedbackViewData,
		const FText& RecommendedText,
		const FVector& WorldAnchorLocation);

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "AO|Combat UI")
	void OnWorldCombatFeedbackReceived(
		const FAOCombatFeedbackViewData& FeedbackViewData,
		const FText& RecommendedText,
		const FVector& WorldAnchorLocation);
};
