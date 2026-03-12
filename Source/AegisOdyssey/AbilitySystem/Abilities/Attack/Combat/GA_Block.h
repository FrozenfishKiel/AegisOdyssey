// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AegisOdyssey/AbilitySystem/Abilities/AOGameplayAbility.h"
#include "AegisOdyssey/Character/AOHeroComponent.h"
#include "GameplayAbilitySpec.h"
#include "GA_Block.generated.h"

class UAT_WaitRotateToDirection;
/**
 * 角色格挡/弹反技能
 * 长按激活，自动播放三段动画：
 * - 播放开始格挡动画（抬手防御，可弹反）
 * - 开始格挡动画结束后，自动切换到持续格挡动画（循环播放）
 * - 期间松手，播放取消防御动画并结束能力
 */
class UAbilityTask_PlayMontageAndWait;
class UAbilityTask_WaitGameplayEvent;
class UAbilityTask_WaitInputRelease;

USTRUCT(BlueprintType)
struct FBlockTargetData : public FGameplayAbilityTargetData
{
	GENERATED_BODY()

	FBlockTargetData()
		: InputTag(FGameplayTag::EmptyTag)
		, InputType(EInputType::None)
		, PlayRate(1.0f)
		, StartSection(NAME_None)
		, StartTime(0.0f)
	{
	}

	UPROPERTY(BlueprintReadWrite, Category = "Block")
	FGameplayTag InputTag;

	UPROPERTY(BlueprintReadWrite, Category = "Block")
	TEnumAsByte<EInputType> InputType;

	UPROPERTY(BlueprintReadWrite, Category = "Block")
	TSoftObjectPtr<UAnimMontage> StartBlockMontage;

	UPROPERTY(BlueprintReadWrite, Category = "Block")
	TSoftObjectPtr<UAnimMontage> LoopBlockMontage;

	UPROPERTY(BlueprintReadWrite, Category = "Block")
	TSoftObjectPtr<UAnimMontage> EndBlockMontage;

	UPROPERTY(BlueprintReadWrite, Category = "Block")
	float PlayRate;

	UPROPERTY(BlueprintReadWrite, Category = "Block")
	FName StartSection;

	UPROPERTY(BlueprintReadWrite, Category = "Block")
	float StartTime;

	/**
	 * 获取脚本结构体类型
	 * @return 返回FLightAttackTargetData的静态结构体
	 */
	virtual UScriptStruct* GetScriptStruct() const override
	{
		return FBlockTargetData::StaticStruct();
	}

	/**
	 * 转换为字符串表示
	 * @return 返回描述性的字符串
	 */

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
		Ar << StartBlockMontage;
		Ar << LoopBlockMontage;
		Ar << EndBlockMontage;
		Ar << PlayRate;
		Ar << StartSection;
		Ar << StartTime;
		bOutSuccess = true;
		return true;
	}
};

/**
 * 轻攻击目标数据的类型特征
 * 启用网络序列化支持
 */
template<>
struct TStructOpsTypeTraits<FBlockTargetData> : public TStructOpsTypeTraitsBase2<FBlockTargetData>
{
	enum
	{
		WithNetSerializer = true,
	};
};
UCLASS()
class AEGISODYSSEY_API UGA_Block : public UAOGameplayAbility
{
	GENERATED_BODY()
	UGA_Block(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	float SprintSpeedBonusAmount = 0.f;
protected:
	UFUNCTION()
	void OnMontageCompleted();
	UFUNCTION()
	void OnMontageBlendedOut();
	UFUNCTION()
	void OnMontageInterrupted();
	UFUNCTION()
	void OnMontageCancelled();
	UFUNCTION()
	void ClearCombatTags();
	UFUNCTION()
	void OnInputReleased(float TimeHeld);
private:
	void PlayStartBlockAnimation();
	void PlayLoopBlockAnimation();
	void PlayEndBlockAnimation();
	void GetCombatWindowTagsFromMontage(UAnimMontage* InMontage, TArray<FGameplayTag>& OutTags);
	void SetCharacterRotationToBlockDirection();

private:
	UPROPERTY()
	TObjectPtr<UAT_WaitRotateToDirection> RotationTask;
	UPROPERTY()
	TObjectPtr<UAbilityTask_PlayMontageAndWait> PlayEndBlockMontageTask;
	UPROPERTY()
	TObjectPtr<UAbilityTask_PlayMontageAndWait> PlayLoopBlockMontageTask;
	UPROPERTY()
	TObjectPtr<UAbilityTask_PlayMontageAndWait> PlayStartBlockMontageTask;
	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitGameplayEvent> WaitGameplayEventTask;
	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitInputRelease> WaitInputReleaseTask;

	FGameplayTag InputTag;
	TEnumAsByte<EInputType> InputType;

	UPROPERTY()
	class UAnimMontage* StartBlockMontage;

	UPROPERTY()
	class UAnimMontage* LoopBlockMontage;

	UPROPERTY()
	class UAnimMontage* EndBlockMontage;

	UPROPERTY()
	class UAnimMontage* Montage;
	float PlayRate = 0.f;
	FName StartSection;
	float StartTime = 0.f;
	FActiveGameplayEffectHandle SprintSpeedEffectHandle;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rotation", meta = (AllowPrivateAccess = "true"))
	float RotationInterpSpeed;
};
