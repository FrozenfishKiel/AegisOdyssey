// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "AOHarvestWindow.generated.h"

// 采集命中窗是采集动作的正式结算入口。
// 只要蒙太奇挂了这个 NotifyState，GA_Harvest 就会在窗口打开时提交一次采集命中；没挂这个窗，挥击只会播动画，不会采集成功。
UCLASS(DisplayName = "HarvestWindow")
class AEGISODYSSEY_API UAOHarvestWindow : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	UAOHarvestWindow();

protected:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
