// Fill out your copyright notice in the Description page of Project Settings.

#include "AegisOdyssey/Harvest/Nodes/Tree/AOHarvestableTree.h"

#include "Components/PrimitiveComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOHarvestableTree)

AAOHarvestableTree::AAOHarvestableTree()
{
}

void AAOHarvestableTree::BeginPlay()
{
	Super::BeginPlay();

	TreeBodyComponent = ResolveTreeBodyComponent();
}

void AAOHarvestableTree::OnHarvestNodeDepletedNative(const FAOHarvestLifecycleContext& LifecycleContext)
{
	ClearTreeHideTimer();
	SetTreeHarvestTraceBlocked(false);

	switch (DepletedDisposition)
	{
	case EAOHarvestTreeDepletedDisposition::DestroyActor:
		Destroy();
		return;

	case EAOHarvestTreeDepletedDisposition::HideTree:
		break;

	case EAOHarvestTreeDepletedDisposition::KeepFallenTree:
	default:
		break;
	}

	// 公共 depleted 状态会先关闭所有 Primitive 碰撞。
	// KeepFallenTree 需要把主体碰撞恢复回来，后续倒地物理才能继续成立。
	RestoreTreeBodyCollisionFromSnapshot();

	if (bEnablePhysicsOnDepleted)
	{
		SetTreePhysicsEnabled(true);
		ApplyTreeFellImpulse(LifecycleContext);
	}

	if (DepletedDisposition == EAOHarvestTreeDepletedDisposition::HideTree)
	{
		StartTreeHideTimer();
	}
}

void AAOHarvestableTree::OnHarvestNodeRespawnedNative()
{
	ClearTreeHideTimer();
	SetTreePhysicsEnabled(false);
	SetTreeVisualState(true);
	SetTreeHarvestTraceBlocked(true);
}

void AAOHarvestableTree::ApplyTreeFellImpulse(const FAOHarvestLifecycleContext& LifecycleContext)
{
	TreeBodyComponent = ResolveTreeBodyComponent();
	if (TreeBodyComponent == nullptr || FellImpulseStrength <= 0.0f)
	{
		return;
	}

	const FVector FallDirection = ResolveTreeFallDirection(LifecycleContext);
	if (FallDirection.IsNearlyZero())
	{
		return;
	}

	const FVector TorqueAxis = ResolveTreeFallTorqueAxis(LifecycleContext);
	if (TorqueAxis.IsNearlyZero())
	{
		return;
	}

	ResetTreeBodyMotion();

	const float TorqueStrength = FellImpulseStrength * FMath::Max(1.0f, 1.0f + UpwardImpulseRatio);
	const FVector Torque = TorqueAxis * TorqueStrength;
	TreeBodyComponent->AddTorqueInRadians(Torque, NAME_None, true);
}

void AAOHarvestableTree::SetTreeVisualState(bool bVisible)
{
	SetActorHiddenInGame(!bVisible);
	SetActorEnableCollision(bVisible);

	TInlineComponentArray<UPrimitiveComponent*> PrimitiveComponents(this);
	for (UPrimitiveComponent* PrimitiveComponent : PrimitiveComponents)
	{
		if (PrimitiveComponent == nullptr)
		{
			continue;
		}

		PrimitiveComponent->SetHiddenInGame(!bVisible);
		PrimitiveComponent->SetVisibility(bVisible, true);
	}
}

void AAOHarvestableTree::SetTreePhysicsEnabled(bool bEnabled)
{
	TreeBodyComponent = ResolveTreeBodyComponent();
	if (TreeBodyComponent == nullptr)
	{
		return;
	}

	TreeBodyComponent->SetSimulatePhysics(bEnabled);
	if (bEnabled)
	{
		TreeBodyComponent->SetLinearDamping(DepletedLinearDamping);
		TreeBodyComponent->SetAngularDamping(DepletedAngularDamping);
		if (MaxDepletedAngularVelocity > 0.0f)
		{
			TreeBodyComponent->SetPhysicsMaxAngularVelocityInDegrees(MaxDepletedAngularVelocity, false, NAME_None);
		}

		ResetTreeBodyMotion();
		TreeBodyComponent->WakeAllRigidBodies();
	}
}

void AAOHarvestableTree::SetTreeHarvestTraceBlocked(bool bBlocked)
{
	TInlineComponentArray<UPrimitiveComponent*> PrimitiveComponents(this);
	for (UPrimitiveComponent* PrimitiveComponent : PrimitiveComponents)
	{
		if (PrimitiveComponent == nullptr)
		{
			continue;
		}

		PrimitiveComponent->SetCollisionResponseToChannel(ECC_Visibility, bBlocked ? ECR_Block : ECR_Ignore);
	}
}

