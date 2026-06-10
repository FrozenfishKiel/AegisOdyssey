// Fill out your copyright notice in the Description page of Project Settings.

#include "AOInteractionSessionWidget.h"

#include "AegisOdyssey/Interaction/AOInteractionSessionComponent.h"
#include "AegisOdyssey/Interaction/Session/AOInteractionSessionModel.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOInteractionSessionWidget)

void UAOInteractionSessionWidget::SetInteractionSessionModel(UAOInteractionSessionModel* NewSessionModel)
{
	InteractionSessionModel = NewSessionModel;
	HandleInteractionSessionChanged(NewSessionModel);
	OnInteractionSessionChanged.Broadcast(NewSessionModel);
}

void UAOInteractionSessionWidget::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();

	if (!InteractionSessionModel)
	{
		return;
	}

	if (APlayerController* OwningPlayerController = GetOwningPlayer())
	{
		if (UAOInteractionSessionComponent* SessionComponent =
			OwningPlayerController->FindComponentByClass<UAOInteractionSessionComponent>())
		{
			if (SessionComponent->GetCurrentSessionModel() == InteractionSessionModel)
			{
				SessionComponent->RequestCloseCurrentSession();
			}
		}
	}
}

void UAOInteractionSessionWidget::HandleInteractionSessionChanged(UAOInteractionSessionModel* NewSessionModel)
{
}
