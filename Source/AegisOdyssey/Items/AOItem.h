// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AegisOdyssey/Interaction/InteractableTarget.h"
#include "AegisOdyssey/Interaction/PickUpable.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Actor.h"
#include "AOItem.generated.h"

UCLASS(Blueprintable,BlueprintType)
class AEGISODYSSEY_API AAOItem : public AActor , public IInteractableTarget , public IPickUpable
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AAOItem();
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> DefaultStaticMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<USkeletalMeshComponent> DefaultSkeletalMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UBoxComponent> PickUpBox;

	
	virtual void InitializeActorSpawnConfig(){}
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void GatherInteractionOptions(FInteractionOptionBuilder& OptionBuilder) override;

	virtual void CustomizeInteractionEventData(const FGameplayTag& InteractionEventTag, FGameplayEventData& InOutEventData) override;

	virtual FInventoryPickUp GetPickUpInventory() const override;
protected:
	UPROPERTY(EditAnywhere , Category = "Interaction Options")
	FInteractionOption Option;

	UPROPERTY(EditAnywhere , Category = "Interaction Options")
	FInventoryPickUp StaticInventory;
};