void AAOHarvestableTree::RestoreTreeBodyCollisionFromSnapshot()
{
	TreeBodyComponent = ResolveTreeBodyComponent();
	if (TreeBodyComponent == nullptr)
	{
		return;
	}

	for (const FAOHarvestPrimitiveCollisionSnapshot& Snapshot : PrimitiveCollisionSnapshot)
	{
		if (Snapshot.PrimitiveComponent != TreeBodyComponent)
		{
			continue;
		}

		TreeBodyComponent->SetCollisionEnabled(Snapshot.CollisionEnabled);
		return;
	}
}

void AAOHarvestableTree::StartTreeHideTimer()
{
	const float HideDelay = ResolveTreeHideDelay();
	if (HideDelay <= 0.0f)
	{
		SetTreePhysicsEnabled(false);
		SetTreeVisualState(false);
		return;
	}

	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	World->GetTimerManager().ClearTimer(TreeHideTimerHandle);
	World->GetTimerManager().SetTimer(TreeHideTimerHandle, this, &ThisClass::HandleTreeHideTimerFinished, HideDelay, false);
}

void AAOHarvestableTree::ClearTreeHideTimer()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(TreeHideTimerHandle);
	}
}

void AAOHarvestableTree::HandleTreeHideTimerFinished()
{
	SetTreePhysicsEnabled(false);
	SetTreeVisualState(false);
}

float AAOHarvestableTree::ResolveTreeHideDelay() const
{
	if (DepletedDisposition != EAOHarvestTreeDepletedDisposition::HideTree)
	{
		return 0.0f;
	}

	return FMath::Max(0.0f, HideAfterDepletedDelay);
}

FVector AAOHarvestableTree::ResolveTreeFallDirection(const FAOHarvestLifecycleContext& LifecycleContext) const
{
	FVector FallDirection = ResolveHarvesterPushDirection(LifecycleContext);

	if (FallDirection.IsNearlyZero())
	{
		FallDirection = GetActorForwardVector();
		FallDirection.Z = 0.0f;
	}

	return FallDirection;
}

FVector AAOHarvestableTree::ResolveHarvesterPushDirection(const FAOHarvestLifecycleContext& LifecycleContext) const
{
	if (LifecycleContext.bHasHarvesterForward)
	{
		FVector PushDirection = LifecycleContext.HarvesterForward;
		PushDirection.Z = 0.0f;
		return PushDirection;
	}

	if (LifecycleContext.HarvesterActor == nullptr)
	{
		return FVector::ZeroVector;
	}

	FVector PushDirection = LifecycleContext.HarvesterActor->GetActorForwardVector();
	PushDirection.Z = 0.0f;
	return PushDirection;
}

FVector AAOHarvestableTree::ResolveTreeFallTorqueAxis(const FAOHarvestLifecycleContext& LifecycleContext) const
{
	const FVector FallDirection = ResolveTreeFallDirection(LifecycleContext);
	if (FallDirection.IsNearlyZero())
	{
		return FVector::ZeroVector;
	}

	// 扭矩轴必须与“角色朝前推”的水平向量正交，才能让树沿着角色前方方向起倒，
	// 这里不再混入任何额外侧向分量，避免出现先横移或朝两边偏倒的感觉。
	return FVector::CrossProduct(FVector::UpVector, FallDirection);
}

void AAOHarvestableTree::ResetTreeBodyMotion()
{
	TreeBodyComponent = ResolveTreeBodyComponent();
	if (TreeBodyComponent == nullptr)
	{
		return;
	}

	TreeBodyComponent->SetPhysicsLinearVelocity(FVector::ZeroVector);
	TreeBodyComponent->SetPhysicsAngularVelocityInRadians(FVector::ZeroVector);
}

UPrimitiveComponent* AAOHarvestableTree::ResolveTreeBodyComponent() const
{
	if (TreeBodyComponent != nullptr)
	{
		return TreeBodyComponent;
	}

	if (UPrimitiveComponent* RootPrimitive = Cast<UPrimitiveComponent>(GetRootComponent()))
	{
		return RootPrimitive;
	}

	TInlineComponentArray<UPrimitiveComponent*> PrimitiveComponents(const_cast<AAOHarvestableTree*>(this));
	for (UPrimitiveComponent* PrimitiveComponent : PrimitiveComponents)
	{
		if (PrimitiveComponent == nullptr || PrimitiveComponent == GetRootComponent())
		{
			continue;
		}

		if (PrimitiveComponent->Mobility == EComponentMobility::Movable)
		{
			return PrimitiveComponent;
		}
	}

	return FindComponentByClass<UPrimitiveComponent>();
}
