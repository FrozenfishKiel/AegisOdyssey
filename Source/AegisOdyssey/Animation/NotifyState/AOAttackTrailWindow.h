#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "AOAttackTrailWindow.generated.h"

/**
 * 只负责刀光显示时长的 NotifyState。
 * 这层不再承担武器判定窗口语义，只负责派发攻击表现 Begin/End。
 */
UCLASS(DisplayName = "AttackTrailWindow")
class AEGISODYSSEY_API UAOAttackTrailWindow : public UAnimNotifyState
{
	GENERATED_BODY()

protected:
	virtual void NotifyBegin(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		float TotalDuration,
		const FAnimNotifyEventReference& EventReference) override;

	virtual void NotifyEnd(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;
};
