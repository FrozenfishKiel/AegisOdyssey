// Fill out your copyright notice in the Description page of Project Settings.

#include "TestHarvestLifecycleActors.h"

#include "Components/BoxComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TestHarvestLifecycleActors)

ATestHarvestLifecycleActor::ATestHarvestLifecycleActor()
{
	TestCollisionComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("TestCollisionComponent"));
	TestCollisionComponent->SetupAttachment(RootScene);
	TestCollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void ATestHarvestLifecycleActor::InitializeForTest()
{
	CapturePrimitiveCollisionSnapshot();
}

UPrimitiveComponent* ATestHarvestLifecycleActor::GetTestCollisionComponent() const
{
	return TestCollisionComponent;
}

bool ATestHarvestLifecycleActor::WasDepletedNativeCalledWithCollisionDisabled() const
{
	return bSawCollisionDisabledOnDepleted;
}

bool ATestHarvestLifecycleActor::WasRespawnNativeCalledWithCollisionRestored() const
{
	return bSawCollisionRestoredOnRespawn;
}

void ATestHarvestLifecycleActor::OnHarvestNodeDepletedNative(const FAOHarvestLifecycleContext& LifecycleContext)
{
	(void)LifecycleContext;
	bSawCollisionDisabledOnDepleted =
		TestCollisionComponent != nullptr &&
		TestCollisionComponent->GetCollisionEnabled() == ECollisionEnabled::NoCollision;
}

void ATestHarvestLifecycleActor::OnHarvestNodeRespawnedNative()
{
	bSawCollisionRestoredOnRespawn =
		TestCollisionComponent != nullptr &&
		TestCollisionComponent->GetCollisionEnabled() == ECollisionEnabled::QueryOnly;
}

ATestHarvestLifecycleTree::ATestHarvestLifecycleTree()
{
	TestTreeBodyComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("TestTreeBodyComponent"));
	TestTreeBodyComponent->SetupAttachment(RootScene);
	TestTreeBodyComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	TestTreeBodyComponent->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	TreeBodyComponent = TestTreeBodyComponent;
	DepletedDisposition = EAOHarvestTreeDepletedDisposition::KeepFallenTree;
	bEnablePhysicsOnDepleted = false;
	FellImpulseStrength = 0.0f;
}

void ATestHarvestLifecycleTree::InitializeForTest()
{
	CapturePrimitiveCollisionSnapshot();
}

UPrimitiveComponent* ATestHarvestLifecycleTree::GetTestTreeBodyComponent() const
{
	return TestTreeBodyComponent;
}

bool ATestHarvestLifecycleTree::WasTreeNativeCalledAfterCommonDepletedState() const
{
	return bSawCommonDepletedStateBeforeTreeNative;
}

void ATestHarvestLifecycleTree::OnHarvestNodeDepletedNative(const FAOHarvestLifecycleContext& LifecycleContext)
{
	(void)LifecycleContext;
	bSawCommonDepletedStateBeforeTreeNative =
		TestTreeBodyComponent != nullptr &&
		TestTreeBodyComponent->GetCollisionEnabled() == ECollisionEnabled::NoCollision;

	Super::OnHarvestNodeDepletedNative(LifecycleContext);
}
