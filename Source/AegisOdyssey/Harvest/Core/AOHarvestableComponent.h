#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AegisOdyssey/Harvest/Core/AOHarvestTypes.h"
#include "AOHarvestableComponent.generated.h"

class UAOHarvestableDefinition;
struct FTimerHandle;

UCLASS(ClassGroup = ("AO"), BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class AEGISODYSSEY_API UAOHarvestableComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAOHarvestableComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	// 当前这个世界采集节点绑定的静态定义资源。
	UFUNCTION(BlueprintPure, Category = "AO|Harvest")
	UAOHarvestableDefinition* GetHarvestableDefinition() const { return HarvestableDefinition; }

	UFUNCTION(BlueprintPure, Category = "AO|Harvest")
	float GetTotalHarvestProgress() const;

	UFUNCTION(BlueprintPure, Category = "AO|Harvest")
	float GetCurrentHarvestProgress() const { return RuntimeState.CurrentProgress; }

	UFUNCTION(BlueprintPure, Category = "AO|Harvest")
	bool IsDepleted() const { return RuntimeState.bDepleted; }

	UFUNCTION(BlueprintPure, Category = "AO|Harvest")
	bool IsRespawnPending() const { return RuntimeState.bRespawnPending; }

	// 给命中结算使用的轻量入口判断。
	// 回答“这个节点此刻是否还能接受一次采集请求”。
	UFUNCTION(BlueprintCallable, Category = "AO|Harvest")
	bool CanAcceptHarvestRequest() const;

	// 把服务端 Resolver 已经算好的最终结果应用到节点运行时状态上。
	// 公共的耗尽/重生生命周期也从这里分发，Actor 子类只关心自己的表现逻辑。
	UFUNCTION(BlueprintCallable, Category = "AO|Harvest")
	bool ApplyHarvestResult(const FAOHarvestResult& HarvestResult);

	UFUNCTION(BlueprintCallable, Category = "AO|Harvest")
	bool ApplyHarvestResultWithContext(const FAOHarvestLifecycleContext& LifecycleContext);

	// 由节点组件自己基于当前运行时状态解析这次请求最终会扣多少。
	UFUNCTION(BlueprintCallable, Category = "AO|Harvest")
	bool ResolveHarvestProgressRequest(float RequestedProgress, FAOHarvestResult& InOutResult) const;

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastPlayHarvestCue(FVector_NetQuantize CueLocation, FVector_NetQuantizeNormal CueNormal, bool bDepletedAfterHit);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Harvest")
	TObjectPtr<UAOHarvestableDefinition> HarvestableDefinition = nullptr;

	UPROPERTY(ReplicatedUsing = OnRep_LastDepletedEvent, VisibleInstanceOnly, BlueprintReadOnly, Category = "Harvest")
	FAOHarvestReplicatedDepletedEvent LastDepletedEvent;

	UPROPERTY(ReplicatedUsing = OnRep_RuntimeState, VisibleInstanceOnly, BlueprintReadOnly, Category = "Harvest")
	FAOHarvestNodeRuntimeState RuntimeState;

protected:
	UFUNCTION()
	void OnRep_LastDepletedEvent();

	UFUNCTION()
	void OnRep_RuntimeState(const FAOHarvestNodeRuntimeState& PreviousState);

private:
	void UpdateReplicatedDepletedEvent(const FAOHarvestLifecycleContext& LifecycleContext);
	FAOHarvestLifecycleContext BuildLifecycleContextFromReplicatedDepletedEvent() const;
	void BroadcastNodeDepleted(const FAOHarvestLifecycleContext& LifecycleContext);
	void BroadcastNodeRespawned();
	void ResetHarvestNodeState();
	void StartRespawnTimerIfNeeded();
	void ClearRespawnTimer();
	void HandleRespawnTimerFinished();
	void RefreshRuntimeStateFromDefinition();

private:
	FTimerHandle RespawnTimerHandle;
};
