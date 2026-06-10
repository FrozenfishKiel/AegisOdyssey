// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AegisOdyssey/SkillSystem/Components/AOSkillComponent.h"
#include "AegisOdyssey/UI/Widgets/Inventory/AOInventoryUI.h"
#include "AOSkillSlotUI.generated.h"

class UAOInventoryComponent;
class UAOInventoryItemInstance;
class UImage;
class UInputAction;
class UAOSkillComponent;
class UTextBlock;
class UMVVM_HUD;

UCLASS(Abstract, Blueprintable)
class AEGISODYSSEY_API UAOSkillSlotUI : public UAOInventoryUI
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintPure, Category = "AO|Skill UI")
	UMVVM_HUD* GetMainHUDViewModel() const;

	UFUNCTION(BlueprintCallable, Category = "AO|Skill UI")
	void SetSkillSlotData(const FAOSkillSlotViewData& InSkillSlotViewData);

	UFUNCTION(BlueprintCallable, Category = "AO|Skill UI")
	void SetObservedSlotIndex(int32 InObservedSlotIndex);

	UFUNCTION(BlueprintCallable, Category = "AO|Skill UI|DragDrop")
	void SetSourceContainer(UAOInventoryComponent* InSourceContainer);

	UFUNCTION(BlueprintCallable, Category = "AO|Skill UI")
	void SetInputDisplayText(const FText& InInputDisplayText);

	UFUNCTION(BlueprintCallable, Category = "AO|Skill UI")
	void SetOwningSkillComponent(UAOSkillComponent* InSkillComponent);

	UFUNCTION(BlueprintPure, Category = "AO|Skill UI|Input")
	const UInputAction* GetSkillSlotInputAction() const;

	UFUNCTION(BlueprintPure, Category = "AO|Skill UI|Input")
	FName GetSkillSlotInputMappingName() const;

	UFUNCTION(BlueprintPure, Category = "AO|Skill UI|Input")
	FText GetResolvedInputDisplayText() const;

	UFUNCTION(BlueprintPure, Category = "AO|Skill UI")
	const FAOSkillSlotViewData& GetSkillSlotViewData() const { return SkillSlotViewData; }

	UFUNCTION(BlueprintPure, Category = "AO|Skill UI")
	int32 GetObservedSlotIndex() const { return ObservedSlotIndex; }

	UFUNCTION(BlueprintPure, Category = "AO|Skill UI")
	int32 GetIndex() const { return Index; }

	void SetIndex(int32 InIndex)
	{
		Index = InIndex;
		ObservedSlotIndex = InIndex;
	}

	UFUNCTION(BlueprintPure, Category = "AO|Skill UI")
	UAOSkillComponent* GetOwningSkillComponent() const;

	UFUNCTION(BlueprintPure, Category = "AO|Skill UI|DragDrop")
	UAOInventoryComponent* GetSourceContainer() const { return SourceContainer; }

	UFUNCTION(BlueprintPure, Category = "AO|Skill UI|DragDrop")
	bool CanAcceptDraggedSourceSlotForThisSkillSlot(UAOInventoryComponent* InSourceContainer, int32 InSourceSlotIndex) const;

	UFUNCTION(BlueprintPure, Category = "AO|Skill UI|DragDrop")
	bool CanAcceptDraggedItemForThisSkillSlot(const UAOInventoryItemInstance* SourceItemInstance) const;

	UFUNCTION(BlueprintCallable, Category = "AO|Skill UI|DragDrop")
	bool RequestEquipDraggedSourceSlotToThisSkillSlot(UAOInventoryComponent* InSourceContainer, int32 InSourceSlotIndex);

	UFUNCTION(BlueprintCallable, Category = "AO|Skill UI|DragDrop")
	bool RequestEquipDraggedItemToThisSkillSlot(UAOInventoryItemInstance* SourceItemInstance);

	UFUNCTION(BlueprintCallable, Category = "AO|Skill UI")
	void RefreshSkillSlotDisplay();

	UFUNCTION(BlueprintPure, Category = "AO|Skill UI|Cooldown")
	bool GetSkillSlotCooldownState(float& OutTimeRemaining, float& OutTotalDuration) const;

	UFUNCTION(BlueprintPure, Category = "AO|Skill UI|Cooldown")
	float GetSkillSlotCooldownRemaining() const;

	UFUNCTION(BlueprintPure, Category = "AO|Skill UI|Cooldown")
	bool IsSkillSlotOnCooldown() const;

protected:
	virtual bool ResolveInventoryItemContextMenuRequest(
		UAOInventoryComponent*& OutSourceInventory,
		int32& OutSourceSlotIndex,
		UAOInventoryItemInstance*& OutItemInstance) const override;

	virtual const UAOInventoryItemDefinition* ResolveHoverTooltipItemDefinition() const override;
	virtual void HandleSkillSlotDataUpdated(bool bHasValidSkill);
	bool ResolveCurrentSkillSlotViewData(FAOSkillSlotViewData& OutViewData) const;

protected:
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	TObjectPtr<UImage> SkillIcon = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> InputIndex = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> SkillNameText = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "AO|Skill UI")
	int32 Index = INDEX_NONE;

	UPROPERTY(BlueprintReadOnly, Category = "AO|Skill UI")
	int32 ObservedSlotIndex = INDEX_NONE;

	UPROPERTY(BlueprintReadOnly, Category = "AO|Skill UI")
	FAOSkillSlotViewData SkillSlotViewData;

	UPROPERTY(BlueprintReadOnly, Category = "AO|Skill UI|DragDrop")
	TObjectPtr<UAOInventoryComponent> SourceContainer = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "AO|Skill UI")
	FText InputDisplayText;

	UPROPERTY(BlueprintReadOnly, Category = "AO|Skill UI")
	TObjectPtr<UAOSkillComponent> OwningSkillComponent = nullptr;
};
