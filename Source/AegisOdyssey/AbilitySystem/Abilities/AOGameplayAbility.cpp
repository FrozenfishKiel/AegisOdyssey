// Fill out your copyright notice in the Description page of Project Settings.


#include "AOGameplayAbility.h"
#include "AegisOdyssey/AbilitySystem/AOAbilitySystem.h"
#include "AegisOdyssey/Character/AOCharacter.h"
#include "AegisOdyssey/Character/AOHeroComponent.h"
#include "AegisOdyssey/Player/AOPlayerController.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(AOGameplayAbility)
UAOGameplayAbility::UAOGameplayAbility(const FObjectInitializer& ObjectInitializer)
{
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateNo;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	NetSecurityPolicy = EGameplayAbilityNetSecurityPolicy::ClientOrServer;
}

//获取本地ASC
UAOAbilitySystem* UAOGameplayAbility::GetAOAbilitySystem() const
{
	return (CurrentActorInfo ? Cast<UAOAbilitySystem>(CurrentActorInfo->AbilitySystemComponent) : nullptr);
}

AAOPlayerController* UAOGameplayAbility::GetAOPlayerControllerFromActorInfo() const
{
	return (CurrentActorInfo ? Cast<AAOPlayerController>(CurrentActorInfo->PlayerController) : nullptr);
}

AController* UAOGameplayAbility::GetControllerFromActorInfo() const
{
	if (CurrentActorInfo)
	{
		if (AController* PC = CurrentActorInfo->PlayerController.Get())
		{
			return PC;
		}

		AActor* TestActor = CurrentActorInfo->OwnerActor.Get();  //在所有者链中查找玩家控制器或者Pawn
		while (TestActor)
		{
			if (AController* C = Cast<AController>(TestActor))
			{
				return C;
			}

			if (APawn* Pawn = Cast<APawn>(TestActor))
			{
				return Pawn->GetController();
			}

			TestActor = TestActor->GetOwner();
		}
	}

	return nullptr;
}

AAOCharacter* UAOGameplayAbility::GetLyraCharacterFromActorInfo() const
{
	return (CurrentActorInfo ? Cast<AAOCharacter>(CurrentActorInfo->AvatarActor.Get()) : nullptr);
}

UAOHeroComponent* UAOGameplayAbility::GetHeroComponentFromActorInfo() const
{
	return(CurrentActorInfo ? UAOHeroComponent::FindHeroComponent(CurrentActorInfo->AvatarActor.Get()) : nullptr);
}

void UAOGameplayAbility::TryActivateAbilityOnSpawn(const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilitySpec& Spec) const
{
	// 对于实例化能力，使用实例的CurrentActivationInfo而不是Spec.ActivationInfo
	// Spec.ActivationInfo已被弃用，只适用于非实例化能力
	const bool bIsPredicting = (CurrentActivationInfo.ActivationMode == EGameplayAbilityActivationMode::Predicting);

	// Try to activate if activation policy is on spawn.
	if (ActorInfo && !Spec.IsActive() && !bIsPredicting && ActivationPolicy == EAOAbilityActivationPolicy::OnSpawn)
	{
		UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
		const AActor* AvatarActor = ActorInfo->AvatarActor.Get();

		// If avatar actor is torn off or about to die, don't try to activate until we get the new one.
		if (ASC && AvatarActor && !AvatarActor->GetTearOff() && (AvatarActor->GetLifeSpan() <= 0.0f))
		{
			const bool bIsLocalExecution = (NetExecutionPolicy == EGameplayAbilityNetExecutionPolicy::LocalPredicted) || (NetExecutionPolicy == EGameplayAbilityNetExecutionPolicy::LocalOnly);
			const bool bIsServerExecution = (NetExecutionPolicy == EGameplayAbilityNetExecutionPolicy::ServerOnly) || (NetExecutionPolicy == EGameplayAbilityNetExecutionPolicy::ServerInitiated);

			const bool bClientShouldActivate = ActorInfo->IsLocallyControlled() && bIsLocalExecution;
			const bool bServerShouldActivate = ActorInfo->IsNetAuthority() && bIsServerExecution;
			
			if (bClientShouldActivate || bServerShouldActivate)
			{
				ASC->TryActivateAbility(Spec.Handle);
			}
		}
	}
}

void UAOGameplayAbility::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);

	// 在服务器和客户端都调用TryActivateAbilityOnSpawn，但通过ActorInfo->IsNetAuthority()区分执行逻辑
	// 服务器负责权威激活，客户端负责预测激活
	//TryActivateAbilityOnSpawn(ActorInfo, Spec);
}

//尝试激活此GA实例的时候会先行通过此函数判断
bool UAOGameplayAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!ActorInfo || !ActorInfo->AbilitySystemComponent.IsValid())
	{
		return false;
	}

	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	//@TODO Possibly remove after setting up tag relationships
	UAOAbilitySystem* LyraASC = CastChecked<UAOAbilitySystem>(ActorInfo->AbilitySystemComponent.Get());
	return true;
}

void UAOGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	AActor* SourceActor = GetAvatarActorFromActorInfo();
}
