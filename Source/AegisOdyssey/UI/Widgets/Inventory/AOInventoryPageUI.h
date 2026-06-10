#pragma once

#include "CoreMinimal.h"
#include "AegisOdyssey/UI/Widgets/Inventory/AOInventoryUI.h"
#include "AOInventoryPageUI.generated.h"

class UAOBackPackUI;
class UAOFormalEquipmentBarUI;
class UAOQuickBarUI;
class UAOSkillBarUI;

UCLASS()
class AEGISODYSSEY_API UAOInventoryPageUI : public UAOInventoryUI
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintCallable, Category = "AO|Inventory UI")
	virtual void RefreshInventoryPageContexts();

	UFUNCTION(BlueprintPure, Category = "AO|Inventory UI")
	bool HasTargetInventoryDisplayContext() const;

	UFUNCTION(BlueprintPure, Category = "AO|Inventory UI")
	FText GetTargetInventoryDisplayName() const;

	UFUNCTION(BlueprintPure, Category = "AO|Inventory UI")
	bool IsTargetInventoryOwnerDead() const;

protected:
private:
	void ApplyInventoryPageContexts();
	FAOInventoryDisplayContext BuildSelfInventoryDisplayContext() const;
	FAOInventoryDisplayContext BuildTargetInventoryDisplayContext() const;
	bool IsInventoryOwnerDead(const AActor* InventoryOwnerActor) const;

private:
	UPROPERTY(BlueprintReadWrite, Category = "AO|Inventory UI", meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	TObjectPtr<UAOBackPackUI> SelfBackPackPanel = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "AO|Inventory UI", meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	TObjectPtr<UAOQuickBarUI> SelfQuickBarPanel = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "AO|Inventory UI", meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	TObjectPtr<UAOFormalEquipmentBarUI> SelfFormalEquipmentPanel = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "AO|Inventory UI", meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	TObjectPtr<UAOSkillBarUI> SelfSkillPanel = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "AO|Inventory UI", meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	TObjectPtr<UAOBackPackUI> TargetBackPackPanel = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "AO|Inventory UI", meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	TObjectPtr<UAOQuickBarUI> TargetQuickBarPanel = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "AO|Inventory UI", meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	TObjectPtr<UAOFormalEquipmentBarUI> TargetFormalEquipmentPanel = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "AO|Inventory UI", meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	TObjectPtr<UAOSkillBarUI> TargetSkillPanel = nullptr;

	UPROPERTY(Transient)
	FAOInventoryDisplayContext SelfInventoryDisplayContext;

	UPROPERTY(Transient)
	FAOInventoryDisplayContext TargetInventoryDisplayContext;
};
