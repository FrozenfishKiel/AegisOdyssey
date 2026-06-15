// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AegisOdyssey/Character/Enemies/AI/Decision/Inventory/AOAIInventoryDecisionTypes.h"
#include "AegisOdyssey/StateTree/AOStateTreeComponentBase.h"
#include "Components/StateTreeComponent.h"
#include "Components/StateTreeAIComponentSchema.h"
#include "AOAILogicStateTreeComponentBase.generated.h"

class UStateTree;
class UAOAIDecisionComponent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class AEGISODYSSEY_API UAOAILogicStateTreeComponentBase : public UAOStateTreeComponentBase
{
	GENERATED_BODY()

public:
	UAOAILogicStateTreeComponentBase();

protected:
	virtual void BeginPlay() override;
	virtual void InitializeComponent() override;
	virtual void UninitializeComponent() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void FullReset() override;

	void ApplyDefaultStateTreeIfNeeded();
	void BindInventoryDecisionEvents();
	void UnbindInventoryDecisionEvents();
	void HandleSubmittedInventoryDecisionChanged(const FAOAIInventoryDecisionResult& SubmittedInventoryDecision);
	bool TryDispatchSubmittedInventoryDecisionEvent(const FAOAIInventoryDecisionResult& SubmittedInventoryDecision);

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "AO AI|StateTree")
	TObjectPtr<UStateTree> DefaultStateTree;

	UPROPERTY(Transient)
	TObjectPtr<UAOAIDecisionComponent> CachedDecisionComponent = nullptr;

	FDelegateHandle SubmittedInventoryDecisionChangedHandle;

	UPROPERTY(Transient)
	bool bHasPendingSubmittedInventoryDecisionEvent = false;

	UPROPERTY(Transient)
	FAOAIInventoryDecisionResult PendingSubmittedInventoryDecisionEvent;
};
