#include "AegisOdyssey/Equipment/Features/AOEquipmentFeatureAction_AddComponents.h"

#include "AegisOdyssey/Equipment/AOEquipmentInstance.h"
#include "AegisOdyssey/StateTree/AOStateTreeComponentBase.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOEquipmentFeatureAction_AddComponents)

void UAOEquipmentFeatureAction_AddComponents::ActivateForEquipment(UAOEquipmentInstance* EquipmentInstance, AActor* TargetActor, FAOEquipmentFeatureActionRuntimeData& RuntimeData) const
{
	if (EquipmentInstance == nullptr || TargetActor == nullptr)
	{
		return;
	}

	for (const FAOEquipmentFeatureComponentEntry& Entry : ComponentList)
	{
		if (!ShouldApplyToActor(TargetActor, Entry.bClientComponent, Entry.bServerComponent))
		{
			continue;
		}

		UClass* ComponentClass = Entry.ComponentClass.LoadSynchronous();
		if (ComponentClass == nullptr)
		{
			continue;
		}

		const EGameFrameworkAddComponentFlags AdditionFlags = static_cast<EGameFrameworkAddComponentFlags>(Entry.AdditionFlags);
		if (!CanAddComponent(TargetActor, ComponentClass, AdditionFlags))
		{
			continue;
		}

		if (UActorComponent* AddedComponent = TargetActor->AddComponentByClass(ComponentClass, false, FTransform::Identity, false))
		{
			RuntimeData.AddedComponents.Add(AddedComponent);

			if (TargetActor->HasActorBegunPlay())
			{
				if (UAOStateTreeComponentBase* AddedStateTreeComponent = Cast<UAOStateTreeComponentBase>(AddedComponent))
				{
					if (!AddedStateTreeComponent->IsRunning() && AddedStateTreeComponent->GetStateTreeAsset() != nullptr)
					{
						AddedStateTreeComponent->RestartLogic();
					}
				}
			}
		}
	}
}

void UAOEquipmentFeatureAction_AddComponents::DeactivateForEquipment(UAOEquipmentInstance* EquipmentInstance, AActor* TargetActor, FAOEquipmentFeatureActionRuntimeData& RuntimeData) const
{
	if (EquipmentInstance == nullptr || TargetActor == nullptr)
	{
		RuntimeData.Reset();
		return;
	}

	for (UActorComponent* AddedComponent : RuntimeData.AddedComponents)
	{
		if (AddedComponent != nullptr)
		{
			AddedComponent->DestroyComponent();
		}
	}

	RuntimeData.Reset();
}

bool UAOEquipmentFeatureAction_AddComponents::CanAddComponent(AActor* TargetActor, UClass* ComponentClass, EGameFrameworkAddComponentFlags AdditionFlags) const
{
	if (TargetActor == nullptr || ComponentClass == nullptr)
	{
		return false;
	}

	TInlineComponentArray<UActorComponent*> ExistingComponents(TargetActor);
	for (UActorComponent* ExistingComponent : ExistingComponents)
	{
		if (ExistingComponent == nullptr)
		{
			continue;
		}

		if (EnumHasAnyFlags(AdditionFlags, EGameFrameworkAddComponentFlags::AddUnique) && ExistingComponent->GetClass() == ComponentClass)
		{
			return false;
		}

		if (EnumHasAnyFlags(AdditionFlags, EGameFrameworkAddComponentFlags::AddIfNotChild) && ExistingComponent->IsA(ComponentClass))
		{
			return false;
		}
	}

	return true;
}
