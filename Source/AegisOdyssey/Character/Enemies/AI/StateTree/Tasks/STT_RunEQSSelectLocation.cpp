#include "STT_RunEQSSelectLocation.h"

#include "AIController.h"
#include "AITypes.h"
#include "EnvironmentQuery/EnvQuery.h"
#include "EnvironmentQuery/EnvQueryManager.h"
#include "GameFramework/Pawn.h"
#include "StateTreeAsyncExecutionContext.h"
#include "StateTreeExecutionContext.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(STT_RunEQSSelectLocation)

FSTT_RunEQSSelectLocation::FSTT_RunEQSSelectLocation()
{
	bShouldCallTick = false;
	bShouldCopyBoundPropertiesOnTick = false;
	bShouldCopyBoundPropertiesOnExitState = false;
}

EStateTreeRunStatus FSTT_RunEQSSelectLocation::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	InstanceData.SelectedLocation = FAISystem::InvalidLocation;
	InstanceData.bHasValidLocation = false;
	InstanceData.QueryStatus = EEnvQueryStatus::Processing;
	InstanceData.RequestId = INDEX_NONE;

	if (InstanceData.QueryTemplate == nullptr)
	{
		UE_LOG(LogStateTree, Warning, TEXT("FSTT_RunEQSSelectLocation::EnterState: QueryTemplate is missing."));
		return EStateTreeRunStatus::Failed;
	}

	AActor* QuerierActor = ResolveQuerierActor(Context, InstanceData);
	if (QuerierActor == nullptr)
	{
		UE_LOG(LogStateTree, Warning, TEXT("FSTT_RunEQSSelectLocation::EnterState: QuerierActor is missing."));
		return EStateTreeRunStatus::Failed;
	}

	UEnvQueryManager* QueryManager = UEnvQueryManager::GetCurrent(QuerierActor);
	if (QueryManager == nullptr)
	{
		UE_LOG(LogStateTree, Warning, TEXT("FSTT_RunEQSSelectLocation::EnterState: EnvQueryManager is missing for %s."),
			*GetNameSafe(QuerierActor));
		return EStateTreeRunStatus::Failed;
	}

	FEnvQueryRequest QueryRequest(InstanceData.QueryTemplate, QuerierActor);
	QueryRequest.SetWorldOverride(QuerierActor->GetWorld());
	if (!InstanceData.NamedParams.IsEmpty())
	{
		QueryRequest.SetNamedParams(InstanceData.NamedParams);
	}

	InstanceData.RequestId = QueryRequest.Execute(
		InstanceData.RunMode,
		FQueryFinishedSignature::CreateLambda([WeakContext = Context.MakeWeakExecutionContext()](TSharedPtr<FEnvQueryResult> QueryResult) mutable
		{
			const FStateTreeStrongExecutionContext StrongContext = WeakContext.MakeStrongExecutionContext();
			if (FInstanceDataType* InstanceDataPtr = StrongContext.GetInstanceDataPtr<FInstanceDataType>())
			{
				InstanceDataPtr->RequestId = INDEX_NONE;
				InstanceDataPtr->SelectedLocation = FAISystem::InvalidLocation;
				InstanceDataPtr->bHasValidLocation = false;
				InstanceDataPtr->QueryStatus = QueryResult.IsValid() ? QueryResult->GetRawStatus() : EEnvQueryStatus::Failed;

				bool bSuccess = false;
				if (QueryResult.IsValid() && QueryResult->IsSuccessful() && QueryResult->Items.Num() > 0)
				{
					const FVector Location = QueryResult->GetItemAsLocation(0);
					InstanceDataPtr->SelectedLocation = Location;
					InstanceDataPtr->bHasValidLocation = true;

					if (FVector* SharedResult = InstanceDataPtr->Result.GetPtrFromStrongExecutionContext<FVector, true>(StrongContext))
					{
						*SharedResult = Location;
					}

					bSuccess = true;
				}

				if (!bSuccess && !InstanceDataPtr->bFailIfNoValidResult)
				{
					bSuccess = true;
				}

				StrongContext.FinishTask(bSuccess ? EStateTreeFinishTaskType::Succeeded : EStateTreeFinishTaskType::Failed);
			}
		}));

	return InstanceData.RequestId != INDEX_NONE ? EStateTreeRunStatus::Running : EStateTreeRunStatus::Failed;
}

void FSTT_RunEQSSelectLocation::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	if (InstanceData.RequestId != INDEX_NONE)
	{
		if (UEnvQueryManager* QueryManager = UEnvQueryManager::GetCurrent(Context.GetOwner()))
		{
			QueryManager->AbortQuery(InstanceData.RequestId);
		}

		InstanceData.RequestId = INDEX_NONE;
	}
}

AActor* FSTT_RunEQSSelectLocation::ResolveQuerierActor(const FStateTreeExecutionContext& Context, const FInstanceDataType& InstanceData) const
{
	if (InstanceData.QuerierActor != nullptr)
	{
		return InstanceData.QuerierActor;
	}

	AActor* OwnerActor = Cast<AActor>(Context.GetOwner());
	if (OwnerActor == nullptr)
	{
		return nullptr;
	}

	if (AAIController* AIController = Cast<AAIController>(OwnerActor))
	{
		if (APawn* ControlledPawn = AIController->GetPawn())
		{
			return ControlledPawn;
		}
	}

	if (APawn* OwnerPawn = Cast<APawn>(OwnerActor))
	{
		return OwnerPawn;
	}

	return OwnerActor;
}
