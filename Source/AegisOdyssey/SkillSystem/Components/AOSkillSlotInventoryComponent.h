// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AegisOdyssey/Inventory/AOInventoryComponent.h"
#include "Components/GameFrameworkInitStateInterface.h"
#include "AOSkillSlotInventoryComponent.generated.h"

class UAOSkillComponent;

/**
 * 技能槽库存适配层。
 *
 * 这个组件只做一件事：
 * 把“技能槽里当前放了什么来源物品”投影成库存结构，
 * 再把这份投影结果同步回 SkillComponent 作为正式运行时输入。
 *
 * 它自己不维护技能实例真相，也不负责授予 Ability 或处理冷却。
 */
UCLASS(BlueprintType, meta = (BlueprintSpawnableComponent))
class AEGISODYSSEY_API UAOSkillSlotInventoryComponent : public UAOInventoryComponent, public IGameFrameworkInitStateInterface
{
	GENERATED_BODY()

public:
	static const FName NAME_ActorFeatureName;

	UAOSkillSlotInventoryComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual FName GetFeatureName() const override { return NAME_ActorFeatureName; }
	virtual UAOInventoryComponent* GetInventoryComponent() override { return this; }

	// 读取当前角色身上的技能运行时组件。
	UFUNCTION(BlueprintPure, Category = "Skill")
	UAOSkillComponent* GetOwningSkillComponent() const;

	// 按当前库存投影结果重建一次技能运行时真相。
	UFUNCTION(BlueprintCallable, Category = "Skill")
	void SyncSkillRuntimeFromInventoryProjection();

	virtual void BroadCastInventoryChange(int32 ChangedIndex = 0) override;

protected:
	virtual void BeginPlay() override;
	virtual void OnRegister() override;
	virtual void InitializeParams() override;
	virtual void InitializeOrRefreshInventorySlots() override;
	virtual void OnActorInitStateChanged(const FActorInitStateChangedParams& Params) override;
	virtual void HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) override;
	virtual void CheckDefaultInitialization() override;
};
