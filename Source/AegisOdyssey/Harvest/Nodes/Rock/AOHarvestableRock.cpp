#include "AegisOdyssey/Harvest/Nodes/Rock/AOHarvestableRock.h"

#include "Components/PrimitiveComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOHarvestableRock)

AAOHarvestableRock::AAOHarvestableRock()
{
}

void AAOHarvestableRock::BeginPlay()
{
	Super::BeginPlay();

	RockBodyComponent = ResolveRockBodyComponent();
}

void AAOHarvestableRock::OnHarvestNodeDepletedNative(const FAOHarvestLifecycleContext& LifecycleContext)
{
	ClearRockHideTimer();
	SetRockHarvestTraceBlocked(false);

	switch (DepletedDisposition)
	{
	case EAOHarvestRockDepletedDisposition::DestroyActor:
		Destroy();
		return;

	case EAOHarvestRockDepletedDisposition::HideRock:
		break;

	case EAOHarvestRockDepletedDisposition::KeepBrokenRock:
	default:
		break;
	}

	RestoreRockBodyCollisionFromSnapshot();

	if (bEnablePhysicsOnDepleted)
	{
		// Rock 节点默认走“受击崩开”路线，而不是树那种定向起倒。
		SetRockPhysicsEnabled(true);
		ApplyRockBreakImpulse(LifecycleContext);
	}

	if (DepletedDisposition == EAOHarvestRockDepletedDisposition::HideRock)
	{
		StartRockHideTimer();
	}
}

void AAOHarvestableRock::OnHarvestNodeRespawnedNative()
{
	ClearRockHideTimer();
	SetRockPhysicsEnabled(false);
	SetRockVisualState(true);
	SetRockHarvestTraceBlocked(true);
}

void AAOHarvestableRock::ApplyRockBreakImpulse(const FAOHarvestLifecycleContext& LifecycleContext)
{
	RockBodyComponent = ResolveRockBodyComponent();
	if (RockBodyComponent == nullptr || BreakImpulseStrength <= 0.0f)
	{
		return;
	}

	const FVector BreakDirection = ResolveRockBreakDirection(LifecycleContext);
	if (BreakDirection.IsNearlyZero())
	{
		return;
	}

	ResetRockBodyMotion();

	const FVector Impulse =
		(BreakDirection + FVector::UpVector * BreakImpulseUpwardRatio).GetSafeNormal() * BreakImpulseStrength;
	RockBodyComponent->AddImpulse(Impulse, NAME_None, true);
}

void AAOHarvestableRock::SetRockVisualState(bool bVisible)
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

void AAOHarvestableRock::SetRockPhysicsEnabled(bool bEnabled)
{
	RockBodyComponent = ResolveRockBodyComponent();
	if (RockBodyComponent == nullptr)
	{
		return;
	}

	RockBodyComponent->SetSimulatePhysics(bEnabled);
	if (bEnabled)
	{
		RockBodyComponent->SetLinearDamping(DepletedLinearDamping);
		RockBodyComponent->SetAngularDamping(DepletedAngularDamping);
		ResetRockBodyMotion();
		RockBodyComponent->WakeAllRigidBodies();
	}
}

void AAOHarvestableRock::SetRockHarvestTraceBlocked(bool bBlocked)
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

void AAOHarvestableRock::RestoreRockBodyCollisionFromSnapshot()
{
	RockBodyComponent = ResolveRockBodyComponent();
	if (RockBodyComponent == nullptr)
	{
		return;
	}

	for (const FAOHarvestPrimitiveCollisionSnapshot& Snapshot : PrimitiveCollisionSnapshot)
	{
		if (Snapshot.PrimitiveComponent != RockBodyComponent)
		{
			continue;
		}

		RockBodyComponent->SetCollisionEnabled(Snapshot.CollisionEnabled);
		return;
	}
}

FVector AAOHarvestableRock::ResolveRockBreakDirection(const FAOHarvestLifecycleContext& LifecycleContext) const
{
	// Rock 优先沿真实命中方向崩开；只有缺少命中信息时才退回采集者朝向。
	FVector BreakDirection = LifecycleContext.HitDirection;
	BreakDirection.Z = 0.0f;
	if (!BreakDirection.IsNearlyZero())
	{
		return BreakDirection.GetSafeNormal();
	}

	if (LifecycleContext.HarvesterActor != nullptr)
	{
		BreakDirection = LifecycleContext.HarvesterActor->GetActorForwardVector();
		BreakDirection.Z = 0.0f;
		if (!BreakDirection.IsNearlyZero())
		{
			return BreakDirection.GetSafeNormal();
		}
	}

	BreakDirection = GetActorForwardVector();
	BreakDirection.Z = 0.0f;
	return BreakDirection.GetSafeNormal();
}

void AAOHarvestableRock::ResetRockBodyMotion()
{
	RockBodyComponent = ResolveRockBodyComponent();
	if (RockBodyComponent == nullptr)
	{
		return;
	}

	RockBodyComponent->SetPhysicsLinearVelocity(FVector::ZeroVector);
	RockBodyComponent->SetPhysicsAngularVelocityInRadians(FVector::ZeroVector);
}

void AAOHarvestableRock::StartRockHideTimer()
{
	const float HideDelay = ResolveRockHideDelay();
	if (HideDelay <= 0.0f)
	{
		SetRockPhysicsEnabled(false);
		SetRockVisualState(false);
		return;
	}

	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	World->GetTimerManager().ClearTimer(RockHideTimerHandle);
	World->GetTimerManager().SetTimer(RockHideTimerHandle, this, &ThisClass::HandleRockHideTimerFinished, HideDelay, false);
}

void AAOHarvestableRock::ClearRockHideTimer()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(RockHideTimerHandle);
	}
}

void AAOHarvestableRock::HandleRockHideTimerFinished()
{
	SetRockPhysicsEnabled(false);
	SetRockVisualState(false);
}

float AAOHarvestableRock::ResolveRockHideDelay() const
{
	if (DepletedDisposition != EAOHarvestRockDepletedDisposition::HideRock)
	{
		return 0.0f;
	}

	return FMath::Max(0.0f, HideAfterDepletedDelay);
}

UPrimitiveComponent* AAOHarvestableRock::ResolveRockBodyComponent() const
{
	if (RockBodyComponent != nullptr)
	{
		return RockBodyComponent;
	}

	if (UPrimitiveComponent* RootPrimitive = Cast<UPrimitiveComponent>(GetRootComponent()))
	{
		return RootPrimitive;
	}

	TInlineComponentArray<UPrimitiveComponent*> PrimitiveComponents(const_cast<AAOHarvestableRock*>(this));
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
