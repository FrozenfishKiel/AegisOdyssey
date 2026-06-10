// Fill out your copyright notice in the Description page of Project Settings.

#include "AOSkillGameplayAbility_ProjectileBase.h"

#include "AegisOdyssey/SkillSystem/Execution/Definitions/AOSkillExecutionDefinition_Projectile.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Engine/World.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOSkillGameplayAbility_ProjectileBase)

UAOSkillGameplayAbility_ProjectileBase::UAOSkillGameplayAbility_ProjectileBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UAOSkillProjectileExecutionDefinition* UAOSkillGameplayAbility_ProjectileBase::GetProjectileExecutionDefinition() const
{
	return Cast<UAOSkillProjectileExecutionDefinition>(GetSkillExecutionDefinitionFromCurrentSourceObject());
}

TSubclassOf<AActor> UAOSkillGameplayAbility_ProjectileBase::GetProjectileActorClassToSpawn() const
{
	return nullptr;
}

bool UAOSkillGameplayAbility_ProjectileBase::ResolveProjectileSpawnTransform(FTransform& OutSpawnTransform) const
{
	const UAOSkillProjectileExecutionDefinition* ExecutionDefinition = GetProjectileExecutionDefinition();
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!ExecutionDefinition || !AvatarActor)
	{
		return false;
	}

	const FAOSkillOriginConfig& OriginConfig = ExecutionDefinition->SpawnOrigin;

	// 先得到技能原点的基础世界变换。
	// 如果配置了有效 Socket，就优先取角色身上任意一个带该 Socket 的 SkeletalMeshComponent；
	// 否则退回角色自身 Transform。
	FTransform BaseTransform = AvatarActor->GetActorTransform();
	if (const ACharacter* Character = Cast<ACharacter>(AvatarActor))
	{
		if (OriginConfig.SocketName != NAME_None)
		{
			// 角色现在可能同时挂主 Mesh、武器 Mesh、额外表现 Mesh。
			// 这里不能只查 GetMesh()，而是要遍历所有 SkeletalMeshComponent，直到找到目标 Socket。
			TInlineComponentArray<USkeletalMeshComponent*> SkeletalMeshComponents(Character);
			for (const USkeletalMeshComponent* MeshComponent : SkeletalMeshComponents)
			{
				if (MeshComponent != nullptr && MeshComponent->DoesSocketExist(OriginConfig.SocketName))
				{
					BaseTransform = MeshComponent->GetSocketTransform(OriginConfig.SocketName);
					break;
				}
			}
		}
	}

	const FTransform OffsetTransform(OriginConfig.RotationOffset, OriginConfig.LocationOffset);
	OutSpawnTransform = OffsetTransform * BaseTransform;

	if (ExecutionDefinition->bUseAvatarFacing)
	{
		FRotator SpawnRotation = AvatarActor->GetActorRotation();

		// 对玩家控制的角色，优先按摄像机/控制器当前朝向来算发射方向。
		// 这样以后上准星时，火球会自然朝屏幕中心线方向飞，而不是只沿角色身体朝向飞。
		if (const APawn* Pawn = Cast<APawn>(AvatarActor))
		{
			if (const AController* Controller = Pawn->GetController())
			{
				SpawnRotation = Controller->GetControlRotation();

				if (const APlayerController* PlayerController = Cast<APlayerController>(Controller))
				{
					FVector ViewStart = FVector::ZeroVector;
					FRotator ViewRotation = SpawnRotation;
					PlayerController->GetPlayerViewPoint(ViewStart, ViewRotation);

					const FVector ViewDirection = ViewRotation.Vector();
					const FVector TraceStart = OutSpawnTransform.GetLocation();
					const float TraceRange = 100000.0f;
					FVector ViewEnd = ViewStart + (ViewDirection * TraceRange);

					FCollisionQueryParams TraceParams(SCENE_QUERY_STAT(FireballAimTrace), false, AvatarActor);
					FHitResult HitResult;
					if (GetWorld()->LineTraceSingleByChannel(HitResult, ViewStart, ViewEnd, ECC_Visibility, TraceParams) && HitResult.bBlockingHit)
					{
						ViewEnd = HitResult.Location;
					}

					const FVector AimDirection = (ViewEnd - TraceStart).GetSafeNormal();
					if (!AimDirection.IsZero())
					{
						SpawnRotation = AimDirection.Rotation();
					}
				}
			}
		}

		SpawnRotation += OriginConfig.RotationOffset;
		OutSpawnTransform.SetRotation(SpawnRotation.Quaternion());
	}

	return true;
}

AActor* UAOSkillGameplayAbility_ProjectileBase::SpawnConfiguredProjectile()
{
	UAOSkillProjectileExecutionDefinition* ExecutionDefinition = GetProjectileExecutionDefinition();
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	UWorld* World = GetWorld();
	const TSubclassOf<AActor> ProjectileClassToSpawn = GetProjectileActorClassToSpawn();
	if (!ExecutionDefinition || !AvatarActor || !World || !ProjectileClassToSpawn)
	{
		return nullptr;
	}

	FTransform SpawnTransform = FTransform::Identity;
	if (!ResolveProjectileSpawnTransform(SpawnTransform))
	{
		return nullptr;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = AvatarActor;
	SpawnParameters.Instigator = Cast<APawn>(AvatarActor);

	// 这里故意只做“生成投射体”这一件事。
	// 投射体飞行、命中、爆炸、二段结算都应该留给投射体 Actor 自己处理。
	AActor* ProjectileActor = World->SpawnActorDeferred<AActor>(
		ProjectileClassToSpawn,
		SpawnTransform,
		AvatarActor,
		SpawnParameters.Instigator);

	if (!ProjectileActor)
	{
		return nullptr;
	}

	const float Lifetime = ExecutionDefinition->ProjectileLifetime.GetValueAtLevel(GetAbilityLevel());
	if (Lifetime > 0.0f)
	{
		// 这是技能层给投射体的生命周期兜底，不替代投射体自身的销毁逻辑。
		ProjectileActor->SetLifeSpan(Lifetime);
	}

	ProjectileActor->FinishSpawning(SpawnTransform);
	OnConfiguredProjectileSpawned(ProjectileActor);
	return ProjectileActor;
}

void UAOSkillGameplayAbility_ProjectileBase::OnConfiguredProjectileSpawned(AActor* SpawnedProjectile)
{
	// 基类默认不做事。
	// 具体技能如果需要给投射体补运行时上下文，可以覆写这里。
}
