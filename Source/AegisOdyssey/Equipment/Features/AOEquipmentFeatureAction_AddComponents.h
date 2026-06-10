#pragma once

#include "CoreMinimal.h"
#include "Components/GameFrameworkComponentManager.h"
#include "AegisOdyssey/Equipment/Features/AOEquipmentFeatureAction.h"
#include "AOEquipmentFeatureAction_AddComponents.generated.h"

class UActorComponent;

USTRUCT(BlueprintType)
struct FAOEquipmentFeatureComponentEntry
{
	GENERATED_BODY()

	FAOEquipmentFeatureComponentEntry()
		: bClientComponent(true)
		, bServerComponent(true)
		, AdditionFlags(static_cast<uint8>(EGameFrameworkAddComponentFlags::None))
	{
	}

	UPROPERTY(EditAnywhere, Category = "Components")
	TSoftClassPtr<UActorComponent> ComponentClass;

	UPROPERTY(EditAnywhere, Category = "Components")
	uint8 bClientComponent : 1;

	UPROPERTY(EditAnywhere, Category = "Components")
	uint8 bServerComponent : 1;

	UPROPERTY(EditAnywhere, Category = "Components", meta = (Bitmask, BitmaskEnum = "/Script/ModularGameplay.EGameFrameworkAddComponentFlags"))
	uint8 AdditionFlags;
};

UCLASS(DisplayName = "AO Add Components")
class AEGISODYSSEY_API UAOEquipmentFeatureAction_AddComponents : public UAOEquipmentFeatureAction
{
	GENERATED_BODY()

public:
	virtual void ActivateForEquipment(UAOEquipmentInstance* EquipmentInstance, AActor* TargetActor, FAOEquipmentFeatureActionRuntimeData& RuntimeData) const override;
	virtual void DeactivateForEquipment(UAOEquipmentInstance* EquipmentInstance, AActor* TargetActor, FAOEquipmentFeatureActionRuntimeData& RuntimeData) const override;

	UPROPERTY(EditAnywhere, Category = "Components", meta = (TitleProperty = "{ComponentClass}"))
	TArray<FAOEquipmentFeatureComponentEntry> ComponentList;

private:
	bool CanAddComponent(AActor* TargetActor, UClass* ComponentClass, EGameFrameworkAddComponentFlags AdditionFlags) const;
};
