#include "AOCombatMessageSubsystem.h"

#include "GameFramework/Pawn.h"
#include "AegisOdyssey/Player/AOPlayerController.h"
#include "AegisOdyssey/Player/AOPlayerState.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOCombatMessageSubsystem)

namespace AOCombatMessageSubsystemPrivate
{
	static AAOPlayerController* ResolveRelevantPlayerController(const AActor* Actor)
	{
		if (Actor == nullptr)
		{
			return nullptr;
		}

		if (const APawn* Pawn = Cast<APawn>(Actor))
		{
			return Cast<AAOPlayerController>(Pawn->GetController());
		}

		if (const AAOPlayerState* PlayerState = Cast<AAOPlayerState>(Actor))
		{
			return PlayerState->GetAOPlayerController();
		}

		for (const AActor* CurrentActor = Actor; CurrentActor != nullptr; CurrentActor = CurrentActor->GetOwner())
		{
			if (const APawn* OwnerPawn = Cast<APawn>(CurrentActor))
			{
				return Cast<AAOPlayerController>(OwnerPawn->GetController());
			}
		}

		return nullptr;
	}
}

UAOCombatMessageSubsystem* UAOCombatMessageSubsystem::Get(const UObject* WorldContextObject)
{
	if (WorldContextObject == nullptr)
	{
		return nullptr;
	}

	if (UWorld* World = WorldContextObject->GetWorld())
	{
		return World->GetSubsystem<UAOCombatMessageSubsystem>();
	}

	return nullptr;
}

void UAOCombatMessageSubsystem::BroadcastCombatResult(const FAOCombatResultMessage& Message)
{
	// 广播侧不对消息做任何二次改写。
	// 到这里的数据已经是战斗系统最终结论，订阅者只负责消费。
	BroadcastCombatResultLocal(Message);

	UWorld* World = GetWorld();
	if (World == nullptr || World->GetNetMode() == NM_Client)
	{
		return;
	}

	TArray<AAOPlayerController*, TInlineAllocator<3>> RelevantControllers;
	const auto AddRelevantController = [&RelevantControllers](AAOPlayerController* PlayerController)
	{
		if (PlayerController != nullptr)
		{
			RelevantControllers.AddUnique(PlayerController);
		}
	};

	AddRelevantController(AOCombatMessageSubsystemPrivate::ResolveRelevantPlayerController(Message.Instigator.Get()));
	AddRelevantController(AOCombatMessageSubsystemPrivate::ResolveRelevantPlayerController(Message.Target.Get()));
	AddRelevantController(AOCombatMessageSubsystemPrivate::ResolveRelevantPlayerController(Message.EffectCauser.Get()));

	for (AAOPlayerController* PlayerController : RelevantControllers)
	{
		PlayerController->ClientBroadcastCombatResultMessage(Message);
	}
}

void UAOCombatMessageSubsystem::BroadcastCombatResultLocal(const FAOCombatResultMessage& Message)
{
	// 只在当前世界做一次消费广播，不再承担网络转发职责。
	OnCombatResultMessage.Broadcast(Message);
}
