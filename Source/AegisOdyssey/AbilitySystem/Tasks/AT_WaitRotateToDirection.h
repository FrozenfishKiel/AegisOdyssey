#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "AT_WaitRotateToDirection.generated.h"

UCLASS()
class AEGISODYSSEY_API UAT_WaitRotateToDirection : public UAbilityTask
{
	GENERATED_BODY()

public:
	UAT_WaitRotateToDirection(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void Activate() override;
	virtual void TickTask(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
	static UAT_WaitRotateToDirection* WaitRotateToDirection(UGameplayAbility* OwningAbility, FRotator TargetRotation, float InterpSpeed);

private:
	FRotator TargetRotation;
	float InterpSpeed;
};
