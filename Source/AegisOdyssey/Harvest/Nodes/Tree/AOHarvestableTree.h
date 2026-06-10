// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AegisOdyssey/Harvest/Core/AOHarvestableActor.h"
#include "AOHarvestableTree.generated.h"

class UPrimitiveComponent;
struct FTimerHandle;

UENUM(BlueprintType)
enum class EAOHarvestTreeDepletedDisposition : uint8
{
	HideTree UMETA(DisplayName = "Hide Tree"),
	KeepFallenTree UMETA(DisplayName = "Keep Fallen Tree"),
	DestroyActor UMETA(DisplayName = "Destroy Actor")
};

// 树木专用的原生采集节点。
// 公共采集层只负责通知节点生命周期，树自己的默认表现统一收口在这里。
UCLASS(Blueprintable, BlueprintType)
class AEGISODYSSEY_API AAOHarvestableTree : public AAOHarvestableActor
{
	GENERATED_BODY()

public:
	AAOHarvestableTree();

	virtual void BeginPlay() override;

protected:
	virtual void OnHarvestNodeDepletedNative(const FAOHarvestLifecycleContext& LifecycleContext) override;
	virtual void OnHarvestNodeRespawnedNative() override;

	UFUNCTION(BlueprintCallable, Category = "AO|Harvest|Tree")
	void ApplyTreeFellImpulse(const FAOHarvestLifecycleContext& LifecycleContext);

	UFUNCTION(BlueprintCallable, Category = "AO|Harvest|Tree")
	void SetTreeVisualState(bool bVisible);

	UFUNCTION(BlueprintCallable, Category = "AO|Harvest|Tree")
	void SetTreePhysicsEnabled(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "AO|Harvest|Tree")
	void SetTreeHarvestTraceBlocked(bool bBlocked);

	UFUNCTION(BlueprintCallable, Category = "AO|Harvest|Tree")
	void RestoreTreeBodyCollisionFromSnapshot();

	UFUNCTION(BlueprintCallable, Category = "AO|Harvest|Tree")
	FVector ResolveTreeFallDirection(const FAOHarvestLifecycleContext& LifecycleContext) const;

	UFUNCTION(BlueprintCallable, Category = "AO|Harvest|Tree")
	FVector ResolveHarvesterPushDirection(const FAOHarvestLifecycleContext& LifecycleContext) const;

	UFUNCTION(BlueprintCallable, Category = "AO|Harvest|Tree")
	FVector ResolveTreeFallTorqueAxis(const FAOHarvestLifecycleContext& LifecycleContext) const;

	UFUNCTION(BlueprintCallable, Category = "AO|Harvest|Tree")
	void ResetTreeBodyMotion();

	void StartTreeHideTimer();
	void ClearTreeHideTimer();
	void HandleTreeHideTimerFinished();
	float ResolveTreeHideDelay() const;
	UPrimitiveComponent* ResolveTreeBodyComponent() const;

protected:
	// 树干的主物理体。
	// 蓝图可以替换或配置它，倒地方向与冲量默认都作用在它身上。
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AO|Harvest|Tree")
	TObjectPtr<UPrimitiveComponent> TreeBodyComponent = nullptr;

	// 树耗尽后先退出采集 Trace 命中链，再按配置决定最终表现。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AO|Harvest|Tree")
	bool bEnablePhysicsOnDepleted = true;

	// 树倒地扭矩强度，数值越大，起倒越猛。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AO|Harvest|Tree|FallTuning", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float FellImpulseStrength = 1200.0f;

	// 作为强度附加倍率保留，值越大，整体倒地扭矩越强。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AO|Harvest|Tree|FallTuning", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float UpwardImpulseRatio = 0.15f;

	// 倒地过程中线速度阻尼，越大越不容易出现横向滑步。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AO|Harvest|Tree|FallTuning", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float DepletedLinearDamping = 4.0f;

	// 倒地过程中角速度阻尼，越大越不容易扭过头或翻滚过猛。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AO|Harvest|Tree|FallTuning", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float DepletedAngularDamping = 3.0f;

	// 限制倒地阶段的最大角速度，防止单次施力过猛时转得太夸张。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AO|Harvest|Tree|FallTuning", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float MaxDepletedAngularVelocity = 180.0f;

	// 决定树耗尽后在场景里最终留下什么状态。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AO|Harvest|Tree")
	EAOHarvestTreeDepletedDisposition DepletedDisposition = EAOHarvestTreeDepletedDisposition::HideTree;

	// HideTree 不再是“立刻消失”，而是“先倒下，再延时隐藏”。
	// 这样默认树族资产不需要逐个补蓝图逻辑，也能先给出完整倒地反馈。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AO|Harvest|Tree", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float HideAfterDepletedDelay = 10.0f;

private:
	FTimerHandle TreeHideTimerHandle;
};
