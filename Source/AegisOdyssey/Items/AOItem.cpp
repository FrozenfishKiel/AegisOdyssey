// Fill out your copyright notice in the Description page of Project Settings.


#include "AOItem.h"

#include "AegisOdyssey/Interaction/PickUpable.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOItem)

AAOItem::AAOItem()
{
	PrimaryActorTick.bCanEverTick = true;

	PickUpBox = CreateDefaultSubobject<UBoxComponent>("PickUpBox");
	SetRootComponent(PickUpBox);

	DefaultStaticMesh = CreateDefaultSubobject<UStaticMeshComponent>("DefaultStaticMesh");
	DefaultStaticMesh->SetupAttachment(PickUpBox);

	DefaultSkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>("DefaultSkeletalMesh");
	DefaultSkeletalMesh->SetupAttachment(PickUpBox);

	bReplicates = true;
}

void AAOItem::BeginPlay()
{
	Super::BeginPlay();
}

void AAOItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AAOItem::GatherInteractionOptions(FInteractionOptionBuilder& OptionBuilder)
{
	OptionBuilder.AddInteractionOption(Option);
}

void AAOItem::CustomizeInteractionEventData(const FGameplayTag& InteractionEventTag, FGameplayEventData& InOutEventData)
{
	IInteractableTarget::CustomizeInteractionEventData(InteractionEventTag, InOutEventData);
}

bool AAOItem::ExecuteInteraction(const FGameplayTag& InteractionEventTag, const FGameplayEventData& EventData)
{
	// Pickup resolves on authority only; the item simply forwards itself into the interactor's inventory chain.
	const AActor* InstigatorActorConst = Cast<AActor>(EventData.Instigator.Get());
	AActor* InstigatorActor = const_cast<AActor*>(InstigatorActorConst);
	if (!HasAuthority() || !InstigatorActor)
	{
		return false;
	}

	TScriptInterface<IPickUpable> PickUpable(this);
	bool bAddedToInventory = false;
	UPickUpableStatics::TryAddPickupToActorInventories(InstigatorActor, PickUpable, bAddedToInventory);

	if (bAddedToInventory)
	{
		Destroy();
		return true;
	}

	return false;
}

FInventoryPickUp AAOItem::GetPickUpInventory() const
{
	return StaticInventory;
}

void AAOItem::DisableEquippedPresentationCollision()
{
	SetActorEnableCollision(false);

	if (PickUpBox != nullptr)
	{
		PickUpBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	if (DefaultStaticMesh != nullptr)
	{
		DefaultStaticMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		DefaultStaticMesh->SetGenerateOverlapEvents(false);
	}

	if (DefaultSkeletalMesh != nullptr)
	{
		DefaultSkeletalMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		DefaultSkeletalMesh->SetGenerateOverlapEvents(false);
	}
}
