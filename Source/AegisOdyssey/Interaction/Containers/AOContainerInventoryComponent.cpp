// Fill out your copyright notice in the Description page of Project Settings.

#include "AOContainerInventoryComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOContainerInventoryComponent)

UAOContainerInventoryComponent::UAOContainerInventoryComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsReplicatedByDefault(true);
}

void UAOContainerInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetOwner() && GetOwner()->HasAuthority())
	{
		InitializeOrRefreshInventorySlots();
	}
}

void UAOContainerInventoryComponent::InitializeOrRefreshInventorySlots()
{
	Super::InitializeOrRefreshInventorySlots();

	if (InventoryList.Entries.Num() < NumSlots)
	{
		InventoryList.Entries.Reserve(NumSlots);
		for (int32 i = InventoryList.Entries.Num(); i < NumSlots; ++i)
		{
			FAOInventoryEntry Entry(this);
			InventoryList.Entries.Emplace(Entry);
		}

		InventoryList.MarkArrayDirty();
	}
}
