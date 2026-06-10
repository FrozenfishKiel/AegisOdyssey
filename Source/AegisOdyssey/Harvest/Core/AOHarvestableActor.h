// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AegisOdyssey/Harvest/Core/AOHarvestableTarget.h"
#include "GameFramework/Actor.h"
#include "AOHarvestableActor.generated.h"

class UAOHarvestableComponent;
class UPrimitiveComponent;
class USceneComponent;

USTRUCT()
struct FAOHarvestPrimitiveCollisionSnapshot
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<UPrimitiveComponent> PrimitiveComponent = nullptr;

	UPROPERTY()
	TEnumAsByte<ECollisionEnabled::Type> CollisionEnabled = ECollisionEnabled::NoCollision;
};

// 世界采集节点的标准对象基类。
// 运行时真相留在 HarvestableComponent，这里只负责对象层生命周期桥接。
UCLASS(Blueprintable, BlueprintType)
class AEGISODYSSEY_API AAOHarvestableActor : public AActor, public IAOHarvestableTarget
{
	GENERATED_BODY()

public:
	AAOHarvestableActor();
	virtual void BeginPlay() override;

	UAOHarvestableComponent* GetOwnedHarvestableComponent() const { return HarvestableComponent; }

	virtual UAOHarvestableComponent* GetHarvestableComponent_Implementation() const override;
	virtual void HandleHarvestNodeDepleted_Implementation(const FAOHarvestLifecycleContext& LifecycleContext) final override;
	virtual void HandleHarvestNodeRespawned_Implementation() final override;

protected:
	// 默认的公共 depleted 处理。
	// 这里先把所有 Primitive 从采集命中链里移出去，具体表现交给子类。
	UFUNCTION(BlueprintCallable, Category = "AO|Harvest")
	virtual void ApplyDefaultHarvestDepletedState();

	// 默认的公共 respawn 处理。
	// 这里恢复初始化时记录下来的碰撞快照，让节点先回到可采状态。
	UFUNCTION(BlueprintCallable, Category = "AO|Harvest")
	virtual void ApplyDefaultHarvestRespawnedState();

	// 固定对象层生命周期顺序：
	// 公共默认状态 -> C++ 节点族默认表现 -> 蓝图轻量补充。
	virtual void OnHarvestNodeDepletedNative(const FAOHarvestLifecycleContext& LifecycleContext);
	virtual void OnHarvestNodeRespawnedNative();

	UFUNCTION(BlueprintImplementableEvent, Category = "AO|Harvest", meta = (DisplayName = "On Harvest Node Depleted"))
	void ReceiveHarvestNodeDepleted(const FAOHarvestLifecycleContext& LifecycleContext);

	UFUNCTION(BlueprintImplementableEvent, Category = "AO|Harvest", meta = (DisplayName = "On Harvest Node Respawned"))
	void ReceiveHarvestNodeRespawned();

	// 遍历 Actor 上所有 Primitive 组件，统一切换碰撞开关。
	void SetHarvestPrimitivesCollisionEnabled(ECollisionEnabled::Type NewCollisionEnabled);
	void CapturePrimitiveCollisionSnapshot();
	void RestorePrimitiveCollisionSnapshot();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AO|Harvest")
	TObjectPtr<USceneComponent> RootScene = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AO|Harvest")
	TObjectPtr<UAOHarvestableComponent> HarvestableComponent = nullptr;

	UPROPERTY(Transient)
	TArray<FAOHarvestPrimitiveCollisionSnapshot> PrimitiveCollisionSnapshot;
};
