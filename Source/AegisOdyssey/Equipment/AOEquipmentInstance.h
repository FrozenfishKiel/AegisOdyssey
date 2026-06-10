// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AegisOdyssey/Equipment/Features/AOEquipmentFeatureAction.h"
#include "AegisOdyssey/Inventory/AOInventoryItemInstance.h"
#include "AOEquipmentInstance.generated.h"

struct FAOEquipmentSpawnedConfig;
class UAOInventoryItemDefinition;
class UAnimMontage;
struct FGameplayTag;

UCLASS(BlueprintType, Blueprintable, DefaultToInstanced)
class AEGISODYSSEY_API UAOEquipmentInstance : public UAOInventoryItemInstance
{
	GENERATED_BODY()

public:
	UAOEquipmentInstance(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual UWorld* GetWorld() const override final;

	UFUNCTION(BlueprintPure, Category = Equipment)
	UObject* GetInstigator() const { return Instigator; }

	void SetInstigator(UObject* InInstigator) { Instigator = InInstigator; }

	UFUNCTION(BlueprintPure, Category = Equipment)
	APawn* GetPawn() const;

	UFUNCTION(BlueprintPure, Category = Equipment, meta = (DeterminesOutputType = PawnType))
	APawn* GetTypedPawn(TSubclassOf<APawn> PawnType) const;

	UFUNCTION(BlueprintPure, Category = Equipment)
	inline TArray<AActor*> GetSpawnedActors() const { return SpawnedActors; }

	virtual void SpawnEquipmentActors(const TArray<FAOEquipmentSpawnedConfig> SpawnConfigList);
	virtual void DestoryEquipmentActors();

	virtual void OnEquiped();
	virtual void OnUnEquiped();

	virtual UAOInventoryManagerComponent* FindTargetInventoryManager() const override;
	virtual bool CanUseFromInventory(const FAOInventoryEntry& InventoryEntry, APawn* UserPawn) const override;
	virtual bool TryUseFromInventory(FAOInventoryEntry& InventoryEntry, APawn* UserPawn, int32& OutConsumeCount) override;

private:
	UFUNCTION()
	void OnRep_Instigator();

	UFUNCTION()
	void OnRep_SpawnedActors();

protected:
	virtual void SetItemDef(TSubclassOf<UAOInventoryItemDefinition> InDef) override;
	void ApplyEquipmentFeatureActions();
	void RemoveEquipmentFeatureActions();
	void PlayEquipAnimation();
	void PlayUnEquipAnimation();
	void TryPlayEquipmentAnimation(UAnimMontage* MontageToPlay, const FGameplayTag& AbilityInputTag) const;

protected:
	UPROPERTY(ReplicatedUsing = OnRep_Instigator)
	TObjectPtr<UObject> Instigator;

	UPROPERTY(ReplicatedUsing = OnRep_SpawnedActors)
	TArray<TObjectPtr<AActor>> SpawnedActors;

	UPROPERTY(Transient)
	TArray<FAOEquipmentFeatureActionRuntimeData> ActiveFeatureActionRuntimeData;

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = Equipment, meta = (DisplayName = "OnEquipped"))
	void K2_OnEquipped();

	UFUNCTION(BlueprintImplementableEvent, Category = Equipment, meta = (DisplayName = "OnUnEquipped"))
	void K2_OnUnEquipped();
};
