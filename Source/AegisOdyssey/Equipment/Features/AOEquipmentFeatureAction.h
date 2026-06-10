#pragma once

#include "CoreMinimal.h"
#include "GameFeatureAction.h"
#include "AOEquipmentFeatureAction.generated.h"

class AActor;
class UActorComponent;
class UAOEquipmentInstance;

USTRUCT()
struct FAOEquipmentFeatureActionRuntimeData
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	TArray<TObjectPtr<UActorComponent>> AddedComponents;

	void Reset()
	{
		AddedComponents.Reset();
	}
};

UCLASS(Abstract, BlueprintType, Blueprintable, DefaultToInstanced, EditInlineNew)
class AEGISODYSSEY_API UAOEquipmentFeatureAction : public UGameFeatureAction
{
	GENERATED_BODY()

public:
	virtual void ActivateForEquipment(UAOEquipmentInstance* EquipmentInstance, AActor* TargetActor, FAOEquipmentFeatureActionRuntimeData& RuntimeData) const PURE_VIRTUAL(UAOEquipmentFeatureAction::ActivateForEquipment,);
	virtual void DeactivateForEquipment(UAOEquipmentInstance* EquipmentInstance, AActor* TargetActor, FAOEquipmentFeatureActionRuntimeData& RuntimeData) const PURE_VIRTUAL(UAOEquipmentFeatureAction::DeactivateForEquipment,);

protected:
	bool ShouldApplyToActor(const AActor* TargetActor, bool bClientComponent, bool bServerComponent) const;
};
