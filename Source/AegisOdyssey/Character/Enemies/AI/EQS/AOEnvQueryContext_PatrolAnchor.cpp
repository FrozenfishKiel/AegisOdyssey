// Fill out your copyright notice in the Description page of Project Settings.

#include "AOEnvQueryContext_PatrolAnchor.h"

#include "AegisOdyssey/Player/AAOAIPlayerBotController.h"
#include "AITypes.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Point.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOEnvQueryContext_PatrolAnchor)

UAOEnvQueryContext_PatrolAnchor::UAOEnvQueryContext_PatrolAnchor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UAOEnvQueryContext_PatrolAnchor::ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const
{
	AActor* QueryOwnerActor = Cast<AActor>(QueryInstance.Owner.Get());
	AAOAIPlayerBotController* AIController = Cast<AAOAIPlayerBotController>(QueryOwnerActor);

	if (AIController == nullptr)
	{
		if (APawn* OwnerPawn = Cast<APawn>(QueryOwnerActor))
		{
			AIController = Cast<AAOAIPlayerBotController>(OwnerPawn->GetController());
		}
	}

	if (AIController == nullptr)
	{
		if (AController* GenericController = Cast<AController>(QueryOwnerActor))
		{
			AIController = Cast<AAOAIPlayerBotController>(GenericController);
		}
	}

	FVector AnchorLocation = FAISystem::InvalidLocation;
	if (AIController != nullptr && AIController->HasPatrolAnchorLocation())
	{
		AnchorLocation = AIController->GetPatrolAnchorLocation();
	}
	else if (QueryOwnerActor != nullptr)
	{
		// 即便还没显式配置锚点，也先退回到自身位置，避免 Patrol EQS 因为空上下文直接失效。
		AnchorLocation = QueryOwnerActor->GetActorLocation();
	}

	if (FAISystem::IsValidLocation(AnchorLocation))
	{
		UEnvQueryItemType_Point::SetContextHelper(ContextData, AnchorLocation);
	}
}
