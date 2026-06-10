// Fill out your copyright notice in the Description page of Project Settings.

#include "AOSkillGameplayAbility_AreaSequenceBase.h"

#include "AegisOdyssey/SkillSystem/Execution/Definitions/AOSkillExecutionDefinition_AreaSequence.h"
#include "AegisOdyssey/Character/CombatInterface.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/KismetSystemLibrary.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOSkillGameplayAbility_AreaSequenceBase)

namespace AOSkillGameplayAbilityAreaSequenceBasePrivate
{
	static TAutoConsoleVariable<int32> CVarSkillDebugDraw(
		TEXT("ao.Skill.DebugDraw"),
		0,
		TEXT("Enable debug draw for AO skill execution helpers.")
	);
}

UAOSkillGameplayAbility_AreaSequenceBase::UAOSkillGameplayAbility_AreaSequenceBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UAOSkillAreaSequenceExecutionDefinition* UAOSkillGameplayAbility_AreaSequenceBase::GetAreaSequenceExecutionDefinition() const
{
	return Cast<UAOSkillAreaSequenceExecutionDefinition>(GetSkillExecutionDefinitionFromCurrentSourceObject());
}

bool UAOSkillGameplayAbility_AreaSequenceBase::ComputeNextAreaWave(int32 WaveIndex, FAOSkillAreaWaveResult& OutWave) const
{
	const UAOSkillAreaSequenceExecutionDefinition* ExecutionDefinition = GetAreaSequenceExecutionDefinition();
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!ExecutionDefinition || !AvatarActor)
	{
		return false;
	}

	if (WaveIndex < 0 || WaveIndex >= ExecutionDefinition->WaveCount)
	{
		return false;
	}

	// “火山喷发”这类技能的程序真相：
	// 1. 先根据角色朝向推一个前方逻辑大圆
	// 2. 每一波再从这个大圆内部随机一个落点
	// 3. 落点周围的小圆才是真正参与命中的伤害范围
	const float AbilityLevel = GetAbilityLevel();
	const FVector Forward = AvatarActor->GetActorForwardVector();
	const FVector Right = AvatarActor->GetActorRightVector();
	const FVector Center =
		AvatarActor->GetActorLocation()
		+ Forward * ExecutionDefinition->AreaCenterForwardDistance.GetValueAtLevel(AbilityLevel)
		+ AvatarActor->GetActorRotation().RotateVector(ExecutionDefinition->AreaCenterOffset);

	const float AreaRadius = ExecutionDefinition->AreaRadius.GetValueAtLevel(AbilityLevel);
	const float ImpactRadius = ExecutionDefinition->ImpactRadius.GetValueAtLevel(AbilityLevel);
	const FVector2D RandomInCircle = FMath::RandPointInCircle(AreaRadius);
	const FVector ImpactPoint = Center + Forward * RandomInCircle.X + Right * RandomInCircle.Y;

	OutWave.AreaCenter = Center;
	OutWave.ImpactPoint = ImpactPoint;
	OutWave.AreaRadius = AreaRadius;
	OutWave.ImpactRadius = ImpactRadius;

	const bool bCanDrawDebug =
		ExecutionDefinition->DebugConfig.bEnableDebugDraw
		&& AOSkillGameplayAbilityAreaSequenceBasePrivate::CVarSkillDebugDraw.GetValueOnGameThread() != 0;
	if (bCanDrawDebug)
	{
		// 调试时同时画出“大逻辑圆”和“本波真实命中小圆”，方便验配置是否正确。
		const FAOSkillDebugConfig& DebugConfig = ExecutionDefinition->DebugConfig;
		DrawDebugSphere(GetWorld(), Center, AreaRadius, 24, DebugConfig.PrimaryDebugColor.ToFColor(true), false, DebugConfig.DebugDrawDuration);
		DrawDebugSphere(GetWorld(), ImpactPoint, ImpactRadius, 16, DebugConfig.SecondaryDebugColor.ToFColor(true), false, DebugConfig.DebugDrawDuration);
	}

	return true;
}

void UAOSkillGameplayAbility_AreaSequenceBase::CollectTargetsInImpactRadius(
	const FVector& ImpactPoint,
	float ImpactRadius,
	TArray<AActor*>& OutTargets) const
{
	OutTargets.Reset();

	if (!GetWorld() || ImpactRadius <= 0.0f)
	{
		return;
	}

	TArray<AActor*> IgnoreActors;
	if (AActor* AvatarActor = GetAvatarActorFromActorInfo())
	{
		IgnoreActors.Add(AvatarActor);
	}

	TArray<AActor*> OverlappedActors;
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(EObjectTypeQuery::ObjectTypeQuery3);

	// 这里只做“范围里有哪些目标”的收集，不做任何结算。
	// 收集完之后，交给 ApplySkillEffectsToTargets 继续走现有战斗尾链。
	if (!UKismetSystemLibrary::SphereOverlapActors(
		this,
		ImpactPoint,
		ImpactRadius,
		ObjectTypes,
		AActor::StaticClass(),
		IgnoreActors,
		OverlappedActors))
	{
		return;
	}

	for (AActor* Target : OverlappedActors)
	{
		if (Target && Target->Implements<UCombatInterface>())
		{
			OutTargets.AddUnique(Target);
		}
	}
}
