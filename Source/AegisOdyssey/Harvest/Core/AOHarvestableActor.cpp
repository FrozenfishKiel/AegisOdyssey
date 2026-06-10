// Fill out your copyright notice in the Description page of Project Settings.

#include "AegisOdyssey/Harvest/Core/AOHarvestableActor.h"

#include "AegisOdyssey/Harvest/Core/AOHarvestableComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SceneComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOHarvestableActor)

AAOHarvestableActor::AAOHarvestableActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
	SetRootComponent(RootScene);

	HarvestableComponent = CreateDefaultSubobject<UAOHarvestableComponent>(TEXT("HarvestableComponent"));
}

void AAOHarvestableActor::BeginPlay()
{
	Super::BeginPlay();

	CapturePrimitiveCollisionSnapshot();
}

UAOHarvestableComponent* AAOHarvestableActor::GetHarvestableComponent_Implementation() const
{
	return HarvestableComponent;
}

void AAOHarvestableActor::HandleHarvestNodeDepleted_Implementation(const FAOHarvestLifecycleContext& LifecycleContext)
{
	ApplyDefaultHarvestDepletedState();
	OnHarvestNodeDepletedNative(LifecycleContext);
	ReceiveHarvestNodeDepleted(LifecycleContext);
}

void AAOHarvestableActor::HandleHarvestNodeRespawned_Implementation()
{
	ApplyDefaultHarvestRespawnedState();
	OnHarvestNodeRespawnedNative();
	ReceiveHarvestNodeRespawned();
}

void AAOHarvestableActor::ApplyDefaultHarvestDepletedState()
{
	SetHarvestPrimitivesCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AAOHarvestableActor::ApplyDefaultHarvestRespawnedState()
{
	RestorePrimitiveCollisionSnapshot();
}

void AAOHarvestableActor::OnHarvestNodeDepletedNative(const FAOHarvestLifecycleContext& LifecycleContext)
{
	(void)LifecycleContext;
}

void AAOHarvestableActor::OnHarvestNodeRespawnedNative()
{
}

void AAOHarvestableActor::SetHarvestPrimitivesCollisionEnabled(ECollisionEnabled::Type NewCollisionEnabled)
{
	TInlineComponentArray<UPrimitiveComponent*> PrimitiveComponents(this);
	for (UPrimitiveComponent* PrimitiveComponent : PrimitiveComponents)
	{
		if (PrimitiveComponent == nullptr)
		{
			continue;
		}

		PrimitiveComponent->SetCollisionEnabled(NewCollisionEnabled);
	}
}

void AAOHarvestableActor::CapturePrimitiveCollisionSnapshot()
{
	PrimitiveCollisionSnapshot.Reset();

	TInlineComponentArray<UPrimitiveComponent*> PrimitiveComponents(this);
	PrimitiveCollisionSnapshot.Reserve(PrimitiveComponents.Num());

	for (UPrimitiveComponent* PrimitiveComponent : PrimitiveComponents)
	{
		if (PrimitiveComponent == nullptr)
		{
			continue;
		}

		FAOHarvestPrimitiveCollisionSnapshot& Snapshot = PrimitiveCollisionSnapshot.AddDefaulted_GetRef();
		Snapshot.PrimitiveComponent = PrimitiveComponent;
		Snapshot.CollisionEnabled = PrimitiveComponent->GetCollisionEnabled();
	}
}

void AAOHarvestableActor::RestorePrimitiveCollisionSnapshot()
{
	for (const FAOHarvestPrimitiveCollisionSnapshot& Snapshot : PrimitiveCollisionSnapshot)
	{
		if (Snapshot.PrimitiveComponent == nullptr)
		{
			continue;
		}

		Snapshot.PrimitiveComponent->SetCollisionEnabled(Snapshot.CollisionEnabled);
	}
}
