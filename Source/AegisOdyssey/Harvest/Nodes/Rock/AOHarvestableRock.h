#pragma once

#include "CoreMinimal.h"
#include "AegisOdyssey/Harvest/Core/AOHarvestableActor.h"
#include "AOHarvestableRock.generated.h"

class UPrimitiveComponent;
struct FTimerHandle;

UENUM(BlueprintType)
enum class EAOHarvestRockDepletedDisposition : uint8
{
	HideRock UMETA(DisplayName = "Hide Rock"),
	KeepBrokenRock UMETA(DisplayName = "Keep Broken Rock"),
	DestroyActor UMETA(DisplayName = "Destroy Actor")
};

UCLASS(Blueprintable, BlueprintType)
class AEGISODYSSEY_API AAOHarvestableRock : public AAOHarvestableActor
{
	GENERATED_BODY()

public:
	AAOHarvestableRock();

	virtual void BeginPlay() override;

protected:
	virtual void OnHarvestNodeDepletedNative(const FAOHarvestLifecycleContext& LifecycleContext) override;
	virtual void OnHarvestNodeRespawnedNative() override;

	UFUNCTION(BlueprintCallable, Category = "AO|Harvest|Rock")
	void ApplyRockBreakImpulse(const FAOHarvestLifecycleContext& LifecycleContext);

	UFUNCTION(BlueprintCallable, Category = "AO|Harvest|Rock")
	void SetRockVisualState(bool bVisible);

	UFUNCTION(BlueprintCallable, Category = "AO|Harvest|Rock")
	void SetRockPhysicsEnabled(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "AO|Harvest|Rock")
	void SetRockHarvestTraceBlocked(bool bBlocked);

	UFUNCTION(BlueprintCallable, Category = "AO|Harvest|Rock")
	void RestoreRockBodyCollisionFromSnapshot();

	UFUNCTION(BlueprintCallable, Category = "AO|Harvest|Rock")
	FVector ResolveRockBreakDirection(const FAOHarvestLifecycleContext& LifecycleContext) const;

	UFUNCTION(BlueprintCallable, Category = "AO|Harvest|Rock")
	void ResetRockBodyMotion();

	void StartRockHideTimer();
	void ClearRockHideTimer();
	void HandleRockHideTimerFinished();
	float ResolveRockHideDelay() const;
	UPrimitiveComponent* ResolveRockBodyComponent() const;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AO|Harvest|Rock")
	TObjectPtr<UPrimitiveComponent> RockBodyComponent = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AO|Harvest|Rock")
	bool bEnablePhysicsOnDepleted = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AO|Harvest|Rock|BreakTuning", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float BreakImpulseStrength = 900.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AO|Harvest|Rock|BreakTuning", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float BreakImpulseUpwardRatio = 0.2f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AO|Harvest|Rock|BreakTuning", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float DepletedLinearDamping = 1.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AO|Harvest|Rock|BreakTuning", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float DepletedAngularDamping = 2.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AO|Harvest|Rock")
	EAOHarvestRockDepletedDisposition DepletedDisposition = EAOHarvestRockDepletedDisposition::HideRock;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AO|Harvest|Rock", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float HideAfterDepletedDelay = 5.0f;

private:
	FTimerHandle RockHideTimerHandle;
};
