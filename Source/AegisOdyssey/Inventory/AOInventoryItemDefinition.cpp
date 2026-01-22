// Fill out your copyright notice in the Description page of Project Settings.


#include "AOInventoryItemDefinition.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(AOInventoryItemDefinition)

UAOInventoryItemDefinition::UAOInventoryItemDefinition(const FObjectInitializer& ObjectInitializer)
{
	
}

UAOInventoryItemFragment* UAOInventoryItemDefinition::FindFragmentByClass(
	const TSubclassOf<UAOInventoryItemFragment> InItemClass) const
{
	check(InItemClass);
	UAOInventoryItemFragment* FoundFragment = nullptr;
	for (UAOInventoryItemFragment* FragmentsRef : Fragments)
	{
		if (FragmentsRef->IsA(InItemClass))
		{
			FoundFragment = FragmentsRef;
			break;
		}
	}
	return FoundFragment;
}

//从蓝图调用，传入物品的信息来获取物品的类引用
const UAOInventoryItemFragment* UAOBlueprintItemLibrary::FindFragmentByClass(
	TSubclassOf<UAOInventoryItemDefinition> ItemDef, TSubclassOf<UAOInventoryItemFragment> FragmentClass)
{
	if (ItemDef != nullptr && FragmentClass != nullptr)
	{
		
	}
	return nullptr;
}

