// Fill out your copyright notice in the Description page of Project Settings.

#include "AOEquipmentInstance.h"

#include "AOEquipmentDefinition.h"
#include "AOWeaponManagerComponent.h"
#include "NativeGameplayTags.h"
#include "Abilities/GameplayAbility.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GameplayPrediction.h"
#include "AegisOdyssey/AbilitySystem/Abilities/GA_PlayAnimationMontage.h"
#include "AegisOdyssey/Character/AOCharacter.h"
#include "AegisOdyssey/Character/AOExtPawnComponent.h"
#include "AegisOdyssey/Equipment/Features/AOEquipmentFeatureAction.h"
#include "AegisOdyssey/Inventory/Fragments/AOFragment_EquipAnimation.h"
#include "AegisOdyssey/Items/AOItem.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOEquipmentInstance)

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Ability_Input_PlayEquipMontage, "Ability.Input.PlayEquipMontage");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Ability_Input_PlayUnEquipMontage, "Ability.Input.PlayUnEquipMontage");

UAOEquipmentInstance::UAOEquipmentInstance(const FObjectInitializer& ObjectInitializer)
{
}

void UAOEquipmentInstance::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ThisClass, Instigator);
	DOREPLIFETIME(ThisClass, SpawnedActors);
}

UWorld* UAOEquipmentInstance::GetWorld() const
{
	if (APawn* OwningPawn = GetPawn())
	{
		return OwningPawn->GetWorld();
	}

	return nullptr;
}

APawn* UAOEquipmentInstance::GetPawn() const
{
	return Cast<APawn>(GetRuntimeOwnerActor());
}

APawn* UAOEquipmentInstance::GetTypedPawn(TSubclassOf<APawn> PawnType) const
{
	if (UClass* ActualPawnType = PawnType)
	{
		if (AActor* ResolvedRuntimeOwnerActor = GetRuntimeOwnerActor())
		{
			if (ResolvedRuntimeOwnerActor->IsA(ActualPawnType))
			{
				return Cast<APawn>(ResolvedRuntimeOwnerActor);
			}
		}
	}

	return nullptr;
}

void UAOEquipmentInstance::SpawnEquipmentActors(const TArray<FAOEquipmentSpawnedConfig> SpawnConfigList)
{
	if (APawn* OwningPawn = GetPawn())
	{
		if (!OwningPawn->HasAuthority())
		{
			return;
		}

		AAOCharacter* AOCharacter = Cast<AAOCharacter>(OwningPawn);
		if (AOCharacter == nullptr)
		{
			return;
		}

		for (const FAOEquipmentSpawnedConfig& SpawnConfig : SpawnConfigList)
		{
			AAOItem* NewActor = GetWorld()->SpawnActorDeferred<AAOItem>(SpawnConfig.ActorSpawnedClass, FTransform::Identity, OwningPawn);
			if (NewActor == nullptr)
			{
				continue;
			}

			UGameplayStatics::FinishSpawningActor(NewActor, FTransform::Identity);
			NewActor->InitializeActorSpawnConfig();
			NewActor->SetActorEnableCollision(false);

			USceneComponent* AttachTarget = AOCharacter->GetEquipmentAttachTargetByTag(SpawnConfig.SpawnedMeshTag);
			if (AttachTarget == nullptr)
			{
				NewActor->Destroy();
				continue;
			}

			NewActor->SetActorRelativeTransform(SpawnConfig.SpawnedTransform);
			NewActor->AttachToComponent(AttachTarget, FAttachmentTransformRules::KeepRelativeTransform, SpawnConfig.AttachSocketName);
			SpawnedActors.AddUnique(NewActor);
		}
	}
}

void UAOEquipmentInstance::DestoryEquipmentActors()
{
	for (AActor* Actor : SpawnedActors)
	{
		if (Actor != nullptr)
		{
			Actor->Destroy();
		}
	}

	SpawnedActors.Reset();
}

void UAOEquipmentInstance::OnEquiped()
{
	ApplyEquipmentFeatureActions();
	PlayEquipAnimation();
}

void UAOEquipmentInstance::OnUnEquiped()
{
	PlayUnEquipAnimation();
	RemoveEquipmentFeatureActions();
}

UAOInventoryManagerComponent* UAOEquipmentInstance::FindTargetInventoryManager() const
{
	if (APawn* OwnerPawn = GetPawn())
	{
		return OwnerPawn->FindComponentByClass<UAOWeaponManagerComponent>();
	}

	return nullptr;
}

bool UAOEquipmentInstance::CanUseFromInventory(const FAOInventoryEntry& InventoryEntry, APawn* UserPawn) const
{
	return InventoryEntry.Instance == this && UserPawn != nullptr && FindTargetInventoryManager() != nullptr;
}

bool UAOEquipmentInstance::TryUseFromInventory(FAOInventoryEntry& InventoryEntry, APawn* UserPawn, int32& OutConsumeCount)
{
	OutConsumeCount = 0;

	if (!CanUseFromInventory(InventoryEntry, UserPawn))
	{
		return false;
	}

	if (UAOInventoryManagerComponent* TargetInventoryManager = FindTargetInventoryManager())
	{
		TargetInventoryManager->OnItemUse(InventoryEntry);
		return true;
	}

	return false;
}

void UAOEquipmentInstance::OnRep_Instigator()
{
}

void UAOEquipmentInstance::OnRep_SpawnedActors()
{
	for (AActor* SpawnedActor : SpawnedActors)
	{
		AAOItem* ItemActor = Cast<AAOItem>(SpawnedActor);
		if (ItemActor == nullptr)
		{
			continue;
		}

		ItemActor->DisableEquippedPresentationCollision();
		ItemActor->SetActorEnableCollision(false);
	}
}

