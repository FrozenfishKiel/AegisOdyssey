// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AegisOdyssey/Inventory/AOInventoryComponent.h"
#include "AegisOdyssey/Inventory/AOInventoryManagerComponent.h"
#include "AOWeaponManagerComponent.generated.h"


/**
 * 
 */
class UAOEquipmentInstance;
UCLASS(BlueprintType)
class AEGISODYSSEY_API UAOWeaponManagerComponent : public UAOInventoryManagerComponent
{
	GENERATED_BODY()

public:
	UAOWeaponManagerComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	virtual void OnItemUse(FAOInventoryEntry& TargetItem);
	virtual void OnItemUnUse(FAOInventoryEntry& TargetItem);
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	void EquipItem(FAOInventoryEntry& InEquipment);
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	void UnequipItem(FAOInventoryEntry& InItem);

	UFUNCTION(BlueprintCallable)
	UAOEquipmentInstance* GetCurrentWeaponInstance() const {return CurrentWeaponInstance;}
	
	UFUNCTION()
	void OnRep_CurrentWeaponInstance(UAOEquipmentInstance* LastWeaponInstance);
protected:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void ChangedItemOnSlot(const int32 ChangedIndex, int32 CurrentIndex, TArray<FAOInventoryEntry>* Slots) override;
private:
	UPROPERTY(ReplicatedUsing = OnRep_CurrentWeaponInstance)
	TObjectPtr<UAOEquipmentInstance> CurrentWeaponInstance;
	UPROPERTY(Replicated)
	bool bIsUse = false;
	UPROPERTY(ReplicatedUsing = OnRep_Weapon)
	FAOInventoryEntry Weapon;
	UFUNCTION()
	void OnRep_Weapon();
};
