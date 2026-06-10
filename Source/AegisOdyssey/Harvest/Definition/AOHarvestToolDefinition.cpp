#include "AegisOdyssey/Harvest/Definition/AOHarvestToolDefinition.h"

#include "AegisOdyssey/Harvest/Fragments/AOHarvestToolFragment.h"
#include "AegisOdyssey/Harvest/Items/AOHarvestToolInstance.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOHarvestToolDefinition)

UAOHarvestToolDefinition::UAOHarvestToolDefinition(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PreferredInstanceType = UAOHarvestToolInstance::StaticClass();
}

const UAOHarvestToolFragment* UAOHarvestToolDefinition::FindHarvestToolFragment() const
{
	return FindFragmentByClass<UAOHarvestToolFragment>();
}
