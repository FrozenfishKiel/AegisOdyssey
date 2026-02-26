// Fill out your copyright notice in the Description page of Project Settings.


#include "GA_LightAttack.h"
#include "AegisOdyssey/StateTree/AOStateTreeComponentBase.h"
#include "AegisOdyssey/StateTree/CombatStateTree/AOCombatStateTree.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GA_LightAttack)

bool UGA_LightAttack::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	// 直接从ActorInfo获取AvatarActor
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!AvatarActor)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to get AvatarActor"));
		return false;
	}
	// 1. 检查ActorInfo是否有效
	if (!ActorInfo || !ActorInfo->AvatarActor.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid ActorInfo or AvatarActor"));
		return false;
	}

	// 2. 安全获取AvatarActor
	AActor* TargetActor = ActorInfo->AvatarActor.Get();
	if (!TargetActor)
	{
		UE_LOG(LogTemp, Error, TEXT("AvatarActor is null"));
		return false;
	}
	return true;
}

void UGA_LightAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                      const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                      const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	AActor* TargetActor = ActorInfo->AvatarActor.Get();
	for (auto Components : TargetActor->GetComponents())
	{
		if (Components->IsA(UAOCombatStateTree::StaticClass()))
		{
			UAOCombatStateTree* CombatStateTree = Cast<UAOCombatStateTree>(Components);
			check(CombatStateTree);

			FStateTreeEvent Event;
			Event.Tag = FGameplayTag::RequestGameplayTag(FName("Input.LightAttack"));
			CombatStateTree->SendStateTreeEvent(Event);
			EndAbility(CurrentSpecHandle,CurrentActorInfo,CurrentActivationInfo,true,false);
		}
	}
}
