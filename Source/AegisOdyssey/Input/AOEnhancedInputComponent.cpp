// Fill out your copyright notice in the Description page of Project Settings.


#include "AOEnhancedInputComponent.h"

void UAOEnhancedInputComponent::AddInputMappings(const UAOInputConfig* InputConfig,
                                                 UEnhancedInputLocalPlayerSubsystem* InputSubsystem) const
{
	// 此方法已过时，输入映射上下文已在AOHeroComponent中通过AddMappingContext添加
	// 保留空实现以保持接口兼容性
}

void UAOEnhancedInputComponent::RemoveInputMappings(const UAOInputConfig* InputConfig,
                                                    UEnhancedInputLocalPlayerSubsystem* InputSubsystem) const
{
	// 此方法已过时，输入映射上下文移除应在调用处通过RemoveMappingContext处理
	// 保留空实现以保持接口兼容性
}
