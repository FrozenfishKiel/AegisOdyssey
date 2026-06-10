// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AegisOdyssey/AbilitySystem/Abilities/AOGameplayAbility.h"
#include "AegisOdyssey/Character/AOCombatHitPolicy.h"
#include "AegisOdyssey/Character/AOHeroComponent.h"
#include "GameplayAbilitySpec.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "GA_LightAttack.generated.h"

class UAOWeaponDefinition;
class UAbilityTask;
class UAOWeaponInstance;
class UAbilityTask_PlayMontageAndWait;
class UAT_WaitMovementInput;
class UAT_WaitRotateToDirection;
class UAT_WaitCombatHit;
class USceneComponent;
/**
 * 轻攻击参数对象
 * 通过GameplayEvent的OptionalObject传递给GA_LightAttack
 */

/**
 * 轻攻击目标数据
 * 用于网络传输轻攻击参数（支持自动复制到服务器）
 */
USTRUCT(BlueprintType)
struct FLightAttackTargetData : public FGameplayAbilityTargetData
{
	GENERATED_BODY()

	FLightAttackTargetData()
		: InputTag(FGameplayTag::EmptyTag)
		, InputType(EInputType::None)
		, PlayRate(1.0f)
		, StartSection(NAME_None)
		, StartTime(0.0f)
	{
	}

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
	
	UPROPERTY()
	TObjectPtr<const UAOWeaponInstance> DataWeaponInstance;

	/**
	 * 获取脚本结构体类型
	 * @return 返回FLightAttackTargetData的静态结构体
	 */
	virtual UScriptStruct* GetScriptStruct() const override
	{
		return FLightAttackTargetData::StaticStruct();
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
		Ar << DataWeaponInstance;
		bOutSuccess = true;
		return true;
	}
};

/**
 * 轻攻击目标数据的类型特征
 * 启用网络序列化支持
 */
template<>
struct TStructOpsTypeTraits<FLightAttackTargetData> : public TStructOpsTypeTraitsBase2<FLightAttackTargetData>
{
	enum
	{
		WithNetSerializer = true,
	};
};

/**
 * 
 */
UCLASS()
class AEGISODYSSEY_API UGA_LightAttack : public UAOGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_LightAttack(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	UPROPERTY(EditAnywhere , BlueprintReadOnly)
	FGameplayTag ListenAttackActionTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	FGameplayTag AttackTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	FAOCombatHitPolicy HitPolicy;
protected:
	UFUNCTION()
	void PlayMontageAnimation();
	UFUNCTION()
	void OnMontageCompleted();
	UFUNCTION()
	void OnMontageBlendedOut();
	UFUNCTION()
	void OnMontageInterrupted();
	UFUNCTION()
	void OnMontageCancelled();
	UFUNCTION()
	void OnMovementInputDetected();

	UFUNCTION()
	void ClearCombatTags();

private:
	void GetCombatWindowTagsFromMontage(UAnimMontage* InMontage, TArray<FGameplayTag>& OutTags);
	void SetCharacterRotationToAttackDirection();
	UAOWeaponInstance* GetAOWeaponInstance() const;
	UAOWeaponDefinition* GetAOWeaponDefinition() const;

	UPROPERTY()
	TObjectPtr<UAbilityTask_PlayMontageAndWait> MontageTask;

	UPROPERTY()
	TObjectPtr<UAT_WaitMovementInput> MovementInputTask;

	UPROPERTY()
	TObjectPtr<UAT_WaitRotateToDirection> RotationTask;
	
	UPROPERTY()
	TObjectPtr<UAT_WaitCombatHit> WaitCombatHit;

	UPROPERTY(EditDefaultsOnly , BlueprintReadOnly , meta = (AllowPrivateAccess = "true"))
	FGameplayTag CancelAbilityTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rotation", meta = (AllowPrivateAccess = "true"))
	float RotationInterpSpeed;
	



	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LightAttack",meta = (AllowPrivateAccess = "true"))
	FGameplayTag InputTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LightAttack", meta = (AllowPrivateAccess = "true"))
	TEnumAsByte<EInputType> InputType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LightAttack", meta = (AllowPrivateAccess = "true"))
	class UAnimMontage* Montage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LightAttack", meta = (AllowPrivateAccess = "true"))
	float PlayRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LightAttack", meta = (AllowPrivateAccess = "true"))
	FName StartSection;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LightAttack", meta = (AllowPrivateAccess = "true"))
	float StartTime;
	
	UPROPERTY()
	TWeakObjectPtr<const UAOWeaponInstance> WeaponInstance;

};


UCLASS()
class AEGISODYSSEY_API UAT_WaitCombatHit : public UAbilityTask
{
	GENERATED_BODY()
public:
	virtual void Activate() override;
	virtual void TickTask(float DeltaTime) override;
	virtual void OnDestroy(bool bInOwnerFinished) override;
	
	UAT_WaitCombatHit(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	UFUNCTION(BlueprintCallable,Category = "Ability|Tasks",meta = (HidePin = "OwningAbility",DefaultToSelf = "OwningAbility" , BlueprintInternalUseOnly = "TRUE"))
	static UAT_WaitCombatHit* WaitCombatHit(UGA_LightAttack* OwningAbility,UAOWeaponInstance* WeaponInstance);
	
private:
	// 批量处理命中结果
	void ProcessHitsBatch();
	void BeginHitWindow();
	void EndHitWindow();
protected:
	UPROPERTY()
	TWeakObjectPtr<UAOWeaponInstance> Weapon;
	TWeakObjectPtr<UGA_LightAttack> LightAttack;
private:
	UPROPERTY()
	TArray<UStaticMeshComponent*> CacheStaticMeshComponents;
	UPROPERTY()
	TArray<USkeletalMeshComponent*> CacheSkeletalMeshComponents;
	TMap<TObjectKey<USceneComponent>, FVector> PreviousTraceStartByComponent;
	TMap<TObjectKey<USceneComponent>, FVector> PreviousTraceEndByComponent;

	// 预分配的命中结果数组，用于批量处理
	TArray<FHitResult> HitResultPool;
	// 预分配的目标数组，用于去重
	TArray<AActor*> UniqueTargetsPool;
	TMap<TObjectKey<AActor>, FHitResult> FirstHitResultByTargetPool;
	TSet<TObjectKey<AActor>> HitTargetsInActiveWindow;
	bool bWasHitWindowActiveLastTick = false;
};
