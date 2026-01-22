#include "AOAnimInstance.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "AegisOdyssey/Character/AOCharacter.h"
#include "AegisOdyssey/Character/AOCharacterMovementComponent.h"
#include "Misc/DataValidation.h"

UAOAnimInstance::UAOAnimInstance(const FObjectInitializer& ObjectInitializer)
{
}

void UAOAnimInstance::InitializeWithAbilitySystem(UAbilitySystemComponent* ASC)
{
	check(ASC);

	GameplayTagPropertyMap.Initialize(this,ASC);
}

EDataValidationResult UAOAnimInstance::IsDataValid(class FDataValidationContext& Context) const
{
	Super::IsDataValid(Context);

	GameplayTagPropertyMap.IsDataValid(this,Context);

	return(Context.GetNumErrors()>0) ? EDataValidationResult::Invalid : EDataValidationResult::Valid;
}

void UAOAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	if (AActor* OwningActor = GetOwningActor())
	{
		if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(OwningActor))
		{
			InitializeWithAbilitySystem(ASC);
		}
	}
}

void UAOAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	const AAOCharacter* Character = Cast<AAOCharacter>(GetOwningActor());
	if (!Character) return;

	UAOCharacterMovementComponent* CharMoveComp = CastChecked<UAOCharacterMovementComponent>(Character->GetCharacterMovement());
}

void UAOAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeThreadSafeUpdateAnimation(DeltaSeconds);
	
}
