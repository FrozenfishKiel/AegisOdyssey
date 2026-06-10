// Fill out your copyright notice in the Description page of Project Settings.

#include "AegisOdyssey/Harvest/Items/AOHarvestResourceDefinition.h"

#include "AegisOdyssey/Harvest/Items/AOHarvestResourceItemInstance.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOHarvestResourceDefinition)

UAOHarvestResourceDefinition::UAOHarvestResourceDefinition(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PreferredInstanceType = UAOHarvestResourceItemInstance::StaticClass();
}
