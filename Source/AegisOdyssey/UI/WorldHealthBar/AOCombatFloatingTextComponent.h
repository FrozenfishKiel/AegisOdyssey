#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/WidgetComponent.h"
#include "AOCombatFloatingTextComponent.generated.h"

class UWidgetComponent;
class UUserWidget;
struct FAOCombatFeedbackViewData;

// 目标身上的世界跳字组件。
// 它只负责把已经整理好的战斗反馈显示成目标侧跳字，不参与血条显隐资格判定。
UCLASS(ClassGroup = (AO), meta = (BlueprintSpawnableComponent))
class AEGISODYSSEY_API UAOCombatFloatingTextComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAOCombatFloatingTextComponent();

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void DisplayWorldCombatFeedback(const FAOCombatFeedbackViewData& FeedbackViewData);

protected:
	void CleanupExpiredWidgets();
	UWidgetComponent* AcquireFloatingTextWidgetComponent(const FVector& WorldAnchorLocation);
	FVector ResolveFloatingTextWorldLocation(const FAOCombatFeedbackViewData& FeedbackViewData) const;

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AO|Combat UI", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UUserWidget> FloatingTextWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AO|Combat UI", meta = (AllowPrivateAccess = "true"))
	EWidgetSpace WidgetSpace = EWidgetSpace::Screen;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AO|Combat UI", meta = (AllowPrivateAccess = "true"))
	FVector FallbackWorldOffset = FVector(0.0f, 0.0f, 90.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AO|Combat UI", meta = (AllowPrivateAccess = "true", ClampMin = "1", UIMin = "1"))
	FIntPoint WidgetDrawSize = FIntPoint(240, 96);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AO|Combat UI", meta = (AllowPrivateAccess = "true", ClampMin = "1", UIMin = "1"))
	int32 MaxActiveFloatingTexts = 8;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UWidgetComponent>> FloatingTextWidgetPool;
};
