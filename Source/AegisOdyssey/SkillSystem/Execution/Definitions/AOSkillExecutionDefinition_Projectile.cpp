// Fill out your copyright notice in the Description page of Project Settings.

#include "AOSkillExecutionDefinition_Projectile.h"

#include "AegisOdyssey/AOSkillCueTags.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOSkillExecutionDefinition_Projectile)

UAOSkillProjectileExecutionDefinition::UAOSkillProjectileExecutionDefinition(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	EffectConfig.HitPolicy.PolicyType = EAOCombatHitPolicyType::SingleHit;
	CueConfig.ExecuteCueTag = AOSkillCueTags::GameplayCue_Skill_Fireball_Impact;
}
