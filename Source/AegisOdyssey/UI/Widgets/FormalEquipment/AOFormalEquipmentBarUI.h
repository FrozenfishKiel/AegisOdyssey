#pragma once

#include "CoreMinimal.h"
#include "AegisOdyssey/UI/Widgets/Inventory/AOInventoryUI.h"
#include "AOFormalEquipmentBarUI.generated.h"

class UAOFormalEquipmentManagerComponent;
class UAOFormalEquipmentSlotInventoryComponent;
class UAOFormalEquipmentSlotUI;
class UMVVM_InventoryMenu;
class UPanelWidget;

UCLASS(Abstract, Blueprintable)
class AEGISODYSSEY_API UAOFormalEquipmentBarUI : public UAOInventoryUI
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual UMVVM_InventoryMenu* GetInventoryViewModel() const override;

	UFUNCTION(BlueprintCallable, Category = "AO|Inventory UI")
	void SetDisplayContext(const FAOInventoryDisplayContext& InDisplayContext);

	UFUNCTION(BlueprintCallable, Category = "AO|FormalEquipment UI")
	void RefreshFormalEquipmentBar();

protected:
	virtual void HandleFormalEquipmentBarRebuilt();

	UFUNCTION(BlueprintPure, Category = "AO|FormalEquipment UI")
	UAOFormalEquipmentSlotInventoryComponent* GetFormalEquipmentSlotInventory() const;

	UFUNCTION(BlueprintPure, Category = "AO|FormalEquipment UI")
	UAOFormalEquipmentManagerComponent* GetFormalEquipmentManager() const;

protected:
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UPanelWidget> FormalEquipmentSlotContainer = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "AO|FormalEquipment UI")
	TSubclassOf<UAOFormalEquipmentSlotUI> FormalEquipmentSlotClass;

private:
	void HandleFormalEquipmentListChanged();

private:
	UPROPERTY(Transient)
	FAOInventoryDisplayContext DisplayContext;

	FDelegateHandle RefreshFormalEquipmentBarDelegateHandle;
};
