// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "GameplayTagContainer.h"
#include "AOCombatWindow.generated.h"

/**
 * 动作时间窗口判定
 * 用于标记动画中的连招窗口，在窗口内可以输入下一个攻击
 */
UCLASS(DisplayName = "CombatWindow")
class AEGISODYSSEY_API UAOCombatWindow : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	UAOCombatWindow();

protected:
	/**
	 * 进入连招窗口时调用
	 * @param MeshComp 骨骼网格组件
	 * @param Animation 动画序列
	 * @param TotalDuration 窗口总持续时间
	 * @param EventReference 事件引用
	 */
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;

	/**
	 * 离开连招窗口时调用
	 * @param MeshComp 骨骼网格组件
	 * @param Animation 动画序列
	 * @param EventReference 事件引用
	 */
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

private:
	/** 连招窗口标签 */
	UPROPERTY(EditDefaultsOnly , meta = (AllowPrivateAccess))
	FGameplayTag CombatWindowTag;
};
