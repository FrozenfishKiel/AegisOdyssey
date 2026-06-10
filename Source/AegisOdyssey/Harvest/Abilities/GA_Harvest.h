#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "AegisOdyssey/AbilitySystem/Abilities/AOGameplayAbility.h"
#include "AegisOdyssey/Harvest/Core/AOHarvestTypes.h"
#include "GA_Harvest.generated.h"

class UAbilityTask_PlayMontageAndWait;
class UAbilityTask_WaitInputRelease;
class UAnimMontage;
class UMeshComponent;
class UAOHarvestableDefinition;
struct FGameplayCueParameters;
struct FAOInventoryReceiveBatch;
struct FAOItemCatalogRow;

UCLASS()
class AEGISODYSSEY_API UGA_Harvest : public UAOGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_Harvest(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr,
		FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	UFUNCTION()
	void OnInputReleased(float TimeHeld);

	UFUNCTION()
	void OnMontageCompleted();

	UFUNCTION()
	void OnMontageBlendedOut();

	UFUNCTION()
	void OnMontageInterrupted();

	UFUNCTION()
	void OnMontageCancelled();

private:
	bool ExtractHarvestTargetData(const FGameplayEventData* TriggerEventData, FAOHarvestTargetData& OutTargetData) const;
	bool RebuildToolRuntimeContext(FAOHarvestRuntimeContext& InOutRuntimeContext) const;
	void PlayHarvestMontage();
	void ClearHarvestWindowTag();
	bool ResolveHarvestTargetFromHitContext(FAOHarvestHitContext& InOutHitContext) const;
	bool BuildHarvestHitContext(FAOHarvestHitContext& OutHitContext) const;
	bool ResolveHarvestTraceFromTool(FAOHarvestHitContext& InOutHitContext) const;
	bool TryResolveHarvestTraceFromMeshComponent(const UMeshComponent* MeshComponent, const FAOHarvestHitCheckConfig& HitCheckConfig, FVector& OutTraceStart, FVector& OutTraceEnd, FVector& OutFacingDirection) const;
	void DrawHarvestHitDebugPreview(const FAOHarvestHitContext& HitContext) const;
	void ShowHarvestProgressOnScreen(const FAOHarvestHitContext& HitContext, const FAOHarvestResult& HarvestResult) const;
	FGameplayCueParameters BuildHarvestCueParameters(const FAOHarvestHitContext& HitContext, const UAOHarvestableDefinition* HarvestableDefinition) const;
	void ExecuteHarvestCue(const FAOHarvestHitContext& HitContext, const FAOHarvestResult& HarvestResult) const;
	void TryProcessHarvestHitOnAuthority();
	void ExecuteHarvestHit();
	bool BuildRewardItemBatch(const FAOHarvestResult& HarvestResult, TArray<FAOItemCatalogRow>& OutItemRows, TArray<int32>& OutItemCounts) const;
	bool BuildRewardReceiveBatch(const FAOHarvestResult& HarvestResult, FAOInventoryReceiveBatch& OutReceiveBatch) const;
	bool TryCommitHarvestRewards(const FAOHarvestHitContext& HitContext, const FAOHarvestResult& HarvestResult);

protected:
	// 当前这次采集动作的运行时快照。
	// Ability 只在本次挥击生命周期内持有它，不要求角色常驻一个采集组件。
	UPROPERTY(Transient)
	FAOHarvestTargetData CurrentHarvestTargetData;

	UPROPERTY(Transient)
	TObjectPtr<UAbilityTask_WaitInputRelease> WaitInputReleaseTask = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UAbilityTask_PlayMontageAndWait> HarvestMontageTask = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<class UAT_WaitHarvestHit> WaitHarvestHitTask = nullptr;

	UPROPERTY(Transient)
	bool bHasValidHarvestContext = false;

	// 这些是状态树随本次激活一起下发的动作参数。
	// GA 不自己决定播哪段采集动画，只消费这次请求给出的动作配置。
	UPROPERTY(Transient)
	TObjectPtr<UAnimMontage> Montage = nullptr;

	UPROPERTY(Transient)
	float PlayRate = 1.0f;

	UPROPERTY(Transient)
	FName StartSection = NAME_None;

	UPROPERTY(Transient)
	float StartTime = 0.0f;

	// 调试开关只作用于当前这类采集 Ability。
	// 打开后会强制绘制本次采集挥击的预览与最终判定结果，不依赖全局 CVar。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Harvest|Debug", meta = (AllowPrivateAccess = "true"))
	bool bEnableHarvestDebugDraw = false;

	// 调试图保留多久。
	// 数值越大越容易看清每次挥击的轨迹和最终判定结果，但也更容易让多次挥击的图叠在一起。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Harvest|Debug", meta = (AllowPrivateAccess = "true", ClampMin = "0.0", UIMin = "0.0"))
	float HarvestDebugDrawDuration = 2.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Harvest|Debug", meta = (AllowPrivateAccess = "true"))
	bool bShowHarvestProgressOnScreen = true;

	// 这条 Tag 只表示“当前挥击是否进入采集命中窗”。
	// 它和战斗窗口概念平行，但刻意独立，避免拳头战斗与采集两套判定互相串窗。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Harvest", meta = (AllowPrivateAccess = "true"))
	FGameplayTag HarvestHitWindowTag;

	friend class UAT_WaitHarvestHit;
};

UCLASS()
class AEGISODYSSEY_API UAT_WaitHarvestHit : public UAbilityTask
{
	GENERATED_BODY()

public:
	UAT_WaitHarvestHit(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void Activate() override;
	virtual void TickTask(float DeltaTime) override;
	virtual void OnDestroy(bool bInOwnerFinished) override;

	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
	static UAT_WaitHarvestHit* WaitHarvestHit(UGA_Harvest* OwningAbility);

private:
	UPROPERTY()
	TWeakObjectPtr<UGA_Harvest> HarvestAbility;

	bool bWasHitWindowActiveLastTick = false;
	bool bSubmittedHitInCurrentWindow = false;
};
