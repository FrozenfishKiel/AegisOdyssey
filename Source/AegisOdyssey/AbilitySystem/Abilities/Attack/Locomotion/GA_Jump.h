// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AegisOdyssey/AbilitySystem/Abilities/AOGameplayAbility.h"
#include "AegisOdyssey/Character/AOHeroComponent.h"
#include "GameplayAbilitySpec.h"
#include "GA_Jump.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct FJumpTargetData : public FGameplayAbilityTargetData
{
	GENERATED_BODY()

	FJumpTargetData()
	{
	}

	/**
	 * 获取脚本结构体类型
	 * @return 返回FLightAttackTargetData的静态结构体
	 */
	virtual UScriptStruct* GetScriptStruct() const override
	{
		return FJumpTargetData::StaticStruct();
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
		bOutSuccess = true;
		return true;
	}
};

/**
 * 轻攻击目标数据的类型特征
 * 启用网络序列化支持
 */
template<>
struct TStructOpsTypeTraits<FJumpTargetData> : public TStructOpsTypeTraitsBase2<FJumpTargetData>
{
	enum
	{
		WithNetSerializer = true,
	};
};
UCLASS()
class AEGISODYSSEY_API UGA_Jump : public UAOGameplayAbility
{
	GENERATED_BODY()
public:
	UGA_Jump(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	virtual void CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility) override;

private:
	UFUNCTION()
	void OnMovementModeChanged(EMovementMode NewMovementMode);
	
	UFUNCTION()
	void OnJumpTimeout();
};
