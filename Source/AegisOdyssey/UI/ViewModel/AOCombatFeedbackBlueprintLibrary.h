#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AOCombatFeedbackViewData.h"
#include "AOCombatFeedbackBlueprintLibrary.generated.h"

class UMVVM_CombatFeedbackFeed;
class UMVVM_CombatResources;
class UMVVM_Crafting;
class UMVVM_HUD;
class UMVVM_ItemHoverTooltip;
class UMVVM_LocalCombatState;
class UMVVM_TargetHealthBarCollection;

UCLASS()
class AEGISODYSSEY_API UAOCombatFeedbackBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "AO|Combat UI", meta = (WorldContext = "WorldContextObject"))
	static UMVVM_HUD* GetMainHUDViewModel(const UObject* WorldContextObject);

	UFUNCTION(BlueprintPure, Category = "AO|Combat UI", meta = (WorldContext = "WorldContextObject"))
	static UMVVM_CombatResources* GetCombatResourcesViewModel(const UObject* WorldContextObject);

	UFUNCTION(BlueprintPure, Category = "AO|Combat UI", meta = (WorldContext = "WorldContextObject"))
	static UMVVM_LocalCombatState* GetLocalCombatStateViewModel(const UObject* WorldContextObject);

	UFUNCTION(BlueprintPure, Category = "AO|Combat UI", meta = (WorldContext = "WorldContextObject"))
	static UMVVM_CombatFeedbackFeed* GetCombatFeedbackFeedViewModel(const UObject* WorldContextObject);

	UFUNCTION(BlueprintPure, Category = "AO|Combat UI", meta = (WorldContext = "WorldContextObject"))
	static UMVVM_TargetHealthBarCollection* GetTargetHealthBarCollectionViewModel(const UObject* WorldContextObject);

	// 制作系统蓝图获取 Crafting ViewModel 的通用入口。
	// 继承 UAOCraftingWidgetBase 的 Widget 内部默认也走这条链取数。
	UFUNCTION(BlueprintPure, Category = "AO|Crafting UI", meta = (WorldContext = "WorldContextObject"))
	static UMVVM_Crafting* GetCraftingViewModel(const UObject* WorldContextObject);

	// 物品悬浮 Tooltip 的 HUD 级通用获取入口。
	// 各个库存格子、制造条目等只要拿得到 Definition，都应复用这一份 ViewModel。
	UFUNCTION(BlueprintPure, Category = "AO|Inventory UI", meta = (WorldContext = "WorldContextObject"))
	static UMVVM_ItemHoverTooltip* GetItemHoverTooltipViewModel(const UObject* WorldContextObject);

	UFUNCTION(BlueprintPure, Category = "AO|Combat UI")
	static bool ShouldDisplayFloatingText(const FAOCombatFeedbackViewData& Feedback);

	UFUNCTION(BlueprintPure, Category = "AO|Combat UI")
	static bool IsLocalRelevantFeedback(const FAOCombatFeedbackViewData& Feedback);

	UFUNCTION(BlueprintPure, Category = "AO|Combat UI")
	static bool IsLocalInstigatorFeedback(const FAOCombatFeedbackViewData& Feedback);

	UFUNCTION(BlueprintPure, Category = "AO|Combat UI")
	static bool IsLocalTargetFeedback(const FAOCombatFeedbackViewData& Feedback);

	UFUNCTION(BlueprintPure, Category = "AO|Combat UI")
	static bool ShouldRouteToHUD(const FAOCombatFeedbackViewData& Feedback);

	UFUNCTION(BlueprintPure, Category = "AO|Combat UI")
	static bool ShouldRouteToWorldFloatingText(const FAOCombatFeedbackViewData& Feedback);

	UFUNCTION(BlueprintPure, Category = "AO|Combat UI")
	static bool IsImportantCombatFeedback(const FAOCombatFeedbackViewData& Feedback);

	UFUNCTION(BlueprintPure, Category = "AO|Combat UI")
	static bool IsDamageFeedback(const FAOCombatFeedbackViewData& Feedback);

	UFUNCTION(BlueprintPure, Category = "AO|Combat UI")
	static bool IsDefensiveFeedback(const FAOCombatFeedbackViewData& Feedback);

	UFUNCTION(BlueprintPure, Category = "AO|Combat UI")
	static FText BuildRecommendedCombatText(const FAOCombatFeedbackViewData& Feedback);

	UFUNCTION(BlueprintPure, Category = "AO|Combat UI")
	static bool IsFeedbackRelatedToActor(const FAOCombatFeedbackViewData& Feedback, const AActor* Actor);

	UFUNCTION(BlueprintCallable, Category = "AO|Combat UI")
	static TArray<FAOCombatFeedbackViewData> ConsumePendingCombatFeedbackFromFeed(UMVVM_CombatFeedbackFeed* CombatFeedbackFeedViewModel);

	UFUNCTION(BlueprintCallable, Category = "AO|Combat UI", meta = (DeprecatedFunction, DeprecationMessage = "Use ConsumePendingCombatFeedbackFromFeed with UMVVM_CombatFeedbackFeed instead."))
	static TArray<FAOCombatFeedbackViewData> ConsumePendingCombatFeedback(UMVVM_HUD* HUDViewModel);
};
