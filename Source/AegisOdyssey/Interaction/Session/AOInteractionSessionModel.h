// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "AOInteractionSessionModel.generated.h"

class UCommonActivatableWidget;
class UAOInteractionSessionComponent;

UCLASS(Abstract, BlueprintType)
class AEGISODYSSEY_API UAOInteractionSessionModel : public UObject
{
	GENERATED_BODY()

public:
	virtual void ActivateSession(UAOInteractionSessionComponent* InOwnerSessionComponent);
	virtual void DeactivateSession();

	virtual AActor* GetInteractableActor() const { return InteractableActor.Get(); }
	UAOInteractionSessionComponent* GetOwnerSessionComponent() const { return OwnerSessionComponent.Get(); }

	virtual TSubclassOf<UCommonActivatableWidget> GetSessionWidgetClass() const { return SessionWidgetClass; }
	void SetSessionWidgetClass(TSubclassOf<UCommonActivatableWidget> InSessionWidgetClass) { SessionWidgetClass = InSessionWidgetClass; }

protected:
	UPROPERTY(Transient)
	TWeakObjectPtr<UAOInteractionSessionComponent> OwnerSessionComponent;

	UPROPERTY(Transient)
	TWeakObjectPtr<AActor> InteractableActor;

	UPROPERTY(Transient)
	TSubclassOf<UCommonActivatableWidget> SessionWidgetClass;
};
