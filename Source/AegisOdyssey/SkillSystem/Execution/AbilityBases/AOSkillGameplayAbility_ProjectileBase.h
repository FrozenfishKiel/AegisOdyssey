// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AegisOdyssey/SkillSystem/Core/AOSkillGameplayAbility.h"
#include "AOSkillGameplayAbility_ProjectileBase.generated.h"

class AActor;
class UAOSkillProjectileExecutionDefinition;

/**
 * 投射体技能执行基类。
 *
 * 它只负责把“投射体执行定义对象”落成运行时行为，
 * 至于投射体飞行、碰撞、爆炸细节仍由具体投射体 Actor 自己决定。
 */
UCLASS(Abstract)
class AEGISODYSSEY_API UAOSkillGameplayAbility_ProjectileBase : public UAOSkillGameplayAbility
{
	GENERATED_BODY()

public:
	UAOSkillGameplayAbility_ProjectileBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	// 统一读取当前技能绑定的投射体执行定义对象。
	UAOSkillProjectileExecutionDefinition* GetProjectileExecutionDefinition() const;

	// 和火山喷发对齐：
	// 具体生成哪一个运行时投射体类，由具体技能 Ability 自己决定。
	// 基类默认返回空，防止“定义里一份、GA 里一份”双入口重复。
	virtual TSubclassOf<AActor> GetProjectileActorClassToSpawn() const;

	// 统一把执行定义里的“原点语义”解析成最终世界变换。
	bool ResolveProjectileSpawnTransform(FTransform& OutSpawnTransform) const;

	// 按执行定义生成投射体。
	// 它只负责生成，不负责投射体后续是否命中、如何爆炸。
	AActor* SpawnConfiguredProjectile();

	// 给具体技能一个生成后初始化钩子。
	// 例如火球术可以在这里把“当前 SkillAbility”回写给投射体。
	virtual void OnConfiguredProjectileSpawned(AActor* SpawnedProjectile);
};
