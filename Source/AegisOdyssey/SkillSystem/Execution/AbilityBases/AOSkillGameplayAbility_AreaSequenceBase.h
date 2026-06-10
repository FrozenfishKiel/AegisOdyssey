// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AegisOdyssey/SkillSystem/Core/AOSkillGameplayAbility.h"
#include "AOSkillGameplayAbility_AreaSequenceBase.generated.h"

class AActor;
class UAOSkillAreaSequenceExecutionDefinition;

USTRUCT(BlueprintType)
struct FAOSkillAreaWaveResult
{
	GENERATED_BODY()

	// 本次区域序列技能的大逻辑圆圆心。
	UPROPERTY(BlueprintReadOnly, Category = "AO|Skill")
	FVector AreaCenter = FVector::ZeroVector;

	// 本次波次实际命中的小范围中心点。
	UPROPERTY(BlueprintReadOnly, Category = "AO|Skill")
	FVector ImpactPoint = FVector::ZeroVector;

	// 逻辑大圆半径。
	UPROPERTY(BlueprintReadOnly, Category = "AO|Skill")
	float AreaRadius = 0.0f;

	// 单次局部命中半径。
	UPROPERTY(BlueprintReadOnly, Category = "AO|Skill")
	float ImpactRadius = 0.0f;
};

/**
 * 区域序列技能执行基类。
 *
 * 用于承载“前方大区域中，按时间节奏逐次打出小范围命中”的技能。
 */
UCLASS(Abstract)
class AEGISODYSSEY_API UAOSkillGameplayAbility_AreaSequenceBase : public UAOSkillGameplayAbility
{
	GENERATED_BODY()

public:
	UAOSkillGameplayAbility_AreaSequenceBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	// 统一读取当前技能绑定的区域序列执行定义对象。
	UAOSkillAreaSequenceExecutionDefinition* GetAreaSequenceExecutionDefinition() const;

	// 计算某一波区域命中的结果：
	// 1. 先得到角色前方的逻辑大圆
	// 2. 再从大圆内随机一个本波小范围落点
	bool ComputeNextAreaWave(int32 WaveIndex, FAOSkillAreaWaveResult& OutWave) const;

	// 根据单次小范围落点收集命中目标。
	// 这里只回答“谁在范围里”，命中后怎么结算交给 ApplySkillEffectsToTargets。
	void CollectTargetsInImpactRadius(const FVector& ImpactPoint, float ImpactRadius, TArray<AActor*>& OutTargets) const;
};
