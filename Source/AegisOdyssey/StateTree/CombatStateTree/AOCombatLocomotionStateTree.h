// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AegisOdyssey/StateTree/AOStateTreeComponentBase.h"
#include "AOCombatLocomotionStateTree.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent),DisplayName = "AOCombatLocomotionStateTree")
class AEGISODYSSEY_API UAOCombatLocomotionStateTree : public UAOStateTreeComponentBase
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UAOCombatLocomotionStateTree();
	virtual void CallStateTreeToSentEvent(const FGameplayTag InTargetTag,const EInputType InInputType) override;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void InitializeComponent() override;
	virtual void UninitializeComponent() override;
public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

private:
	FDelegateHandle OnPressInputLoadHandle;
	FDelegateHandle OnReleaseInputLoadHandle;
	FDelegateHandle OnStartInputLoadHandle;

	FDelegateHandle OnPressInputBufferHandle;
	FDelegateHandle OnReleaseInputBufferHandle;
	FDelegateHandle OnStartInputBufferHandle;
};
