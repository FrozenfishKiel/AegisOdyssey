#pragma once

#include "CoreMinimal.h"
#include "AegisOdyssey/Harvest/Core/AOHarvestableActor.h"
#include "AOHarvestableBush.generated.h"

class UPrimitiveComponent;
struct FTimerHandle;

UENUM(BlueprintType)
enum class EAOHarvestBushDepletedDisposition : uint8
{
	HideBush UMETA(DisplayName = "Hide Bush"),
	KeepFlattenedBush UMETA(DisplayName = "Keep Flattened Bush"),
	DestroyActor UMETA(DisplayName = "Destroy Actor")
};

UCLASS(Blueprintable, BlueprintType)
class AEGISODYSSEY_API AAOHarvestableBush : public AAOHarvestableActor
{
	GENERATED_BODY()

public:
	AAOHarvestableBush();

	virtual void BeginPlay() override;

protected:
	virtual void OnHarvestNodeDepletedNative(const FAOHarvestLifecycleContext& LifecycleContext) override;
	virtual void OnHarvestNodeRespawnedNative() override;

	UFUNCTION(BlueprintCallable, Category = "AO|Harvest|Bush")
	void ApplyBushHitImpulse(const FAOHarvestLifecycleContext& LifecycleContext);

	UFUNCTION(BlueprintCallable, Category = "AO|Harvest|Bush")
	void SetBushVisualState(bool bVisible);

	UFUNCTION(BlueprintCallable, Category = "AO|Harvest|Bush")
	void SetBushPhysicsEnabled(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "AO|Harvest|Bush")
	void SetBushHarvestTraceBlocked(bool bBlocked);

	UFUNCTION(BlueprintCallable, Category = "AO|Harvest|Bush")
	void RestoreBushBodyCollisionFromSnapshot();

	UFUNCTION(BlueprintCallable, Category = "AO|Harvest|Bush")
	FVector ResolveBushHitDirection(const FAOHarvestLifecycleContext& LifecycleContext) const;

	UFUNCTION(BlueprintCallable, Category = "AO|Harvest|Bush")
	void ResetBushBodyMotion();

	void StartBushHideTimer();
	void ClearBushHideTimer();
	void HandleBushHideTimerFinished();
	float ResolveBushHideDelay() const;
	UPrimitiveComponent* ResolveBushBodyComponent() const;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AO|Harvest|Bush")
	TObjectPtr<UPrimitiveComponent> BushBodyComponent = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AO|Harvest|Bush")
	bool bEnablePhysicsOnDepleted = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AO|Harvest|Bush|HitTuning", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float HitImpulseStrength = 250.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AO|Harvest|Bush|HitTuning", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float HitImpulseUpwardRatio = 0.08f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AO|Harvest|Bush|HitTuning", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float DepletedLinearDamping = 6.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AO|Harvest|Bush|HitTuning", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float DepletedAngularDamping = 8.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AO|Harvest|Bush")
	EAOHarvestBushDepletedDisposition DepletedDisposition = EAOHarvestBushDepletedDisposition::HideBush;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AO|Harvest|Bush", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float HideAfterDepletedDelay = 0.25f;

private:
	FTimerHandle BushHideTimerHandle;
};
