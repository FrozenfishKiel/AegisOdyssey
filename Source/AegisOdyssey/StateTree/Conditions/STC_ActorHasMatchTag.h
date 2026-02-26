// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeConditionBase.h"
#include "STC_ActorHasMatchTag.generated.h"

/**
 * 检查Actor是否有匹配的GameplayTag
 * 可在状态树编辑器中直接编辑标签属性
 */
/**
 * 检查Actor是否有匹配的GameplayTag的实例数据
 */
USTRUCT()
struct FActorHasMatchTagInstanceData
{
	GENERATED_BODY()

	/** 需要检查的GameplayTag */
	UPROPERTY(EditAnywhere, Category = "Config")
	FGameplayTag InTag;
};

USTRUCT()
struct AEGISODYSSEY_API FSTC_ActorHasMatchTag : public FStateTreeConditionCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FActorHasMatchTagInstanceData;

public:
	/**
	 * 测试条件是否满足
	 * @param Context 状态树执行上下文
	 * @return 如果Actor有匹配的标签则返回true（如果bInvert为true则返回false）
	 */
	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;
	virtual const UStruct* GetInstanceDataType() const override {return FInstanceDataType::StaticStruct();}

	/** 是否取反结果 */
	UPROPERTY(EditAnywhere, Category = "Config")
	bool bInvert = false;
};
