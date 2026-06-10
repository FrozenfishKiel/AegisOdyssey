// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AegisOdyssey/AbilitySystem/Abilities/AOGameplayAbility.h"
#include "AegisOdyssey/Character/AOHeroComponent.h"
#include "AegisOdyssey/StateTree/CombatStateTree/AOCombatStateTree.h"
#include "GA_Roll.generated.h"

class UAbilityTask_PlayMontageAndWait;

// 翻滚能力运行时携带的目标数据。
// 这里既保存输入语义，也保存本次翻滚要使用的八方向蒙太奇和方向向量，
// 这样玩家和 AI 都可以沿用同一套翻滚能力入口。
USTRUCT(BlueprintType)
struct FRollTargetData : public FGameplayAbilityTargetData
{
	GENERATED_BODY()

	FRollTargetData()
		: InputTag(FGameplayTag::EmptyTag)
		, InputType(EInputType::None)
		, PlayRate(1.0f)
		, StartSection(NAME_None)
		, StartTime(0.0f)
		, ExplicitDirection(FVector::ZeroVector)
		, MoveInputDirection(FVector::ForwardVector)
	{
	}

	UPROPERTY(EditAnywhere, Category = "Config")
	UAnimMontage* ForwardMontage;

	// 八方向翻滚资源。
	UPROPERTY(EditAnywhere, Category = "Config")
	UAnimMontage* BackwardMontage;

	UPROPERTY(EditAnywhere, Category = "Config")
	UAnimMontage* LeftMontage;

	UPROPERTY(EditAnywhere, Category = "Config")
	UAnimMontage* RightMontage;

	UPROPERTY(EditAnywhere, Category = "Config")
	UAnimMontage* ForwardLeftMontage;

	UPROPERTY(EditAnywhere, Category = "Config")
	UAnimMontage* ForwardRightMontage;

	UPROPERTY(EditAnywhere, Category = "Config")
	UAnimMontage* BackwardLeftMontage;

	UPROPERTY(EditAnywhere, Category = "Config")
	UAnimMontage* BackwardRightMontage;
	
	// 输入来源标签，方便上层区分这次翻滚是由哪种输入语义触发的。
	UPROPERTY(BlueprintReadWrite, Category = "LightAttack")
	FGameplayTag InputTag;

	// 输入类型。
	UPROPERTY(BlueprintReadWrite, Category = "LightAttack")
	TEnumAsByte<EInputType> InputType;

	// 如果上层已经显式指定具体蒙太奇，可以从这里直接带入。
	UPROPERTY(BlueprintReadWrite, Category = "LightAttack")
	TSoftObjectPtr<UAnimMontage> Montage;

	// 播放速度、起始段、起始时间，主要用于控制翻滚动作细节。
	UPROPERTY(BlueprintReadWrite, Category = "LightAttack")
	float PlayRate;

	UPROPERTY(BlueprintReadWrite, Category = "LightAttack")
	FName StartSection;

	UPROPERTY(BlueprintReadWrite, Category = "LightAttack")
	float StartTime;

	// AI 或外部逻辑显式传入的世界方向。
	// 当它有效时，本次翻滚方向优先按它来解释，而不是按玩家输入向量解释。
	UPROPERTY(BlueprintReadWrite, Category = "LightAttack")
	FVector ExplicitDirection;

	// 玩家当前的移动输入方向。
	UPROPERTY(BlueprintReadWrite, Category = "LightAttack")
	FVector MoveInputDirection;

	/**
	 * 获取脚本结构体类型
	 * @return 返回FLightAttackTargetData的静态结构体
	 */
	virtual UScriptStruct* GetScriptStruct() const override
	{
		return FRollTargetData::StaticStruct();
	}

	/**
	 * 转换为字符串表示
	 * @return 返回描述性的字符串
	 */
	virtual FString ToString() const override
	{
		return FString::Printf(TEXT("FLightAttackTargetData: InputTag=%s, Montage=%s, PlayRate=%.2f"), 
			*InputTag.ToString(), *Montage.ToString(), PlayRate);
	}

