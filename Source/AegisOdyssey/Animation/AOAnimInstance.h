#pragma once

#include "Animation/AnimInstance.h"
#include "GameplayEffectTypes.h"
#include "AOAnimInstance.generated.h"

UCLASS(Config=Game)
class UAOAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
public:
	UAOAnimInstance(const FObjectInitializer& ObjectInitializer);

	virtual void InitializeWithAbilitySystem(UAbilitySystemComponent* ASC);

protected:
#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override;
protected:
	UPROPERTY(EditDefaultsOnly,Category = "GameplayTags")
	FGameplayTagBlueprintPropertyMap GameplayTagPropertyMap;

	UPROPERTY(EditDefaultsOnly,Category = "Character State Data")
	float GroundDistance = -1.0f;
};
