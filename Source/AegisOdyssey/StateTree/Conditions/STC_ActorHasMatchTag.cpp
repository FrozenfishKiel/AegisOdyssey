// Fill out your copyright notice in the Description page of Project Settings.


#include "STC_ActorHasMatchTag.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "StateTreeExecutionContext.h"
#include "AegisOdyssey/Character/AOCharacter.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(STC_ActorHasMatchTag)

bool FSTC_ActorHasMatchTag::TestCondition(FStateTreeExecutionContext& Context) const
{
	// 获取实例数据
	const FActorHasMatchTagInstanceData& InstanceData = Context.GetInstanceData<FActorHasMatchTagInstanceData>(*this);
	// 尝试获取状态树角色
	if (AAOCharacter* Character = Cast<AAOCharacter>(Context.GetOwner()))
	{
		//如果是本地控制的角色则获取其PlayerState
		if (Character->IsLocallyControlled())
		{
			if (UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Character))
			{
				bool bHasTag = SourceASC->HasMatchingGameplayTag(InstanceData.InTag);
				return bHasTag ^ bInvert;
			}
		}
	}
	return false ^ bInvert;
}
