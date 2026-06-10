// Fill out your copyright notice in the Description page of Project Settings.

#include "AegisOdyssey/Harvest/Items/AOHarvestResourceItemInstance.h"

#include "AegisOdyssey/Harvest/Items/AOHarvestResourceDefinition.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOHarvestResourceItemInstance)

UAOHarvestResourceItemInstance::UAOHarvestResourceItemInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

const UAOHarvestResourceDefinition* UAOHarvestResourceItemInstance::GetHarvestResourceDefinition() const
{
	return Cast<UAOHarvestResourceDefinition>(GetItemCDO());
}