void UAOEquipmentInstance::SetItemDef(TSubclassOf<UAOInventoryItemDefinition> InDef)
{
	ItemDef = InDef;
	if (ItemDef)
	{
		if (APawn* OwnerPawn = GetPawn())
		{
			ItemCDO = NewObject<UAOEquipmentDefinition>(OwnerPawn, InDef);
			OwnerPawn->AddReplicatedSubObject(ItemCDO);
		}
	}
}

void UAOEquipmentInstance::ApplyEquipmentFeatureActions()
{
	if (!ActiveFeatureActionRuntimeData.IsEmpty())
	{
		return;
	}

	APawn* OwnerPawn = GetPawn();
	UAOEquipmentDefinition* EquipmentDefinition = Cast<UAOEquipmentDefinition>(GetItemCDO());
	if (OwnerPawn == nullptr || EquipmentDefinition == nullptr)
	{
		return;
	}

	const TArray<TObjectPtr<UAOEquipmentFeatureAction>>& FeatureActions = EquipmentDefinition->GetFeatureActionsToGrant();
	ActiveFeatureActionRuntimeData.SetNum(FeatureActions.Num());

	for (int32 ActionIndex = 0; ActionIndex < FeatureActions.Num(); ++ActionIndex)
	{
		if (const UAOEquipmentFeatureAction* FeatureAction = FeatureActions[ActionIndex])
		{
			FeatureAction->ActivateForEquipment(this, OwnerPawn, ActiveFeatureActionRuntimeData[ActionIndex]);
		}
	}
}

void UAOEquipmentInstance::RemoveEquipmentFeatureActions()
{
	APawn* OwnerPawn = GetPawn();
	UAOEquipmentDefinition* EquipmentDefinition = Cast<UAOEquipmentDefinition>(GetItemCDO());
	if (OwnerPawn == nullptr || EquipmentDefinition == nullptr || ActiveFeatureActionRuntimeData.IsEmpty())
	{
		ActiveFeatureActionRuntimeData.Reset();
		return;
	}

	const TArray<TObjectPtr<UAOEquipmentFeatureAction>>& FeatureActions = EquipmentDefinition->GetFeatureActionsToGrant();
	const int32 ActionCount = FMath::Min(FeatureActions.Num(), ActiveFeatureActionRuntimeData.Num());

	for (int32 ActionIndex = 0; ActionIndex < ActionCount; ++ActionIndex)
	{
		if (const UAOEquipmentFeatureAction* FeatureAction = FeatureActions[ActionIndex])
		{
			FeatureAction->DeactivateForEquipment(this, OwnerPawn, ActiveFeatureActionRuntimeData[ActionIndex]);
		}
	}

	ActiveFeatureActionRuntimeData.Reset();
}

void UAOEquipmentInstance::PlayEquipAnimation()
{
	const UAOEquipmentDefinition* EquipmentDefinition = Cast<UAOEquipmentDefinition>(GetItemCDO());
	if (EquipmentDefinition == nullptr)
	{
		return;
	}

	const UAOFragment_EquipAnimation* EquipAnimation = EquipmentDefinition->FindFragmentByClass<UAOFragment_EquipAnimation>();
	if (EquipAnimation == nullptr || EquipAnimation->EquipMontage == nullptr)
	{
		return;
	}

	TryPlayEquipmentAnimation(EquipAnimation->EquipMontage, TAG_Ability_Input_PlayEquipMontage);
}

void UAOEquipmentInstance::PlayUnEquipAnimation()
{
	const UAOEquipmentDefinition* EquipmentDefinition = Cast<UAOEquipmentDefinition>(GetItemCDO());
	if (EquipmentDefinition == nullptr)
	{
		return;
	}

	const UAOFragment_EquipAnimation* EquipAnimation = EquipmentDefinition->FindFragmentByClass<UAOFragment_EquipAnimation>();
	if (EquipAnimation == nullptr || EquipAnimation->UnEquipMontage == nullptr)
	{
		return;
	}

	TryPlayEquipmentAnimation(EquipAnimation->UnEquipMontage, TAG_Ability_Input_PlayUnEquipMontage);
}

void UAOEquipmentInstance::TryPlayEquipmentAnimation(UAnimMontage* MontageToPlay, const FGameplayTag& AbilityInputTag) const
{
	if (MontageToPlay == nullptr || !AbilityInputTag.IsValid())
	{
		return;
	}

	AAOCharacter* AOCharacter = Cast<AAOCharacter>(GetPawn());
	if (AOCharacter == nullptr)
	{
		return;
	}

	UAOExtPawnComponent* ExtPawnComponent = AOCharacter->FindComponentByClass<UAOExtPawnComponent>();
	UAbilitySystemComponent* ASC = ExtPawnComponent ? ExtPawnComponent->GetAbilitySystemComponent() : nullptr;
	if (ASC == nullptr)
	{
		return;
	}

	FPlayAnimationMontageTargetData* TargetData = new FPlayAnimationMontageTargetData();
	TargetData->DataMontage = MontageToPlay;

	FGameplayAbilityTargetDataHandle TargetDataHandle;
	TargetDataHandle.Data.Add(TSharedPtr<FGameplayAbilityTargetData>(TargetData));

	FGameplayEventData EventData;
	EventData.EventTag = AbilityInputTag;
	EventData.TargetData = TargetDataHandle;

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(AOCharacter, AbilityInputTag, EventData);
}
