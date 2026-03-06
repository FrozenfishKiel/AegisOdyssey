#include "AT_WaitMovementInput.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Pawn.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(AT_WaitMovementInput)

UAT_WaitMovementInput::UAT_WaitMovementInput(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bTickingTask = true;
}

UAT_WaitMovementInput* UAT_WaitMovementInput::WaitMovementInput(UGameplayAbility* OwningAbility)
{
	UAT_WaitMovementInput* MyObj = NewAbilityTask<UAT_WaitMovementInput>(OwningAbility);
	return MyObj;
}

void UAT_WaitMovementInput::Activate()
{
	SetWaitingOnAvatar();
}

void UAT_WaitMovementInput::TickTask(float DeltaTime)
{
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		if (HasMovementInput())
		{
			OnMovementInputDetected.Broadcast();
		}
	}
}

bool UAT_WaitMovementInput::HasMovementInput() const
{
	if (!Ability || !Ability->GetAvatarActorFromActorInfo())
	{
		return false;
	}

	AActor* OwnerActor = Ability->GetAvatarActorFromActorInfo();
	APawn* Pawn = Cast<APawn>(OwnerActor);

	if (!Pawn)
	{
		return false;
	}

	UCharacterMovementComponent* MovementComp = Pawn->FindComponentByClass<UCharacterMovementComponent>();

	if (!MovementComp)
	{
		return false;
	}

	FVector LastInputVector = MovementComp->GetLastInputVector();
	float InputMagnitude = LastInputVector.Size();

	return InputMagnitude > 0.1f;
}
