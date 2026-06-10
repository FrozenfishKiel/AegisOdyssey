#include "STT_FindNearestTarget.h"
#include "StateTreeExecutionContext.h"
#include "AegisOdyssey/Player/AAOAIPlayerBotController.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(STT_FindNearestTarget)

EStateTreeRunStatus FSTT_FindNearestTarget::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	AActor* OwnerActor = Cast<AActor>(Context.GetOwner());
	AAOAIPlayerBotController* AIController = Cast<AAOAIPlayerBotController>(OwnerActor->GetInstigatorController());

	if (!OwnerActor)
	{
		return EStateTreeRunStatus::Failed;
	}

	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsWithTag(OwnerActor->GetWorld(), InstanceData.TargetTag, FoundActors);

	AActor* NearestTarget = nullptr;
	float NearestDistSq = FMath::Square(InstanceData.SearchRadius);

	for (AActor* PotentialTarget : FoundActors)
	{
		if (PotentialTarget && PotentialTarget != OwnerActor)
		{
			const float DistSq = FVector::DistSquared(OwnerActor->GetActorLocation(), PotentialTarget->GetActorLocation());
			if (DistSq < NearestDistSq)
			{
				NearestDistSq = DistSq;
				NearestTarget = PotentialTarget;
			}
		}
	}

	InstanceData.TargetActor = NearestTarget;

	if (AIController != nullptr)
	{
		AIController->SetCurrentTarget(NearestTarget);
	}

	if (NearestTarget)
	{
		UE_LOG(LogStateTree, Log, TEXT("FSTT_FindNearestTarget: Found target %s"), *NearestTarget->GetName());
		return EStateTreeRunStatus::Succeeded;
	}

	return EStateTreeRunStatus::Failed;
}
