// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AegisOdyssey/AbilitySystem/Abilities/AOGameplayAbility.h"
#include "AegisOdyssey/Character/AOHeroComponent.h"
#include "AegisOdyssey/StateTree/CombatStateTree/AOCombatStateTree.h"
#include "GA_Roll.generated.h"

class UAT_WaitRotateToDirection;
class UAbilityTask_PlayMontageAndWait;
/**
 * 
 */
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
		, MoveInputDirection(FVector::ForwardVector)
	{
	}

	UPROPERTY(EditAnywhere, Category = "Config")
	UAnimMontage* ForwardMontage;

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
	
	UPROPERTY(BlueprintReadWrite, Category = "LightAttack")
	FGameplayTag InputTag;

	UPROPERTY(BlueprintReadWrite, Category = "LightAttack")
	TEnumAsByte<EInputType> InputType;

	UPROPERTY(BlueprintReadWrite, Category = "LightAttack")
	TSoftObjectPtr<UAnimMontage> Montage;

	UPROPERTY(BlueprintReadWrite, Category = "LightAttack")
	float PlayRate;

	UPROPERTY(BlueprintReadWrite, Category = "LightAttack")
	FName StartSection;

	UPROPERTY(BlueprintReadWrite, Category = "LightAttack")
	float StartTime;

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
	virtual void CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility) override;
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	virtual void ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const override;
	virtual const FGameplayTagContainer* GetCooldownTags() const override;
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
	void SelectDirectionalMontage();
private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rotation", meta = (AllowPrivateAccess = "true"))
	float RotationInterpSpeed;
	
	UPROPERTY()
	TObjectPtr<UAT_WaitRotateToDirection> RotationTask;
	
	UPROPERTY()
	TObjectPtr<UAbilityTask_PlayMontageAndWait> MontageTask;
	
	UPROPERTY()
	FGameplayTag InputTag;

	UPROPERTY()
	TEnumAsByte<EInputType> InputType;
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

	float PlayRate;

	FName StartSection;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cooldown", meta = (AllowPrivateAccess = "true"))
	float CooldownDuration;
	
	float StartTime = 0.f;

	UPROPERTY()
	FVector SavedMoveInputDirection;
};
