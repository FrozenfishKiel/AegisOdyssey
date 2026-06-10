// Fill out your copyright notice in the Description page of Project Settings.

#include "AOWeaponInstance.h"

#include "AegisOdyssey/Combat/Effects/AOAttackEffectProfile.h"
#include "AegisOdyssey/Equipment/Weapons/AOWeaponDefinition.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOWeaponInstance)

UAOWeaponInstance::UAOWeaponInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UAOWeaponInstance::SetItemDef(TSubclassOf<UAOInventoryItemDefinition> InDef)
{
	Super::SetItemDef(InDef);
}

const UAOAttackEffectProfile* UAOWeaponInstance::GetEffectiveAttackEffectProfile() const
{
	const UAOWeaponDefinition* WeaponDefinition = Cast<UAOWeaponDefinition>(GetItemCDO());
	return WeaponDefinition != nullptr ? WeaponDefinition->GetDefaultAttackEffectProfile() : nullptr;
}

void UAOWeaponInstance::GetSpawnedWeaponActors(TArray<AActor*>& OutActors) const
{
	OutActors.Reset();

	for (AActor* SpawnedActor : GetSpawnedActors())
	{
		if (SpawnedActor != nullptr)
		{
			OutActors.Add(SpawnedActor);
		}
	}
}

void UAOWeaponInstance::GetSpawnedWeaponAttachComponents(TArray<USceneComponent*>& OutComponents) const
{
	OutComponents.Reset();

	TArray<AActor*> SpawnedWeaponActors;
	GetSpawnedWeaponActors(SpawnedWeaponActors);
	for (AActor* SpawnedWeaponActor : SpawnedWeaponActors)
	{
		if (SpawnedWeaponActor == nullptr)
		{
			continue;
		}

		TInlineComponentArray<USkeletalMeshComponent*> SkeletalMeshComponents;
		SpawnedWeaponActor->GetComponents(SkeletalMeshComponents);
		for (USkeletalMeshComponent* SkeletalMeshComponent : SkeletalMeshComponents)
		{
			if (SkeletalMeshComponent != nullptr)
			{
				OutComponents.AddUnique(SkeletalMeshComponent);
			}
		}

		TInlineComponentArray<UStaticMeshComponent*> StaticMeshComponents;
		SpawnedWeaponActor->GetComponents(StaticMeshComponents);
		for (UStaticMeshComponent* StaticMeshComponent : StaticMeshComponents)
		{
			if (StaticMeshComponent != nullptr)
			{
				OutComponents.AddUnique(StaticMeshComponent);
			}
		}

		if (USceneComponent* RootComponent = SpawnedWeaponActor->GetRootComponent())
		{
			OutComponents.AddUnique(RootComponent);
		}
	}
}

USceneComponent* UAOWeaponInstance::FindSpawnedWeaponAttachComponentBySocket(const FName SocketName) const
{
	if (SocketName.IsNone())
	{
		return nullptr;
	}

	TArray<USceneComponent*> AttachComponents;
	GetSpawnedWeaponAttachComponents(AttachComponents);
	for (USceneComponent* AttachComponent : AttachComponents)
	{
		if (AttachComponent != nullptr && AttachComponent->DoesSocketExist(SocketName))
		{
			return AttachComponent;
		}
	}

	return nullptr;
}
