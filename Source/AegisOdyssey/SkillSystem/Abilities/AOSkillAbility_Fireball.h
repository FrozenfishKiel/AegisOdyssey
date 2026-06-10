// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "AegisOdyssey/SkillSystem/Execution/AbilityBases/AOSkillGameplayAbility_ProjectileBase.h"
#include "AOSkillAbility_Fireball.generated.h"

class UAbilityTask_PlayMontageAndWait;
class UAbilityTask_WaitGameplayEvent;
class UAnimMontage;
class UAOEquipmentInstance;
class AAOSkillProjectile_Fireball;

/**
 * 火球术技能 Ability。
 *
 * 这一层负责把“施法动画 -> 动画事件决定真正发射 -> 投射体独立继续飞行”这条链打通。
 */
UCLASS()
class AEGISODYSSEY_API UAOSkillAbility_Fireball : public UAOSkillGameplayAbility_ProjectileBase
{
	GENERATED_BODY()

public:
	UAOSkillAbility_Fireball(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled) override;

	virtual TSubclassOf<AActor> GetProjectileActorClassToSpawn() const override;
	virtual void OnConfiguredProjectileSpawned(AActor* SpawnedProjectile) override;

private:
	void StartMontageTaskIfNeeded();
	bool StartSpawnProjectileEventWaitIfNeeded();
	void ClearMontageTask();
	void ClearSpawnProjectileEventTask();
	void HideCurrentWeaponIfNeeded();
	void RestoreHiddenWeaponIfNeeded();
	void SpawnProjectileFromAnimationEvent();
	void FinishAbilityAfterMontage(bool bWasCancelled);
	void FinishAbilityAfterProjectileHandoff();

	UFUNCTION()
	void HandleSpawnProjectileEventReceived(FGameplayEventData Payload);

	UFUNCTION()
	void HandleStartMontageCompleted();

	UFUNCTION()
	void HandleStartMontageBlendedOut();

	UFUNCTION()
	void HandleStartMontageInterrupted();

	UFUNCTION()
	void HandleStartMontageCancelled();

private:
	// 火球术自己的起手施法蒙太奇。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AO|Fireball|Animation", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimMontage> StartMontage = nullptr;

	// 动画里哪一个 GameplayEvent 代表“现在真正发射火球”。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AO|Fireball|Animation", meta = (Categories = "GameplayEvent", AllowPrivateAccess = "true"))
	FGameplayTag SpawnProjectileEventTag;

	// 和火山喷发一样，火球术也允许技能自己指定“生成哪一个运行时火球类”。
	// 这样后续可以直接在技能蓝图上切 BP 子类去配飞行 Niagara、命中 Niagara、材质或别的表现参数。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AO|Fireball|Runtime", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AAOSkillProjectile_Fireball> RuntimeProjectileClass;

	// 防止动画事件重复生成投射体。
	UPROPERTY()
	bool bProjectileSpawned = false;

	UPROPERTY()
	TObjectPtr<UAbilityTask_PlayMontageAndWait> StartMontageTask;

	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitGameplayEvent> SpawnProjectileEventTask;

	// 火球术在起手发射阶段同样可以临时隐藏武器，
	// 避免法球从角色手部附近生成时和当前手持武器穿帮。
	TWeakObjectPtr<UAOEquipmentInstance> HiddenWeaponInstance;
};
