#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "AT_WaitMovementInput.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMovementInputDetectedDelegate);

UCLASS()
class AEGISODYSSEY_API UAT_WaitMovementInput : public UAbilityTask
{
	GENERATED_BODY()

public:
	UAT_WaitMovementInput(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UPROPERTY(BlueprintAssignable)
	FMovementInputDetectedDelegate OnMovementInputDetected;

	virtual void Activate() override;
	virtual void TickTask(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
	static UAT_WaitMovementInput* WaitMovementInput(UGameplayAbility* OwningAbility);

private:
	bool HasMovementInput() const;
};
