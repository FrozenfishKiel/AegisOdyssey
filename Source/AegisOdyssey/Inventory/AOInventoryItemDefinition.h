// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AOInventoryItemDefinition.generated.h"

class UAOInventoryItemInstance;
/**
 * 
 */

UCLASS(DefaultToInstanced, EditInlineNew , Abstract)
class AEGISODYSSEY_API UAOInventoryItemFragment : public UObject
{
	GENERATED_BODY()
public:
	virtual void OnInstanceCreated(UAOInventoryItemInstance* InItemInstance) const {}
	
};
UCLASS(BlueprintType , Blueprintable)
class AEGISODYSSEY_API UAOInventoryItemDefinition : public UObject
{
	GENERATED_BODY()

public:
	UAOInventoryItemDefinition(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UPROPERTY(EditDefaultsOnly , BlueprintReadOnly , Category = "Config")
	FName DisplayName;

	UPROPERTY(EditDefaultsOnly , BlueprintReadOnly , Category = "Config")
	TArray<TObjectPtr<UAOInventoryItemFragment>> Fragments;
public:
	
	UFUNCTION(BlueprintCallable,Category = "Components" , meta = (DeterminesOutputType = "InItemClass"))
	UAOInventoryItemFragment* FindFragmentByClass(const TSubclassOf<UAOInventoryItemFragment> InItemClass) const;

	
	template<typename T>
	const T *FindFragmentByClass() const;

	virtual bool IsSupportedForNetworking() const override{return true;}

};

template <typename T>
const T* UAOInventoryItemDefinition::FindFragmentByClass() const
{
	static_assert(TPointerIsConvertibleFromTo<T,const UAOInventoryItemFragment>::Value);
	return (T*)FindFragmentByClass(T::StaticClass());
}

UCLASS()
class AEGISODYSSEY_API UAOBlueprintItemLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable , meta = (DeterminesOutputType = FragmentClass))
	static const UAOInventoryItemFragment* FindFragmentByClass(TSubclassOf<UAOInventoryItemDefinition> ItemDef ,
		TSubclassOf<UAOInventoryItemFragment> FragmentClass);
};