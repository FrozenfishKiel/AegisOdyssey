// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AegisOdyssey/SkillSystem/Components/AOSkillComponent.h"
#include "AegisOdyssey/UI/Widgets/Inventory/AOInventoryUI.h"
#include "AOSkillBarUI.generated.h"

class UAOInventoryComponent;
class UAOSkillComponent;
class UAOSkillSlotInventoryComponent;
class UAOSkillSlotUI;
class UEnhancedInputLocalPlayerSubsystem;
class UMVVM_HUD;
class UPanelWidget;

UCLASS(Abstract, Blueprintable)
class AEGISODYSSEY_API UAOSkillBarUI : public UAOInventoryUI
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION(BlueprintCallable, Category = "AO|Inventory UI")
	void SetDisplayContext(const FAOInventoryDisplayContext& InDisplayContext);

	UFUNCTION(BlueprintPure, Category = "AO|Skill UI")
	UMVVM_HUD* GetMainHUDViewModel() const;

	UFUNCTION(BlueprintCallable, Category = "AO|Skill UI")
	void RequestRefreshFromSkillSystem();

	UFUNCTION(BlueprintCallable, Category = "AO|Skill UI")
	void RefreshSkillBar();

	UFUNCTION(BlueprintPure, Category = "AO|Skill UI")
	TArray<FAOSkillSlotViewData> GetSkillSlotViewDataList() const;

	UFUNCTION(BlueprintPure, Category = "AO|Skill UI")
	bool GetSkillSlotViewData(int32 SlotIndex, FAOSkillSlotViewData& OutViewData) const;

protected:
	virtual void HandleSkillBarRebuilt();
	virtual FText BuildSlotInputDisplayText(int32 SlotIndex, int32 TotalSlotCount) const;

protected:
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UPanelWidget> SkillSlotContainer = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "AO|Skill UI")
	TSubclassOf<UAOSkillSlotUI> SkillSlotClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AO|Skill UI|DragDrop")
	TObjectPtr<UAOInventoryComponent> SourceContainer = nullptr;

private:
	UAOSkillComponent* GetObservedSkillComponent() const;
	UAOSkillSlotInventoryComponent* GetObservedSkillSlotInventory() const;
	void BindSkillObservationDelegate();
	void UnbindSkillObservationDelegate();
	void HandleSkillObservationDataChanged();

	void BindInputMappingRefreshDelegate();
	void UnbindInputMappingRefreshDelegate();

	UFUNCTION()
	void HandleControlMappingsRebuilt();

	UAOInventoryComponent* ResolveSkillSlotContainer() const;

private:
	UPROPERTY(Transient)
	FAOInventoryDisplayContext DisplayContext;

	TWeakObjectPtr<UAOSkillComponent> BoundSkillComponent;
	FDelegateHandle SkillObservationDataChangedHandle;
	TWeakObjectPtr<UEnhancedInputLocalPlayerSubsystem> BoundEnhancedInputSubsystem;
};
