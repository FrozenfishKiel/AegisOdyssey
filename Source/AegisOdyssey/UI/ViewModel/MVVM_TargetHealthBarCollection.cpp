#include "MVVM_TargetHealthBarCollection.h"

#include "AegisOdyssey/UI/WorldHealthBar/AOLocalTargetHealthBarObserverComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MVVM_TargetHealthBarCollection)

UMVVM_TargetHealthBarCollection::UMVVM_TargetHealthBarCollection(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UMVVM_TargetHealthBarCollection::SetObserverComponent(UAOLocalTargetHealthBarObserverComponent* InObserverComponent)
{
	if (UE_MVVM_SET_PROPERTY_VALUE(ObserverComponent, InObserverComponent))
	{
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetObserverComponent);
	}
}
