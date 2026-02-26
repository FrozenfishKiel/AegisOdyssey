#include "STT_PlayAnimation.h"
#include "StateTreeExecutionContext.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AegisOdyssey/Character/AOCharacter.h"
#include "AegisOdyssey/AbilitySystem/AOAbilitySystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(STT_PlayAnimation)

EStateTreeRunStatus FSTT_PlayAnimation::EnterState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	if (AAOCharacter* Character = Cast<AAOCharacter>(Context.GetOwner()))
	{
		if (Character->IsLocallyControlled())
		{
			if (UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Character))
			{
				SourceASC->AddLooseGameplayTag(InstanceData.StateTag);
			}

			if (USkeletalMeshComponent* Mesh = InstanceData.SkeletalMesh)
			{
				if (UAnimInstance* AnimInstance = Mesh->GetAnimInstance())
				{
					if (InstanceData.Montage)
					{
						AnimInstance->Montage_Play(
							InstanceData.Montage,
							InstanceData.PlayRate,
							EMontagePlayReturnType::MontageLength,
							InstanceData.StartTime,
							InstanceData.bStopAllMontages
						);
					}
				}
			}
		}
	}

	return EStateTreeRunStatus::Running;
}

void FSTT_PlayAnimation::ExitState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	if (AAOCharacter* Character = Cast<AAOCharacter>(Context.GetOwner()))
	{
		if (Character->IsLocallyControlled())
		{
			if (UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Character))
			{
				SourceASC->RemoveLooseGameplayTag(InstanceData.StateTag);
			}

			if (USkeletalMeshComponent* Mesh = InstanceData.SkeletalMesh)
			{
				if (UAnimInstance* AnimInstance = Mesh->GetAnimInstance())
				{
					if (InstanceData.Montage)
					{
						AnimInstance->Montage_Stop(0.5,InstanceData.Montage);
					}
				}
			}
		}
	}
}

EStateTreeRunStatus FSTT_PlayAnimation::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	if (USkeletalMeshComponent* Mesh = InstanceData.SkeletalMesh)
	{
		if (UAnimInstance* AnimInstance = Mesh->GetAnimInstance())
		{
			if (InstanceData.Montage)
			{
				if (!AnimInstance->Montage_IsPlaying(InstanceData.Montage))
				{
					return EStateTreeRunStatus::Succeeded;
				}
			}
		}
	}

	return EStateTreeRunStatus::Running;
}
