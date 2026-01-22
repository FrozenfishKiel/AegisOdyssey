// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AegisOdyssey/Interaction/InteractableTarget.h"
#include "AegisOdyssey/Interaction/InteractionOption.h"
#include "AegisOdyssey/Interaction/PickUpable.h"
#include "AOInventoryItemInstance.generated.h"

class UAOInventoryItemDefinition;
class UAOInventoryManagerComponent;
/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class AEGISODYSSEY_API UAOInventoryItemInstance : public UObject
{
	GENERATED_BODY()
public:
	UAOInventoryItemInstance(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	virtual void SetItemDef(TSubclassOf<UAOInventoryItemDefinition>InDef);
	UAOInventoryItemDefinition* GetItemCDO() const;

	friend struct FAOInventoryList;

public:

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	UPROPERTY(Replicated)
	TSubclassOf<UAOInventoryItemDefinition> ItemDef;
	UPROPERTY(BlueprintReadOnly,Replicated)
	TObjectPtr<UAOInventoryItemDefinition> ItemCDO;  //物品Definition的实例
public:
	virtual UAOInventoryManagerComponent* FindTargetInventoryManager() const;  //当前Instance隶属于哪个InventoryManager？
	virtual bool IsSupportedForNetworking() const override{return true;}
};

