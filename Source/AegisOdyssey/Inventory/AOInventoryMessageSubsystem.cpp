#include "AegisOdyssey/Inventory/AOInventoryMessageSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOInventoryMessageSubsystem)

UAOInventoryMessageSubsystem* UAOInventoryMessageSubsystem::Get(const UObject* WorldContextObject)
{
	if (WorldContextObject == nullptr)
	{
		return nullptr;
	}

	if (UWorld* World = WorldContextObject->GetWorld())
	{
		return World->GetSubsystem<UAOInventoryMessageSubsystem>();
	}

	return nullptr;
}

void UAOInventoryMessageSubsystem::BroadcastInventoryAcquisition(const FAOInventoryAcquisitionMessage& Message)
{
	if (!Message.IsValid())
	{
		return;
	}

	OnInventoryAcquisitionMessage.Broadcast(Message);
}
