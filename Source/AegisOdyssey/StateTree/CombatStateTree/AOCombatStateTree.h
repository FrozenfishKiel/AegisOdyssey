// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AegisOdyssey/StateTree/AOStateTreeComponentBase.h"
#include "AOCombatStateTree.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent),DisplayName = "AOCombatStateTree")
class AEGISODYSSEY_API UAOCombatStateTree : public UAOStateTreeComponentBase
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UAOCombatStateTree();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
};
