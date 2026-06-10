// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AegisOdyssey/Character/Enemies/AI/Decision/Inventory/AOAIInventoryDecisionTypes.h"
#include "AegisOdyssey/StateTree/AOStateTreeComponentBase.h"
#include "Components/StateTreeComponent.h"
#include "Components/StateTreeAIComponentSchema.h"
#include "AOAILogicStateTreeComponentBase.generated.h"

class UStateTree;
class UAOAIDecisionComponent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class AEGISODYSSEY_API UAOAILogicStateTreeComponentBase : public UAOStateTreeComponentBase
{
	GENERATED_BODY()

public:
	// 构造并初始化 AI 逻辑 StateTree 组件。
	UAOAILogicStateTreeComponentBase();
protected:
	// 开始运行时绑定库存决策事件。
	virtual void BeginPlay() override;

	// 初始化时应用默认 StateTree，并建立事件绑定。
	virtual void InitializeComponent() override;

	// 反初始化时移除事件绑定并停止逻辑。
	virtual void UninitializeComponent() override;

	// 结束播放时清理事件绑定和逻辑状态。
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// 清空 StateTree 实例数据。
	virtual void FullReset() override;

	// 当外部尚未显式指定 StateTree 资源时，自动套用默认资源。
	void ApplyDefaultStateTreeIfNeeded();

	// 绑定正式提交库存结果的变更事件。
	void BindInventoryDecisionEvents();

	// 解绑正式提交库存结果的变更事件。
	void UnbindInventoryDecisionEvents();

	// 把库存决策更新转换成 StateTree Event 推给运行中的逻辑树。
	void HandleSubmittedInventoryDecisionChanged(const FAOAIInventoryDecisionResult& SubmittedInventoryDecision);
public:
	// 组件逐帧 Tick。
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

protected:
	// 默认使用的 StateTree 资源。
	UPROPERTY(EditDefaultsOnly, Category = "AO AI|StateTree")
	TObjectPtr<UStateTree> DefaultStateTree;

	// 缓存的 AI 决策组件，用于管理库存决策事件绑定。
	UPROPERTY(Transient)
	TObjectPtr<UAOAIDecisionComponent> CachedDecisionComponent = nullptr;

	// 正式提交库存结果事件的绑定句柄。
	FDelegateHandle SubmittedInventoryDecisionChangedHandle;
};
