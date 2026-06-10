#pragma once

#include "CoreMinimal.h"
#include "AegisOdyssey/UI/AOActivatableWidget.h"
#include "AOInteractionSessionWidget.generated.h"

class UAOInteractionSessionModel;

UCLASS(Abstract, Blueprintable)
class AEGISODYSSEY_API UAOInteractionSessionWidget : public UAOActivatableWidget
{
	GENERATED_BODY()

public:
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnInteractionSessionChanged, UAOInteractionSessionModel*);

	UFUNCTION(BlueprintCallable, Category = "AO|Interaction")
	virtual void SetInteractionSessionModel(UAOInteractionSessionModel* NewSessionModel);

	UFUNCTION(BlueprintPure, Category = "AO|Interaction")
	UAOInteractionSessionModel* GetInteractionSessionModel() const { return InteractionSessionModel; }

	FOnInteractionSessionChanged& GetOnInteractionSessionChanged() { return OnInteractionSessionChanged; }

protected:
	virtual void NativeOnDeactivated() override;
	virtual void HandleInteractionSessionChanged(UAOInteractionSessionModel* NewSessionModel);

protected:
	UPROPERTY(Transient, BlueprintReadOnly, Category = "AO|Interaction")
	TObjectPtr<UAOInteractionSessionModel> InteractionSessionModel = nullptr;

	FOnInteractionSessionChanged OnInteractionSessionChanged;
};
