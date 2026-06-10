// Fill out your copyright notice in the Description page of Project Settings.

#include "AegisOdyssey/Harvest/Items/AOHarvestToolInstance.h"

#include "AegisOdyssey/Harvest/Definition/AOHarvestToolDefinition.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOHarvestToolInstance)

UAOHarvestToolInstance::UAOHarvestToolInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

const UAOHarvestToolDefinition* UAOHarvestToolInstance::GetHarvestToolDefinition() const
{
	return Cast<UAOHarvestToolDefinition>(GetItemCDO());
}

const UAOHarvestToolFragment* UAOHarvestToolInstance::GetHarvestToolFragment() const
{
	const UAOHarvestToolDefinition* HarvestToolDefinition = GetHarvestToolDefinition();
	return HarvestToolDefinition ? HarvestToolDefinition->FindHarvestToolFragment() : nullptr;
}

const UAOHarvestToolProfile* UAOHarvestToolInstance::GetHarvestToolProfile() const
{
	const UAOHarvestToolDefinition* HarvestToolDefinition = GetHarvestToolDefinition();
	return HarvestToolDefinition ? HarvestToolDefinition->GetHarvestToolProfile() : nullptr;
}
