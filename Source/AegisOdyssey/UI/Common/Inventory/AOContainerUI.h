// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AegisOdyssey/UI/Widgets/Inventory/AOInventoryUI.h"
#include "AOContainerUI.generated.h"

class UAOContainerInteractionSessionModel;
class UAOContainerSlot;
class UAOInteractionSessionComponent;
class UAOInteractionSessionModel;
class UMVVM_InventoryMenu;
class UWrapBox;

UCLASS()
class AEGISODYSSEY_API UAOContainerUI : public UAOInventoryUI
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual UMVVM_InventoryMenu* GetInventoryViewModel() const override;

	UFUNCTION(BlueprintPure, Category = "AO|Interaction")
	UAOContainerInteractionSessionModel* GetContainerSessionModel() const;

	UFUNCTION(BlueprintPure, Category = "AO|Interaction")
	bool HasContainerSession() const;

protected:
	void BindInteractionSessionComponent();
	void RebindContainerSessionModel(UAOContainerInteractionSessionModel* NewContainerSessionModel);
	void HandleOwningSessionChanged(UAOInteractionSessionModel* NewSessionModel);
	void HandleBoundContainerDataChanged();

	UFUNCTION(BlueprintCallable, Category = "AO|Interaction")
	void RefreshContainerSlots();

protected:
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UWrapBox> ContainerSlotBox = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AO|Interaction")
	TSubclassOf<UAOContainerSlot> ContainerSlotClass;

	FDelegateHandle SessionChangedDelegateHandle;
	FDelegateHandle ContainerDataChangedDelegateHandle;

	UPROPERTY(Transient)
	TObjectPtr<UAOInteractionSessionComponent> BoundSessionComponent = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UAOContainerInteractionSessionModel> BoundContainerSessionModel = nullptr;
};
