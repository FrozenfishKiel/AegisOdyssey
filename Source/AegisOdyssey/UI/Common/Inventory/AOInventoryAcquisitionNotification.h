#pragma once

#include "CoreMinimal.h"
#include "Styling/SlateBrush.h"
#include "AegisOdyssey/Inventory/AOInventoryAcquisitionMessage.h"
#include "AegisOdyssey/Inventory/AOInventoryItemDefinition.h"
#include "AegisOdyssey/Inventory/Fragments/AOFragment_InventoryIcon.h"
#include "AOInventoryAcquisitionNotification.generated.h"

// HUD / 库存提示层消费的轻量物品获取通知。
// 它只负责把统一库存消息整理成 UI 可直接显示的字段，不承担 ViewModel 职责。
USTRUCT(BlueprintType)
struct FAOInventoryAcquisitionNotification
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	int32 SequenceId = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	float EventWorldTimeSeconds = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<AActor> Receiver = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	TSubclassOf<UAOInventoryItemDefinition> ItemDefinitionClass = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	FText DisplayName;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	FSlateBrush IconBrush;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	bool bHasValidIcon = false;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	int32 Count = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	bool bIsLocalRelevant = false;

	void ApplyInventoryAcquisition(
		const FAOInventoryAcquisitionMessage& Message,
		int32 InSequenceId,
		float InEventWorldTimeSeconds,
		const AActor* LocalPlayerActor = nullptr)
	{
		SequenceId = InSequenceId;
		EventWorldTimeSeconds = InEventWorldTimeSeconds;
		Receiver = Message.Receiver;
		ItemDefinitionClass = Message.ItemDefinitionClass;
		Count = Message.Count;

		ResolveDisplayData();
		bIsLocalRelevant = IsSameActorOrOwnedByActor(Receiver.Get(), LocalPlayerActor);
	}

private:
	static bool IsSameActorOrOwnedByActor(const AActor* CandidateActor, const AActor* ExpectedActor)
	{
		if (CandidateActor == nullptr || ExpectedActor == nullptr)
		{
			return false;
		}

		for (const AActor* CurrentActor = CandidateActor; CurrentActor != nullptr; CurrentActor = CurrentActor->GetOwner())
		{
			if (CurrentActor == ExpectedActor)
			{
				return true;
			}
		}

		return false;
	}

	void ResolveDisplayData()
	{
		DisplayName = FText::GetEmpty();
		IconBrush = FSlateBrush();
		bHasValidIcon = false;

		const UAOInventoryItemDefinition* ItemDefinition = ItemDefinitionClass != nullptr
			? GetDefault<UAOInventoryItemDefinition>(ItemDefinitionClass)
			: nullptr;
		if (ItemDefinition == nullptr)
		{
			return;
		}

		DisplayName = FText::FromName(ItemDefinition->DisplayName);

		if (const UAOFragment_InventoryIcon* InventoryIcon = ItemDefinition->FindFragmentByClass<UAOFragment_InventoryIcon>())
		{
			IconBrush = InventoryIcon->Brush;
			bHasValidIcon = InventoryIcon->Brush.GetResourceObject() != nullptr;
		}
	}
};
