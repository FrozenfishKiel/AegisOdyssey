#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "CombatInterface.generated.h"

class UAbilitySystemComponent;
class UGameplayEffect;

USTRUCT()
struct FAttackedInfo
{
	GENERATED_BODY()

	UPROPERTY()
	TWeakObjectPtr<UAbilitySystemComponent> SourceASC = nullptr;

	UPROPERTY()
	TWeakObjectPtr<UAbilitySystemComponent> TargetASC = nullptr;

	// 统一攻击来源对象。
	// 例如技能实例、武器实例等，后续战斗系统需要能稳定识别“这次攻击来自什么资源”。
	UPROPERTY()
	TWeakObjectPtr<UObject> SourceObject = nullptr;

	// 真正把命中送入战斗系统的对象。
	// 近战通常是角色自身，投射体或区域 runtime 则应带它们自己的 Actor。
	UPROPERTY()
	TWeakObjectPtr<AActor> EffectCauser = nullptr;

	// 第一阶段统一攻击来源字段。
	UPROPERTY()
	FGameplayTag AttackTag;

	UPROPERTY()
	FGameplayTag SkillTag;

	UPROPERTY()
	FGameplayTag WeaponTag;

	UPROPERTY()
	FGameplayTagContainer DamageTypeTags;

	// 当前命中如果有可靠的 HitResult，就顺着主链带进 EffectContext。
	UPROPERTY()
	FHitResult HitResult;

	// 统一复用现有战斗尾链应用的 Meta 效果。
	UPROPERTY()
	TArray<TSubclassOf<UGameplayEffect>> MetaGameplayEffects;
};

UINTERFACE()
class UCombatInterface : public UInterface
{
	GENERATED_BODY()
};

class ICombatInterface
{
	GENERATED_BODY()

public:
	virtual void ApplyDamageToTarget(const FAttackedInfo& AttackedInfo) = 0;
};
