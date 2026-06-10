// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PickUpable.generated.h"

class UAOInventoryItemInstance;
class UAOInventoryItemDefinition;
struct FAOInventoryReceiveBatch;

USTRUCT(BlueprintType)
struct FPickUpTemplate
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	int32 StackCount = 1;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UAOInventoryItemDefinition> ItemDef;

	// 正常情况下留空，让 ItemDef 自己决定默认的 Instance 类型。
	// 只有这条拾取模板必须覆盖 Definition 默认规则时，才填写这个字段。
	UPROPERTY(EditAnywhere, meta = (DisplayName = "Item Instance Override"))
	TSubclassOf<UAOInventoryItemInstance> ItemInstanceDef;
};

USTRUCT(BlueprintType)
struct FPickUpInstance
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	int32 StackCount = 1;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UAOInventoryItemInstance> Item = nullptr;
};

USTRUCT(BlueprintType)
struct FInventoryPickUp
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FPickUpInstance> Instances;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FPickUpTemplate> Templates;
};

UINTERFACE(MinimalAPI)
class UPickUpable : public UInterface
{
	GENERATED_BODY()
};

class AEGISODYSSEY_API IPickUpable
{
	GENERATED_BODY()

public:
	virtual FInventoryPickUp GetPickUpInventory() const = 0;
};

UCLASS()
class UPickUpableStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure)
	static TScriptInterface<IPickUpable> GetFirstPickUpableFromActor(AActor* TargetActor);

	// 将拾取内容转换成统一库存接收批次。
	// 拾取系统负责上层玩法语义，真正入库存的规则统一交给 Inventory 模块。
	static bool BuildInventoryReceiveBatchFromPickup(TScriptInterface<IPickUpable>& PickUp, FAOInventoryReceiveBatch& OutReceiveBatch);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	static void TryAddPickupToActorInventories(AActor* Instigator, UPARAM(ref) TScriptInterface<IPickUpable>& PickUp, UPARAM(ref) bool& bCheck);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, meta = (WorldContext = "Abiliy"))
	static void FindCanAddPickUpToInventoryComponent(AActor* Instigator, UPARAM(ref) TScriptInterface<IPickUpable>& PickUp, UPARAM(ref) bool& bCheck);
};
