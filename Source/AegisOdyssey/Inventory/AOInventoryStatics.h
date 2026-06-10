// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AegisOdyssey/Inventory/AOInventoryIteminstance.h"
#include "AegisOdyssey/Inventory/AOInventoryItemDefinition.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AOInventoryStatics.generated.h"

class UAOInventoryComponent;

USTRUCT(BlueprintType)
struct FAOInventoryDefinitionEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	int32 Count = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	TSubclassOf<UAOInventoryItemDefinition> ItemDefinitionClass = nullptr;

	// 可选的实例类覆盖项。
	// 留空时，库存接收链路会回到 ItemDefinition::PreferredInstanceType。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	TSubclassOf<UAOInventoryItemInstance> ItemInstanceClass = nullptr;
};

USTRUCT(BlueprintType)
struct FAOInventoryInstanceEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	int32 Count = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	TObjectPtr<UAOInventoryItemInstance> ItemInstance = nullptr;
};

USTRUCT(BlueprintType)
struct FAOInventoryReceiveBatch
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	TArray<FAOInventoryDefinitionEntry> DefinitionEntries;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	TArray<FAOInventoryInstanceEntry> InstanceEntries;

	bool IsEmpty() const
	{
		return DefinitionEntries.IsEmpty() && InstanceEntries.IsEmpty();
	}
};

UCLASS()
class AEGISODYSSEY_API UAOInventoryStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// 统一库存接收入口：判断 Actor 身上的某个库存组件能否完整接收这一批物品。
	static bool CanActorFullyAcceptInventoryBatch(const AActor* TargetActor, const FAOInventoryReceiveBatch& ReceiveBatch);

	// 统一库存接收入口：按库存组件顺序寻找第一个能完整接收整批物品的组件并正式入库。
	static bool TryAddInventoryBatchToActor(AActor* TargetActor, const FAOInventoryReceiveBatch& ReceiveBatch);

	// 收集对象当前暴露出的全部库存组件，并按统一入包优先级排序。
	static void AppendInventoryComponentsFromActor(const AActor* TargetActor, TArray<UAOInventoryComponent*>& OutInventoryComponents);

private:
	static UAOInventoryComponent* FindPreferredQuickBarInventoryComponent(const AActor* TargetActor, const TArray<UAOInventoryComponent*>& InventoryComponents);
	static void CollectInventoryComponentsFromActor(const AActor* TargetActor, TArray<UAOInventoryComponent*>& OutInventoryComponents);
};
