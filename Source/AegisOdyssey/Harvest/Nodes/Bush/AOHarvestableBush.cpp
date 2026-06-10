#include "AegisOdyssey/Harvest/Nodes/Bush/AOHarvestableBush.h"

#include "Components/PrimitiveComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOHarvestableBush)

AAOHarvestableBush::AAOHarvestableBush()
{
}

void AAOHarvestableBush::BeginPlay()
{
	Super::BeginPlay();

	BushBodyComponent = ResolveBushBodyComponent();
}

void AAOHarvestableBush::OnHarvestNodeDepletedNative(const FAOHarvestLifecycleContext& LifecycleContext)
{
	ClearBushHideTimer();
	SetBushHarvestTraceBlocked(false);

	switch (DepletedDisposition)
	{
	case EAOHarvestBushDepletedDisposition::DestroyActor:
		Destroy();
		return;

	case EAOHarvestBushDepletedDisposition::HideBush:
		break;

	case EAOHarvestBushDepletedDisposition::KeepFlattenedBush:
	default:
		break;
	}

	RestoreBushBodyCollisionFromSnapshot();

	if (bEnablePhysicsOnDepleted)
	{
		// Bush 节点只保留轻量受击反馈，随后很快隐藏，不走复杂的残留物理。
		SetBushPhysicsEnabled(true);
		ApplyBushHitImpulse(LifecycleContext);
	}

	if (DepletedDisposition == EAOHarvestBushDepletedDisposition::HideBush)
	{
		StartBushHideTimer();
	}
}

void AAOHarvestableBush::OnHarvestNodeRespawnedNative()
{
	ClearBushHideTimer();
	SetBushPhysicsEnabled(false);
	SetBushVisualState(true);
	SetBushHarvestTraceBlocked(true);
}

void AAOHarvestableBush::ApplyBushHitImpulse(const FAOHarvestLifecycleContext& LifecycleContext)
{
	BushBodyComponent = ResolveBushBodyComponent();
	if (BushBodyComponent == nullptr || HitImpulseStrength <= 0.0f)
	{
		return;
	}

	const FVector HitDirection = ResolveBushHitDirection(LifecycleContext);
	if (HitDirection.IsNearlyZero())
	{
		return;
	}

	ResetBushBodyMotion();

	const FVector Impulse =
		(HitDirection + FVector::UpVector * HitImpulseUpwardRatio).GetSafeNormal() * HitImpulseStrength;
	BushBodyComponent->AddImpulse(Impulse, NAME_None, true);
}

void AAOHarvestableBush::SetBushVisualState(bool bVisible)
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

void AAOHarvestableBush::SetBushPhysicsEnabled(bool bEnabled)
{
	BushBodyComponent = ResolveBushBodyComponent();
	if (BushBodyComponent == nullptr)
	{
		return;
	}

	BushBodyComponent->SetSimulatePhysics(bEnabled);
	if (bEnabled)
	{
		BushBodyComponent->SetLinearDamping(DepletedLinearDamping);
		BushBodyComponent->SetAngularDamping(DepletedAngularDamping);
		ResetBushBodyMotion();
		BushBodyComponent->WakeAllRigidBodies();
	}
}

void AAOHarvestableBush::SetBushHarvestTraceBlocked(bool bBlocked)
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

void AAOHarvestableBush::RestoreBushBodyCollisionFromSnapshot()
{
	BushBodyComponent = ResolveBushBodyComponent();
	if (BushBodyComponent == nullptr)
	{
		return;
	}

	for (const FAOHarvestPrimitiveCollisionSnapshot& Snapshot : PrimitiveCollisionSnapshot)
	{
		if (Snapshot.PrimitiveComponent != BushBodyComponent)
		{
			continue;
		}

		BushBodyComponent->SetCollisionEnabled(Snapshot.CollisionEnabled);
		return;
	}
}

FVector AAOHarvestableBush::ResolveBushHitDirection(const FAOHarvestLifecycleContext& LifecycleContext) const
{
	// Bush 的受击感优先跟随真实命中方向，缺少命中数据时再退回角色朝前。
	FVector HitDirection = LifecycleContext.HitDirection;
	HitDirection.Z = 0.0f;
	if (!HitDirection.IsNearlyZero())
	{
		return HitDirection.GetSafeNormal();
	}

	if (LifecycleContext.HarvesterActor != nullptr)
	{
		HitDirection = LifecycleContext.HarvesterActor->GetActorForwardVector();
		HitDirection.Z = 0.0f;
		if (!HitDirection.IsNearlyZero())
		{
			return HitDirection.GetSafeNormal();
		}
	}

	HitDirection = GetActorForwardVector();
	HitDirection.Z = 0.0f;
	return HitDirection.GetSafeNormal();
}

void AAOHarvestableBush::ResetBushBodyMotion()
{
	BushBodyComponent = ResolveBushBodyComponent();
	if (BushBodyComponent == nullptr)
	{
		return;
	}

	BushBodyComponent->SetPhysicsLinearVelocity(FVector::ZeroVector);
	BushBodyComponent->SetPhysicsAngularVelocityInRadians(FVector::ZeroVector);
}

void AAOHarvestableBush::StartBushHideTimer()
{
	const float HideDelay = ResolveBushHideDelay();
	if (HideDelay <= 0.0f)
	{
		SetBushPhysicsEnabled(false);
		SetBushVisualState(false);
		return;
	}

	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	World->GetTimerManager().ClearTimer(BushHideTimerHandle);
	World->GetTimerManager().SetTimer(BushHideTimerHandle, this, &ThisClass::HandleBushHideTimerFinished, HideDelay, false);
}

void AAOHarvestableBush::ClearBushHideTimer()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(BushHideTimerHandle);
	}
}

void AAOHarvestableBush::HandleBushHideTimerFinished()
{
	SetBushPhysicsEnabled(false);
	SetBushVisualState(false);
}

float AAOHarvestableBush::ResolveBushHideDelay() const
{
	if (DepletedDisposition != EAOHarvestBushDepletedDisposition::HideBush)
	{
		return 0.0f;
	}

	return FMath::Max(0.0f, HideAfterDepletedDelay);
}

UPrimitiveComponent* AAOHarvestableBush::ResolveBushBodyComponent() const
{
	if (BushBodyComponent != nullptr)
	{
		return BushBodyComponent;
	}

	if (UPrimitiveComponent* RootPrimitive = Cast<UPrimitiveComponent>(GetRootComponent()))
	{
		return RootPrimitive;
	}

	TInlineComponentArray<UPrimitiveComponent*> PrimitiveComponents(const_cast<AAOHarvestableBush*>(this));
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
