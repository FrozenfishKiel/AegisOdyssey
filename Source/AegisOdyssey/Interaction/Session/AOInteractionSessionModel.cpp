// Fill out your copyright notice in the Description page of Project Settings.

#include "AOInteractionSessionModel.h"

#include "AegisOdyssey/Interaction/AOInteractionSessionComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOInteractionSessionModel)

void UAOInteractionSessionModel::ActivateSession(UAOInteractionSessionComponent* InOwnerSessionComponent)
{
	OwnerSessionComponent = InOwnerSessionComponent;
}

void UAOInteractionSessionModel::DeactivateSession()
{
	OwnerSessionComponent = nullptr;
	InteractableActor = nullptr;
}
