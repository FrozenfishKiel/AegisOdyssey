#include "STC_TargetWithinDistanceRange.h"

#include "StateTreeExecutionContext.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(STC_TargetWithinDistanceRange)

bool FSTC_TargetWithinDistanceRange::TestCondition(FStateTreeExecutionContext& Context) const
{
	const FTargetWithinDistanceRangeInstanceData& InstanceData = Context.GetInstanceData(*this);

	if (InstanceData.TargetActor == nullptr)
	{
		return false ^ bInvert;
	}

	const float MinDistance = FMath::Min(InstanceData.MinDistance, InstanceData.MaxDistance);
	const float MaxDistance = FMath::Max(InstanceData.MinDistance, InstanceData.MaxDistance);
	const bool bIsWithinRange = InstanceData.DistanceToTarget >= MinDistance
		&& InstanceData.DistanceToTarget <= MaxDistance;

	return bIsWithinRange ^ bInvert;
}
