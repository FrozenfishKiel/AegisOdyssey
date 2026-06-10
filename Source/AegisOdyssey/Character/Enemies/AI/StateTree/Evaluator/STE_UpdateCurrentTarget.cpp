#include "STE_UpdateCurrentTarget.h"

#include "AegisOdyssey/Equipment/AOWeaponManagerComponent.h"
#include "AegisOdyssey/Equipment/Weapons/AOWeaponDefinition.h"
#include "AegisOdyssey/Equipment/Weapons/AOWeaponInstance.h"
#include "AegisOdyssey/Player/AAOAIPlayerBotController.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "StateTreeExecutionContext.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(STE_UpdateCurrentTarget)

namespace
{
constexpr float DefaultAIAttackRange = 200.0f;

APawn* ResolveOwnerPawn(const FStateTreeExecutionContext& Context)
{
	if (AActor* OwnerActor = Cast<AActor>(Context.GetOwner()))
	{
		if (APawn* OwnerPawn = Cast<APawn>(OwnerActor))
		{
			return OwnerPawn;
		}

		if (AController* OwnerController = Cast<AController>(OwnerActor))
		{
			return OwnerController->GetPawn();
		}
	}

	return nullptr;
}

float ResolveAIAttackRange(const APawn* OwnerPawn)
{
	if (OwnerPawn == nullptr)
	{
		return DefaultAIAttackRange;
	}

	if (const UAOWeaponManagerComponent* WeaponManagerComponent = OwnerPawn->FindComponentByClass<UAOWeaponManagerComponent>())
	{
		if (const UAOWeaponInstance* WeaponInstance = Cast<UAOWeaponInstance>(WeaponManagerComponent->GetCurrentWeaponInstance()))
		{
			if (const UAOWeaponDefinition* WeaponDefinition = Cast<UAOWeaponDefinition>(WeaponInstance->GetItemCDO()))
			{
				return FMath::Max(1.0f, WeaponDefinition->GetAIAttackRange());
			}
		}
	}

	return DefaultAIAttackRange;
}
}

void FSTE_UpdateCurrentTarget::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	APawn* OwnerPawn = ResolveOwnerPawn(Context);
	AAOAIPlayerBotController* AIController = OwnerPawn ? Cast<AAOAIPlayerBotController>(OwnerPawn->GetController()) : nullptr;

	if (AIController == nullptr)
	{
		InstanceData.CurrentTarget = nullptr;
		InstanceData.DistanceToTarget = 0.0f;
		InstanceData.bIsInAttackRange = false;
		InstanceData.bHasTarget = false;
		return;
	}

	AActor* Target = AIController->GetCurrentTarget();
	InstanceData.CurrentTarget = Target;
	InstanceData.bHasTarget = (Target != nullptr);

	if (OwnerPawn != nullptr && Target != nullptr)
	{
		const float Distance = FVector::Dist(OwnerPawn->GetActorLocation(), Target->GetActorLocation());
		const float EffectiveAttackRange = ResolveAIAttackRange(OwnerPawn);
		InstanceData.DistanceToTarget = Distance;
		InstanceData.bIsInAttackRange = (Distance <= EffectiveAttackRange);
		return;
	}

	InstanceData.DistanceToTarget = 0.0f;
	InstanceData.bIsInAttackRange = false;
}
