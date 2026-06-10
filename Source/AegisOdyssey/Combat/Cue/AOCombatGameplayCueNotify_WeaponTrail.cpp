#include "AegisOdyssey/Combat/Cue/AOCombatGameplayCueNotify_WeaponTrail.h"

#include "AegisOdyssey/Equipment/AOWeaponManagerComponent.h"
#include "AegisOdyssey/Equipment/Weapons/AOWeaponInstance.h"
#include "Components/SceneComponent.h"
#include "DrawDebugHelpers.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOCombatGameplayCueNotify_WeaponTrail)

AAOCombatGameplayCueNotify_WeaponTrail::AAOCombatGameplayCueNotify_WeaponTrail()
{
	bAutoDestroyOnRemove = false;
	bAutoAttachToOwner = false;
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;
}

void AAOCombatGameplayCueNotify_WeaponTrail::PostLoad()
{
	Super::PostLoad();

	DebugSettings.bDrawCurrentFrame = bDrawDebugTrail_DEPRECATED;
	DebugSettings.CurrentLineThickness = DebugLineThickness_DEPRECATED;
	DebugSettings.CurrentPointSize = DebugPointSize_DEPRECATED;
	DebugSettings.bDrawHistory = false;
	DebugSettings.HistoryLineThickness = DebugHistoryLineThickness_DEPRECATED;
}

bool AAOCombatGameplayCueNotify_WeaponTrail::OnActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters)
{
	return ActivateTrail(MyTarget, Parameters);
}

bool AAOCombatGameplayCueNotify_WeaponTrail::WhileActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters)
{
	return !ActiveTrailEntries.IsEmpty() || ActivateTrail(MyTarget, Parameters);
}

bool AAOCombatGameplayCueNotify_WeaponTrail::OnRemove_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters)
{
	SetActorTickEnabled(false);

	for (FAOWeaponTrailRuntimeEntry& RuntimeEntry : ActiveTrailEntries)
	{
		if (RuntimeEntry.NiagaraComponent != nullptr)
		{
			RuntimeEntry.NiagaraComponent->Deactivate();
			RuntimeEntry.NiagaraComponent->DestroyComponent();
			RuntimeEntry.NiagaraComponent = nullptr;
		}
	}

	ActiveTrailEntries.Reset();
	return true;
}

void AAOCombatGameplayCueNotify_WeaponTrail::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

bool AAOCombatGameplayCueNotify_WeaponTrail::ActivateTrail(AActor* MyTarget, const FGameplayCueParameters& Parameters)
{
	const UAOWeaponInstance* WeaponInstance = ResolveWeaponInstance(MyTarget, Parameters);
	if (WeaponInstance == nullptr)
	{
		return false;
	}

	if (!ActiveTrailEntries.IsEmpty())
	{
		return true;
	}

	for (const FAOCombatWeaponTrailChannel& TrailChannel : TrailChannels)
	{
		if (!TrailChannel.bEnabled || TrailChannel.NiagaraSystem == nullptr)
		{
			continue;
		}

		USceneComponent* StartComponent = ResolveAttachComponent(WeaponInstance, TrailChannel.StartSocketName);
		USceneComponent* EndComponent = ResolveAttachComponent(WeaponInstance, TrailChannel.EndSocketName);
		if (StartComponent == nullptr || EndComponent == nullptr)
		{
			continue;
		}

		const FVector StartLocation = ResolveSocketWorldLocation(StartComponent, TrailChannel.StartSocketName);
		const FVector EndLocation = ResolveSocketWorldLocation(EndComponent, TrailChannel.EndSocketName);
		const FVector TrailCenterLocation = (StartLocation + EndLocation) * 0.5f;
		const FRotator TrailRotation = (EndLocation - StartLocation).Rotation();

		UNiagaraComponent* NiagaraComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
			TrailChannel.NiagaraSystem,
			StartComponent,
			TrailChannel.StartSocketName,
			TrailCenterLocation,
			TrailRotation,
			EAttachLocation::Type::KeepWorldPosition,
			true,
			true,
			ENCPoolMethod::None,
			true);
		if (NiagaraComponent == nullptr)
		{
			continue;
		}

		NiagaraComponent->SetRelativeScale3D(TrailChannel.RelativeScale);
		DrawDebugTrailLine(StartLocation, EndLocation);

		FAOWeaponTrailRuntimeEntry& RuntimeEntry = ActiveTrailEntries.AddDefaulted_GetRef();
		RuntimeEntry.NiagaraComponent = NiagaraComponent;
		RuntimeEntry.StartComponent = StartComponent;
		RuntimeEntry.EndComponent = EndComponent;
		RuntimeEntry.StartSocketName = TrailChannel.StartSocketName;
		RuntimeEntry.EndSocketName = TrailChannel.EndSocketName;
		RuntimeEntry.StartPointVariableName = TrailChannel.StartPointVariableName;
		RuntimeEntry.EndPointVariableName = TrailChannel.EndPointVariableName;
	}

	if (ActiveTrailEntries.IsEmpty())
	{
		return false;
	}

	return true;
}

