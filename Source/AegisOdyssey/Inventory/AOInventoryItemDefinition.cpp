// Fill out your copyright notice in the Description page of Project Settings.


#include "AOInventoryItemDefinition.h"

#include "AegisOdyssey/Inventory/AOInventoryItemInstance.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOInventoryItemDefinition)

UAOInventoryItemDefinition::UAOInventoryItemDefinition(const FObjectInitializer& ObjectInitializer)
{
}

UAOInventoryItemFragment* UAOInventoryItemDefinition::FindFragmentByClass(
	const TSubclassOf<UAOInventoryItemFragment> InItemClass) const
{
	check(InItemClass);

	for (UAOInventoryItemFragment* FragmentsRef : Fragments)
	{
		if (FragmentsRef != nullptr && FragmentsRef->IsA(InItemClass))
		{
			return FragmentsRef;
		}
	}

	return nullptr;
}

bool UAOInventoryItemDefinition::HasSemanticTag(FGameplayTag Tag, bool bExactMatch) const
{
	if (!Tag.IsValid())
	{
		return false;
	}

	return bExactMatch ? SemanticTags.HasTagExact(Tag) : SemanticTags.HasTag(Tag);
}

TSubclassOf<UAOInventoryItemInstance> UAOInventoryItemDefinition::GetPreferredInstanceType() const
{
	if (PreferredInstanceType != nullptr)
	{
		return PreferredInstanceType;
	}

	return UAOInventoryItemInstance::StaticClass();
}

TSubclassOf<UAOInventoryItemInstance> UAOInventoryItemDefinition::ResolveItemInstanceClass(
	TSubclassOf<UAOInventoryItemDefinition> ItemDefinitionClass,
	TSubclassOf<UAOInventoryItemInstance> ItemInstanceOverrideClass)
{
	if (ItemInstanceOverrideClass != nullptr)
	{
		return ItemInstanceOverrideClass;
	}

	if (ItemDefinitionClass == nullptr)
	{
		return nullptr;
	}

	const UAOInventoryItemDefinition* ItemDefinition = GetDefault<UAOInventoryItemDefinition>(ItemDefinitionClass);
	return ItemDefinition ? ItemDefinition->GetPreferredInstanceType() : nullptr;
}

// 浠庤摑鍥捐皟鐢紝浼犲叆鐗╁搧鐨勪俊鎭潵鑾峰彇鐗╁搧鐨勭被寮曠敤
const UAOInventoryItemFragment* UAOBlueprintItemLibrary::FindFragmentByClass(
	TSubclassOf<UAOInventoryItemDefinition> ItemDef,
	TSubclassOf<UAOInventoryItemFragment> FragmentClass)
{
	if (ItemDef != nullptr && FragmentClass != nullptr)
	{
		if (const UAOInventoryItemDefinition* ItemDefinition = GetDefault<UAOInventoryItemDefinition>(ItemDef))
		{
			return ItemDefinition->FindFragmentByClass(FragmentClass);
		}
	}

	return nullptr;
}
