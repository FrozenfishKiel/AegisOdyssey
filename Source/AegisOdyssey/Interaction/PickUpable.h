// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PickUpable.generated.h"

class UAOInventoryItemInstance;
class UAOInventoryItemDefinition;
class UAOInventoryComponent;
// This class does not need to be modified.

USTRUCT(BlueprintType)
struct FPickUpTemplate
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere)
	int32 StackCount = 1;
	
	UPROPERTY(EditAnywhere)
	TSubclassOf<UAOInventoryItemDefinition> ItemDef;

	UPROPERTY(EditAnywhere)
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
	UPROPERTY(EditAnywhere , BlueprintReadOnly)
	TArray<FPickUpInstance> Instances;

	UPROPERTY(EditAnywhere , BlueprintReadOnly)
	TArray<FPickUpTemplate> Templates;
};


UINTERFACE(MinimalAPI)
class UPickUpable : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class AEGISODYSSEY_API IPickUpable
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	virtual FInventoryPickUp GetPickUpInventory() const = 0;
};

UCLASS()
class UPickUpableStatics :public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	//纯函数对于蓝图来说就是void Func() const
	UFUNCTION(BlueprintPure)
	static TScriptInterface<IPickUpable> GetFirstPickUpableFromActor(AActor* TargetActor);

	static void AddPickupToInventory(UAOInventoryComponent* TargetInventoryManagerComp , TScriptInterface<IPickUpable>& PickUp , bool& bCheck);

	UFUNCTION(BlueprintCallable , BlueprintAuthorityOnly , meta = (WorldContext = "Abiliy"))
	static void FindCanAddPickUpToInventoryComponent(AActor* Instigator , UPARAM(ref) TScriptInterface<IPickUpable>& PickUp , UPARAM(ref) bool& bCheck);
};