const UAOWeaponInstance* AAOCombatGameplayCueNotify_WeaponTrail::ResolveWeaponInstance(AActor* MyTarget, const FGameplayCueParameters& Parameters) const
{
	if (const UAOWeaponInstance* WeaponInstance = Cast<const UAOWeaponInstance>(Parameters.SourceObject.Get()))
	{
		return WeaponInstance;
	}

	APawn* OwnerPawn = Cast<APawn>(MyTarget);
	if (OwnerPawn == nullptr && MyTarget != nullptr)
	{
		OwnerPawn = Cast<APawn>(MyTarget->GetOwner());
	}

	if (OwnerPawn == nullptr)
	{
		return nullptr;
	}

	const UAOWeaponManagerComponent* WeaponManagerComponent = OwnerPawn->FindComponentByClass<UAOWeaponManagerComponent>();
	return WeaponManagerComponent != nullptr
		? Cast<const UAOWeaponInstance>(WeaponManagerComponent->GetCurrentWeaponInstance())
		: nullptr;
}

USceneComponent* AAOCombatGameplayCueNotify_WeaponTrail::ResolveAttachComponent(const UAOWeaponInstance* WeaponInstance, const FName SocketName) const
{
	return WeaponInstance != nullptr ? WeaponInstance->FindSpawnedWeaponAttachComponentBySocket(SocketName) : nullptr;
}

FVector AAOCombatGameplayCueNotify_WeaponTrail::ResolveSocketWorldLocation(const USceneComponent* Component, const FName SocketName)
{
	if (Component == nullptr)
	{
		return FVector::ZeroVector;
	}

	return !SocketName.IsNone() && Component->DoesSocketExist(SocketName)
		? Component->GetSocketLocation(SocketName)
		: Component->GetComponentLocation();
}

void AAOCombatGameplayCueNotify_WeaponTrail::DrawDebugTrailLine(const FVector& StartLocation, const FVector& EndLocation) const
{
	if (!DebugSettings.bDrawCurrentFrame)
	{
		return;
	}

	const UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	DrawDebugLine(
		World,
		StartLocation,
		EndLocation,
		FColor::Cyan,
		false,
		0.0f,
		0,
		DebugSettings.CurrentLineThickness);

	DrawDebugPoint(
		World,
		StartLocation,
		DebugSettings.CurrentPointSize,
		FColor::Green,
		false,
		0.0f);

	DrawDebugPoint(
		World,
		EndLocation,
		DebugSettings.CurrentPointSize,
		FColor::Red,
		false,
		0.0f);
}

void AAOCombatGameplayCueNotify_WeaponTrail::UpdateTrailEntries()
{
	// 当前试验分支只围绕 SpawnSystemAttached 的初始世界变换来驱动，不再在 Tick 中继续修正组件或推送参数。
}
