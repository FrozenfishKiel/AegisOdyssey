#include "AOAnimStateData.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(AOAnimStateData)


const FAOAnimStateContainer& UAOAnimStateData::GetAnimStateContainer(const FGameplayTag TargetTag)
{
	return AnimStates.FindChecked(TargetTag);
}
