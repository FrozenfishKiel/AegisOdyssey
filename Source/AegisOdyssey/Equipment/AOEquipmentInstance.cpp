// Fill out your copyright notice in the Description page of Project Settings.


#include "AOEquipmentInstance.h"
#include "AOEquipmentDefinition.h"
#include "AOWeaponManagerComponent.h"
#include "AegisOdyssey/Items/AOItem.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOEquipmentInstance)

UAOEquipmentInstance::UAOEquipmentInstance(const FObjectInitializer& ObjectInitializer)
{
	
}


void UAOEquipmentInstance::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ThisClass , Instigator);
	DOREPLIFETIME(ThisClass , SpawnedActors);
}


UWorld* UAOEquipmentInstance::GetWorld() const
{
	if (APawn* OwingPawn = GetPawn())
	{
		return OwingPawn->GetWorld();
	}
	else
	{
		return nullptr;
	}
}

APawn* UAOEquipmentInstance::GetPawn() const
{
	return Cast<APawn>(GetOuter()); //创建该instance时会导入Outer，从Outer中获取Pawn（若存在）
}

APawn* UAOEquipmentInstance::GetTypedPawn(TSubclassOf<APawn> PawnType) const
{
	APawn* Result = nullptr;
	if (UClass* ActualPawnType = PawnType)
	{
		if (GetOuter()->IsA(ActualPawnType))
		{
			Result = Cast<APawn>(GetOuter());
		}
	}
	return Result;
}

void UAOEquipmentInstance::SpawnEquipmentActors(const TArray<FAOEquipmentSpawnedConfig> SpawnConfigList)
{
	if (APawn* OwingPawn = GetPawn())
	{
		USceneComponent* AttachTarget = OwingPawn->GetRootComponent();
		if (ACharacter* Char = Cast<ACharacter>(OwingPawn))
		{
			AttachTarget = Char->GetMesh();  //从对应的Pwn中获取对应的Mesh
		}

		for (const FAOEquipmentSpawnedConfig& SpawnConfig : SpawnConfigList)
		{
			AAOItem* NewActor = GetWorld()->SpawnActorDeferred<AAOItem>(SpawnConfig.ActorSpawnedClass , FTransform::Identity , OwingPawn);
			NewActor->FinishSpawning(FTransform::Identity , true);
			NewActor->InitializeActorSpawnConfig();
			NewActor->SetActorRelativeTransform(SpawnConfig.SpawnedTransform);
			NewActor->AttachToComponent(AttachTarget , FAttachmentTransformRules::KeepRelativeTransform , SpawnConfig.AttachSocketName);

			SpawnedActors.Add(NewActor);
		}
	}
}

//卸载武器的时候，会把生成在世界上的武器都销毁
void UAOEquipmentInstance::DestoryEquipmentActors()
{
	for (AActor* Actor : SpawnedActors)
	{
		if (Actor)
		{
			Actor->Destroy();
		}
	}
}

//执行装备的穿戴
void UAOEquipmentInstance::OnEquiped()
{
	K2_OnEquipped();
}

//执行装备的卸载
void UAOEquipmentInstance::OnUnEquiped()
{
	K2_OnUnEquipped();
}

UAOInventoryManagerComponent* UAOEquipmentInstance::FindTargetInventoryManager() const
{
	if (APawn* OwnerPawn = Cast<APawn>(GetOuter()))
	{
		return OwnerPawn->FindComponentByClass<UAOWeaponManagerComponent>();
	}
	return nullptr;
}

void UAOEquipmentInstance::OnRep_Instigator()
{
	
}

void UAOEquipmentInstance::SetItemDef(TSubclassOf<UAOInventoryItemDefinition> InDef)
{
	ItemDef = InDef;
	if (ItemDef)
	{
		if (APawn* OwnerPawn = Cast<APawn>(GetOuter()))
		{
			ItemCDO = NewObject<UAOEquipmentDefinition>(OwnerPawn, InDef);
			OwnerPawn->AddReplicatedSubObject(ItemCDO);
		}
	}
}
