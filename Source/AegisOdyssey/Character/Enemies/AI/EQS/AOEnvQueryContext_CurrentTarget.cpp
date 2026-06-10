// Fill out your copyright notice in the Description page of Project Settings.

#include "AOEnvQueryContext_CurrentTarget.h"

#include "AegisOdyssey/Player/AAOAIPlayerBotController.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Actor.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOEnvQueryContext_CurrentTarget)

UAOEnvQueryContext_CurrentTarget::UAOEnvQueryContext_CurrentTarget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UAOEnvQueryContext_CurrentTarget::ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const
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

	AActor* CurrentTarget = AIController ? AIController->GetCurrentTarget() : nullptr;
	UEnvQueryItemType_Actor::SetContextHelper(ContextData, CurrentTarget);
}
