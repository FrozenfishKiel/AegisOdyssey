// Fill out your copyright notice in the Description page of Project Settings.


#include "AOItem.h"
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

	bReplicates = true;  //物品都是可复制的
}

// Called when the game starts or when spawned
void AAOItem::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
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

FInventoryPickUp AAOItem::GetPickUpInventory() const
{
	return StaticInventory;
}

