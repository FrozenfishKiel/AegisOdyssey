#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Static.h"
#include "AOSkillGameplayCueNotify_Burst.generated.h"

class UNiagaraSystem;
class USoundBase;

USTRUCT(BlueprintType)
struct FAOSkillCueBurstEntry
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cue")
	TObjectPtr<UNiagaraSystem> NiagaraEffect = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cue")
	TObjectPtr<USoundBase> Sound = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cue")
	FVector LocationOffset = FVector::ZeroVector;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cue")
	FRotator RotationOffset = FRotator::ZeroRotator;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cue")
	FVector Scale = FVector(1.0f);
};

/**
 * 技能瞬时表现用的通用 GameplayCue。
 *
 * 这层只负责根据 CueParameters 在目标位置播放资源，
 * 不参与任何命中判定、伤害结算或波次推进。
 */
UCLASS(Blueprintable)
class AEGISODYSSEY_API UAOSkillGameplayCueNotify_Burst : public UGameplayCueNotify_Static
{
	GENERATED_BODY()

public:
	virtual bool OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const override;

protected:
	// 每个条目自己维护一组表现资源，避免多个特效和多个音效只能靠数组顺序硬对齐。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cue")
	TArray<FAOSkillCueBurstEntry> BurstEntries;
};
