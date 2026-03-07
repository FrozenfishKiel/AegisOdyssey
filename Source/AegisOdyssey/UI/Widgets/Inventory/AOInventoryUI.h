// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "AegisOdyssey/UI/ViewModel/Inventory/MVVM_InventoryMenu.h"
#include "Blueprint/UserWidget.h"
#include "AOInventoryUI.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct FAOInventorySlot
{
	GENERATED_BODY()
	FAOInventorySlot() {}

	void operator = (const FAOInventoryEntry& other)
	{
		Instance = other.Instance;
		StackCount = other.StackCount;
		LastObservedCount = other.LastObservedCount;
		SlotOwnerComponent = other.SlotOwnerComponent;
	}
	
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UAOInventoryItemInstance> Instance = nullptr;

	UPROPERTY(BlueprintReadOnly)
	int32 StackCount = 0;
	
	UPROPERTY()
	int32 LastObservedCount = 0;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UAOInventoryComponent> SlotOwnerComponent;
};
UCLASS()
class AEGISODYSSEY_API UAOInventoryUI : public UCommonUserWidget
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintPure, BlueprintCallable)
	UMVVM_InventoryMenu* GetInventoryViewModel() const;
protected:
	template<typename ComponentClass>
	ComponentClass* FindTargetComponent() const;
};

template <typename ComponentClass>
ComponentClass* UAOInventoryUI::FindTargetComponent() const
{
	if (const ULocalPlayer* LocalPlayer = GetOwningLocalPlayer())
	{
		if (APlayerController* SourcePC = LocalPlayer->GetPlayerController(GetWorld()))
		{
			if (APawn* ControlledPawn = SourcePC->GetPawn())
			{
				if (ComponentClass* ViewModelPawn = ControlledPawn->FindComponentByClass<ComponentClass>())
				{
					return ViewModelPawn;
				}
			}
		}
	}
	return nullptr;
}
