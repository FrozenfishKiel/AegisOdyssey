#include "AT_WaitMovementInput.h"
#include "AbilitySystemComponent.h"
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

	// 这里不能只看“已经被移动组件消费过”的输入，
	// 因为像火山喷发这种施法动画阶段，角色可能暂时走不起来，
	// 但玩家其实已经按下了移动。我们要捕捉的是“玩家想移动”的意图。
	const FVector PendingInputVector = Pawn->GetPendingMovementInputVector();
	if (PendingInputVector.SizeSquared() > FMath::Square(0.1f))
	{
		return true;
	}

	const FVector LastInputVector = Pawn->GetLastMovementInputVector();
	return LastInputVector.SizeSquared() > FMath::Square(0.1f);
}
