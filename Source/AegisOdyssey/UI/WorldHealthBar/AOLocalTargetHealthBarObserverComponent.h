#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AOLocalTargetHealthBarObserverComponent.generated.h"

class UAOTargetHealthBarComponent;
struct FAOCombatFeedbackViewData;

USTRUCT()
struct FAOLocalObservedTargetHealthBarEntry
{
	GENERATED_BODY()

	UPROPERTY()
	TWeakObjectPtr<AActor> TargetActor;

	UPROPERTY()
	TWeakObjectPtr<UAOTargetHealthBarComponent> TargetHealthBarComponent;

	UPROPERTY()
	float LastRelevantCombatTime = -1.0f;

	UPROPERTY()
	bool bWasRenderedLastRefresh = false;
};

// 本地玩家视角下的目标血条观察组件。
// 它只决定哪些目标应该被观察和显示，不维护目标生命值真相。
UCLASS(ClassGroup = (AO), meta = (BlueprintSpawnableComponent))
class AEGISODYSSEY_API UAOLocalTargetHealthBarObserverComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAOLocalTargetHealthBarObserverComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void TrackObservedTargetFromCombatFeedback(const FAOCombatFeedbackViewData& FeedbackViewData);

protected:
	void RefreshObservedTargets();
	AActor* GetLocalViewActor() const;
	bool IsActorInCombatDisplayState(const AActor* Actor) const;
	UAOTargetHealthBarComponent* TrackTargetFromCombatFeedback(const FAOCombatFeedbackViewData& FeedbackViewData);
	bool RefreshObservedTargetEntry(FAOLocalObservedTargetHealthBarEntry& Entry, float CurrentWorldTimeSeconds, const AActor* LocalPlayerActor) const;
	void HideAndForgetAllObservedTargets();

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AO|Target Health Bar", meta = (AllowPrivateAccess = "true", ClampMin = "0.0", UIMin = "0.0"))
	float HideDelaySeconds = 6.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AO|Target Health Bar", meta = (AllowPrivateAccess = "true", ClampMin = "0.0", UIMin = "0.0"))
	float ShowDistance = 1500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AO|Target Health Bar", meta = (AllowPrivateAccess = "true", ClampMin = "0.0", UIMin = "0.0"))
	float HideDistance = 2000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AO|Target Health Bar", meta = (AllowPrivateAccess = "true", ClampMin = "0.05", UIMin = "0.05"))
	float RefreshIntervalSeconds = 0.2f;

	UPROPERTY(Transient)
	TArray<FAOLocalObservedTargetHealthBarEntry> ObservedTargets;

	FTimerHandle RefreshObservedTargetsTimerHandle;
};
