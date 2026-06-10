// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AegisOdyssey/Harvest/Core/AOHarvestableActor.h"
#include "AegisOdyssey/Harvest/Nodes/Tree/AOHarvestableTree.h"
#include "TestHarvestLifecycleActors.generated.h"

class UBoxComponent;

UCLASS()
class AEGISODYSSEY_API ATestHarvestLifecycleActor : public AAOHarvestableActor
{
	GENERATED_BODY()

public:
	ATestHarvestLifecycleActor();

	void InitializeForTest();
	UPrimitiveComponent* GetTestCollisionComponent() const;
	bool WasDepletedNativeCalledWithCollisionDisabled() const;
	bool WasRespawnNativeCalledWithCollisionRestored() const;

protected:
	virtual void OnHarvestNodeDepletedNative(const FAOHarvestLifecycleContext& LifecycleContext) override;
	virtual void OnHarvestNodeRespawnedNative() override;

private:
	UPROPERTY()
	TObjectPtr<UBoxComponent> TestCollisionComponent = nullptr;

	bool bSawCollisionDisabledOnDepleted = false;
	bool bSawCollisionRestoredOnRespawn = false;
};

UCLASS()
class AEGISODYSSEY_API ATestHarvestLifecycleTree : public AAOHarvestableTree
{
	GENERATED_BODY()

public:
	ATestHarvestLifecycleTree();

	void InitializeForTest();
	UPrimitiveComponent* GetTestTreeBodyComponent() const;
	bool WasTreeNativeCalledAfterCommonDepletedState() const;

protected:
	virtual void OnHarvestNodeDepletedNative(const FAOHarvestLifecycleContext& LifecycleContext) override;

private:
	UPROPERTY()
	TObjectPtr<UBoxComponent> TestTreeBodyComponent = nullptr;

	bool bSawCommonDepletedStateBeforeTreeNative = false;
};
