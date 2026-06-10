// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AegisOdyssey/SkillSystem/Core/AOSkillExecutionDefinition.h"
#include "AOSkillExecutionDefinition_AreaSequence.generated.h"

/**
 * 区域序列执行定义。
 *
 * 适用于“先得到前方逻辑大圆，再按波次在其中逐次打出小范围命中”的执行链。
 * 它和其它执行定义分文件存放，避免未来继续往同一个总头文件里堆类型。
 */
UCLASS(BlueprintType, Blueprintable, DefaultToInstanced, EditInlineNew)
class AEGISODYSSEY_API UAOSkillAreaSequenceExecutionDefinition : public UAOSkillExecutionDefinition
{
	GENERATED_BODY()

public:
	UAOSkillAreaSequenceExecutionDefinition(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// 两次波次之间的时间间隔。由具体技能 Ability 按这个节奏驱动。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Area Sequence")
	FScalableFloat WaveInterval = FScalableFloat(0.25f);

	// 总共打出多少次局部喷发/命中波次。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Area Sequence", meta = (ClampMin = "1", UIMin = "1"))
	int32 WaveCount = 1;

	// 逻辑大圆的圆心相对角色向前推多远。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Area Sequence")
	FScalableFloat AreaCenterForwardDistance = FScalableFloat(250.0f);

	// 逻辑大圆半径。每一波的小范围落点都从这个大圆内部随机出来。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Area Sequence")
	FScalableFloat AreaRadius = FScalableFloat(400.0f);

	// 单次局部喷发的真实伤害半径。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Area Sequence")
	FScalableFloat ImpactRadius = FScalableFloat(120.0f);

	// 对大圆圆心再做一次局部偏移，便于做非完全正前方的技能。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Area Sequence")
	FVector AreaCenterOffset = FVector::ZeroVector;
};
