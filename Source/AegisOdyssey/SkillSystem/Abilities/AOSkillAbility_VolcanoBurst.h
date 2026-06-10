// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "AegisOdyssey/SkillSystem/Execution/AbilityBases/AOSkillGameplayAbility_AreaSequenceBase.h"
#include "AOSkillAbility_VolcanoBurst.generated.h"

class UAbilityTask_PlayMontageAndWait;
class UAbilityTask_WaitGameplayEvent;
class UAT_WaitMovementInput;
class UAnimMontage;
class UAOEquipmentInstance;
class AAOSkillAreaSequenceRuntime_VolcanoBurst;

struct FTimerHandle;

/**
 * 火山喷发技能 Ability。
 *
 * 这条链和火球术不一样：
 * 1. 它不生成投射体
 * 2. 它会在角色前方逻辑大圆内按波次推进
 * 3. 每一波都会随机一个小范围落点并立刻结算
 *
 * 更重要的是，动画与 GameplayEvent 也只由它自己负责，
 * 父类不再替它默认播放动画、等待事件或决定何时开始执行。
 */
UCLASS()
class AEGISODYSSEY_API UAOSkillAbility_VolcanoBurst : public UAOSkillGameplayAbility_AreaSequenceBase
{
	GENERATED_BODY()

public:
	UAOSkillAbility_VolcanoBurst(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

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

private:
	// 只属于火山喷发自己的动画资源。
	// 不是所有技能都会播动画，所以不放到技能父类，也不塞回 SkillDefinition。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AO|Volcano Burst|Animation", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimMontage> StartMontage = nullptr;

	// 由策划配置“哪一个动画事件标签代表正式开始喷发”。
	// 等待几个事件、等到之后做什么，仍然写死在本技能代码里。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AO|Volcano Burst|Animation", meta = (Categories = "GameplayEvent", AllowPrivateAccess = "true"))
	FGameplayTag StartSequenceEventTag;

	// 对齐普攻/重攻击的“后摇可脱手”语义：
	// 只有当角色身上已经被 AnimNotifyState 挂上这个可取消标签后，
	// 按移动输入才会让火山喷发 Ability 提前收口。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AO|Volcano Burst|Flow", meta = (AllowPrivateAccess = "true"))
	FGameplayTag CancelAbilityTag;

	// 允许技能自己指定“生成哪一个 Volcano Burst runtime 类”。
	// 默认仍然可以是当前 C++ 类，但策划/程序也可以换成 BP 子类去配置特效资源。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AO|Volcano Burst|Runtime", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AAOSkillAreaSequenceRuntime_VolcanoBurst> RuntimeActorClass;

private:
	// 进入真正喷发阶段并启动独立 runtime。
	// 到了这一步，后续长生命周期不再留在 Ability 里。
	void StartWaveSequence();

	// 生成并启动独立喷发执行体。
	// 这个执行体会接管剩余波次、命中结算和自清理。
	bool SpawnAndStartRuntimeSequence();

	// 火山喷发自己管理自己的动画任务。
	void StartMontageTaskIfNeeded();
	bool StartSequenceEventWaitIfNeeded();
	void ClearMontageTask();
	void ClearSequenceEventTask();
	void StartMovementInputTaskIfNeeded();
	void ClearMovementInputTask();
	void ClearWaveTimers();
	void HideCurrentWeaponIfNeeded();
	void RestoreHiddenWeaponIfNeeded();

	UFUNCTION()
	void HandleStartSequenceEventReceived(FGameplayEventData Payload);

	UFUNCTION()
	void HandleStartMontageCompleted();

	UFUNCTION()
	void HandleStartMontageBlendedOut();

	UFUNCTION()
	void HandleStartMontageInterrupted();

	UFUNCTION()
	void HandleStartMontageCancelled();

	UFUNCTION()
	void HandleMovementInputDetected();

	void FinishAbilityAfterMontage(bool bWasCancelled);
	void FinishAbilityAfterRuntimeHandoff();

private:
	// 防止动画事件和 fallback 路径重复启动整条喷发阶段。
	UPROPERTY()
	bool bWaveSequenceStarted = false;

	// 火山喷发自己的动画任务。
	UPROPERTY()
	TObjectPtr<UAbilityTask_PlayMontageAndWait> StartMontageTask;

	// 火山喷发自己的 GameplayEvent 等待任务。
	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitGameplayEvent> StartSequenceEventTask;

	// 复用普攻同一套“检测到移动输入后尝试脱手”的能力任务。
	// 是否真的结束 Ability，不由任务本身决定，而是看 CancelAbilityTag 是否已经进入角色身上。
	UPROPERTY()
	TObjectPtr<UAT_WaitMovementInput> MovementInputTask;

	// fallback 启动定时器。
	FTimerHandle SequenceStartTimerHandle;

	// 弱引用避免技能在执行期间额外延长武器实例生命周期。
	// 当前语义只覆盖施法起手阶段，不覆盖 runtime 全程。
	TWeakObjectPtr<UAOEquipmentInstance> HiddenWeaponInstance;

	// 运行时喷发执行体。弱引用即可，避免技能结束后额外续命。
	TWeakObjectPtr<AAOSkillAreaSequenceRuntime_VolcanoBurst> ActiveRuntimeSequence;
};