	/**
	 * 网络序列化
	 * 用于在客户端和服务器之间传输数据
	 * @param Ar 归档对象
	 * @param Map 包映射
	 * @param bOutSuccess 输出是否成功
	 * @return 是否成功序列化
	 */
	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
	{
		Ar << InputTag;
		Ar << InputType;
		Ar << Montage;
		Ar << PlayRate;
		Ar << StartSection;
		Ar << StartTime;
		Ar << ForwardMontage;
		Ar << BackwardMontage;
		Ar << ForwardLeftMontage;
		Ar << ForwardRightMontage;
		Ar << BackwardLeftMontage;
		Ar << BackwardRightMontage;
		Ar << RightMontage;
		Ar << LeftMontage;
		Ar << ExplicitDirection;
		Ar << MoveInputDirection;
		bOutSuccess = true;
		return true;
	}
};

template<>
struct TStructOpsTypeTraits<FRollTargetData> : public TStructOpsTypeTraitsBase2<FRollTargetData>
{
	enum
	{
		WithNetSerializer = true,
	};
};
UCLASS()
class AEGISODYSSEY_API UGA_Roll : public UAOGameplayAbility
{
	GENERATED_BODY()
protected:
	// 以下是标准 Ability 生命周期入口。
	// 这一版翻滚的关键增强点是：正式接入体力成本，并配合无敌帧窗口进入战斗系统语义。
	virtual void CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility) override;
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual bool CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	virtual void ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const override;
	virtual const FGameplayTagContainer* GetCooldownTags() const override;

	// 播放最终选定的翻滚蒙太奇。
	void PlayMontageAnimation();

	UFUNCTION()
	void OnMontageCompleted();
	UFUNCTION()
	void OnMontageBlendedOut();
	UFUNCTION()
	void OnMontageInterrupted();
	UFUNCTION()
	void OnMontageCancelled();
private:
	// 根据玩家输入或显式方向，在八方向翻滚蒙太奇中选出本次实际要播的那一个。
	void SelectDirectionalMontage();
private:
	// 翻滚朝向插值速度。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rotation", meta = (AllowPrivateAccess = "true"))
	float RotationInterpSpeed;

	// 当前用于等待蒙太奇结束的能力任务。
	UPROPERTY()
	TObjectPtr<UAbilityTask_PlayMontageAndWait> MontageTask;
	
	// 运行时缓存的输入与翻滚资源。
	UPROPERTY()
	FGameplayTag InputTag;

	UPROPERTY()
	TEnumAsByte<EInputType> InputType;

	// 本次最终选中的翻滚蒙太奇。
	UPROPERTY()
	class UAnimMontage* Montage;
	
	UPROPERTY()
	UAnimMontage* ForwardMontage;

	UPROPERTY()
	UAnimMontage* BackwardMontage;

	UPROPERTY()
	UAnimMontage* LeftMontage;

	UPROPERTY()
	UAnimMontage* RightMontage;

	UPROPERTY()
	UAnimMontage* ForwardLeftMontage;

	UPROPERTY()
	UAnimMontage* ForwardRightMontage;

	UPROPERTY()
	UAnimMontage* BackwardLeftMontage;

	UPROPERTY()
	UAnimMontage* BackwardRightMontage;

	// 蒙太奇播放速率。
	float PlayRate;

	// 蒙太奇起始 Section。
	FName StartSection;
	
	// 翻滚冷却时长。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cooldown", meta = (AllowPrivateAccess = "true"))
	float CooldownDuration;

	// 翻滚第一版正式接入体力消耗。
	// 这里先做成简单可调的固定消耗，后续如果要细分不同方向/不同翻滚类型，再从这里往外扩。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cost", meta = (AllowPrivateAccess = "true", ClampMin = "0.0", UIMin = "0.0"))
	float RollVigorCost = 12.0f;
	
	// 动作播放起始时间。
	float StartTime = 0.f;

	// 玩家输入方向缓存。
	UPROPERTY()
	FVector SavedMoveInputDirection;

	// 外部显式方向缓存，主要给 AI 或特殊逻辑复用。
	UPROPERTY()
	FVector SavedExplicitDirection = FVector::ZeroVector;
};
