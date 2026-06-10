// Fill out your copyright notice in the Description page of Project Settings.

#include "AOSkillExecutionDefinition_AreaSequence.h"

#include "AegisOdyssey/AOSkillCueTags.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOSkillExecutionDefinition_AreaSequence)

UAOSkillAreaSequenceExecutionDefinition::UAOSkillAreaSequenceExecutionDefinition(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	EffectConfig.HitPolicy.PolicyType = EAOCombatHitPolicyType::PeriodicRepeat;
	EffectConfig.HitPolicy.RepeatIntervalSeconds = 0.25f;
	CueConfig.ExecuteCueTag = AOSkillCueTags::GameplayCue_Skill_VolcanoBurst_WaveImpact;
}
