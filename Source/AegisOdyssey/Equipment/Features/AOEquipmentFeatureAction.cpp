#include "AegisOdyssey/Equipment/Features/AOEquipmentFeatureAction.h"

#include "GameFramework/Actor.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOEquipmentFeatureAction)

bool UAOEquipmentFeatureAction::ShouldApplyToActor(const AActor* TargetActor, const bool bClientComponent, const bool bServerComponent) const
{
	if (TargetActor == nullptr)
	{
		return false;
	}

	const UWorld* World = TargetActor->GetWorld();
	if (World == nullptr)
	{
		return false;
	}

	const ENetMode NetMode = World->GetNetMode();
	const bool bIsServer = NetMode != NM_Client;
	const bool bIsClient = NetMode != NM_DedicatedServer;

	return (bIsServer && bServerComponent) || (bIsClient && bClientComponent);
}
