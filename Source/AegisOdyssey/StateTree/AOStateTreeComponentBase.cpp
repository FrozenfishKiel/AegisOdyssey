// Fill out your copyright notice in the Description page of Project Settings.

#include "AOStateTreeComponentBase.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOStateTreeComponentBase)

UAOStateTreeComponentBase::UAOStateTreeComponentBase()
{
	bStartLogicAutomatically = false;
	SetIsReplicatedByDefault(true);
}

void UAOStateTreeComponentBase::CallStateTreeToSentEvent(const FGameplayTag InTargetTag, const EInputType InInputType)
{
}
