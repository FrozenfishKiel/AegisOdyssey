// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/StateTreeComponent.h"
#include "AOStateTreeComponentBase.generated.h"

enum EInputType : uint8;

UCLASS(Blueprintable, meta = (BlueprintSpawnableComponent), DisplayName = "AOStateTreeComponent")
class AEGISODYSSEY_API UAOStateTreeComponentBase : public UStateTreeComponent
{
	GENERATED_BODY()

public:
	UAOStateTreeComponentBase();
	const UStateTree* GetStateTreeAsset() const { return StateTreeRef.GetStateTree(); }
	virtual void CallStateTreeToSentEvent(const FGameplayTag InTargetTag, const EInputType InInputType);

protected:
	virtual void FullReset() {}
};
