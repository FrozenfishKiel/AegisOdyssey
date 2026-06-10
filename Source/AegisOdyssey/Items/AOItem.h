// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AegisOdyssey/Interaction/InteractableTarget.h"
#include "AegisOdyssey/Interaction/PickUpable.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Actor.h"
#include "AOItem.generated.h"

/**
 * 场景中的基础可拾取物品 Actor。
 * 它既是可交互对象，也是可拾取对象，统一通过交互接口响应玩家或 AI 的交互。
 */
UCLASS(Blueprintable, BlueprintType)
class AEGISODYSSEY_API AAOItem : public AActor, public IInteractableTarget, public IPickUpable
{
	GENERATED_BODY()

public:
	AAOItem();

	void SetStaticInventory(const FInventoryPickUp& InStaticInventory) { StaticInventory = InStaticInventory; }
	void SetInteractionText(const FText& InInteractionText) { Option.Text = InInteractionText; }

	/** 物品默认静态网格体。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AO|Item")
	TObjectPtr<UStaticMeshComponent> DefaultStaticMesh = nullptr;

	/** 物品默认骨骼网格体。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AO|Item")
	TObjectPtr<USkeletalMeshComponent> DefaultSkeletalMesh = nullptr;

	/** 用于交互检测与拾取碰撞的盒体。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AO|Item")
	TObjectPtr<UBoxComponent> PickUpBox = nullptr;

	/** 供派生类在生成后补充自己的表现配置。 */
	virtual void InitializeActorSpawnConfig() {}

	/** 装备系统把世界物 Actor 挂到角色身上后，统一切到“仅表现、不参与拾取碰撞”的状态。 */
	virtual void DisableEquippedPresentationCollision();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	/** 收集当前物品对外暴露的交互选项。 */
	virtual void GatherInteractionOptions(FInteractionOptionBuilder& OptionBuilder) override;

	/** 在执行交互前补充事件数据上下文。 */
	virtual void CustomizeInteractionEventData(const FGameplayTag& InteractionEventTag, FGameplayEventData& InOutEventData) override;

	/** 执行一次统一交互，拾取物品时由这里落到库存系统。 */
	virtual bool ExecuteInteraction(const FGameplayTag& InteractionEventTag, const FGameplayEventData& EventData) override;

	/** 返回当前物品可提供的拾取库存数据。 */
	virtual FInventoryPickUp GetPickUpInventory() const override;

protected:
	/** 该物品默认暴露给统一交互能力的交互选项。 */
	UPROPERTY(EditAnywhere, Category = "AO|Interaction")
	FInteractionOption Option;

	/** 该物品被拾取时写入库存系统的静态数据。 */
	UPROPERTY(EditAnywhere, Category = "AO|Interaction")
	FInventoryPickUp StaticInventory;
};
