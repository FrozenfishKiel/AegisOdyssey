// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "AegisOdyssey/AbilitySystem/Abilities/AOGameplayAbility.h"
#include "AegisOdyssey/Character/AOCombatHitPolicy.h"
#include "AegisOdyssey/Character/AOHeroComponent.h"
#include "GameplayAbilitySpec.h"
#include "GA_HeavyAttack.generated.h"

class UAOWeaponDefinition;
class UAbilityTask_PlayMontageAndWait;
class UAT_WaitMovementInput;
class UAT_WaitRotateToDirection;
class UAOWeaponInstance;
class USceneComponent;

USTRUCT(BlueprintType)
struct FHeavyAttackTargetData : public FGameplayAbilityTargetData
{
	GENERATED_BODY()

	FHeavyAttackTargetData()
		: InputTag(FGameplayTag::EmptyTag)
		, InputType(EInputType::None)
		, PlayRate(1.0f)
		, StartSection(NAME_None)
		, StartTime(0.0f)
	{
	}

	UPROPERTY(BlueprintReadWrite, Category = "HeavyAttack")
	FGameplayTag InputTag;

	UPROPERTY(BlueprintReadWrite, Category = "HeavyAttack")
	TEnumAsByte<EInputType> InputType;

	UPROPERTY(BlueprintReadWrite, Category = "HeavyAttack")
	TSoftObjectPtr<UAnimMontage> Montage;

	UPROPERTY(BlueprintReadWrite, Category = "HeavyAttack")
	float PlayRate;

	UPROPERTY(BlueprintReadWrite, Category = "HeavyAttack")
	FName StartSection;

	UPROPERTY(BlueprintReadWrite, Category = "HeavyAttack")
	float StartTime;

	UPROPERTY()
	TObjectPtr<const UAOWeaponInstance> DataWeaponInstance;

	virtual UScriptStruct* GetScriptStruct() const override
	{
		return FHeavyAttackTargetData::StaticStruct();
	}

	virtual FString ToString() const override
	{
		return FString::Printf(TEXT("FHeavyAttackTargetData: InputTag=%s, Montage=%s, PlayRate=%.2f"),
			*InputTag.ToString(), *Montage.ToString(), PlayRate);
	}

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

template<>
struct TStructOpsTypeTraits<FHeavyAttackTargetData> : public TStructOpsTypeTraitsBase2<FHeavyAttackTargetData>
{
	enum
	{
		WithNetSerializer = true,
	};
};

UCLASS()
class AEGISODYSSEY_API UGA_HeavyAttack : public UAOGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_HeavyAttack(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr,
		FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
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
	TObjectPtr<class UAT_WaitHeavyAttackCombatHit> WaitCombatHit;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	FGameplayTag CancelAbilityTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rotation", meta = (AllowPrivateAccess = "true"))
	float RotationInterpSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HeavyAttack", meta = (AllowPrivateAccess = "true"))
	FGameplayTag InputTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HeavyAttack", meta = (AllowPrivateAccess = "true"))
	TEnumAsByte<EInputType> InputType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HeavyAttack", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimMontage> Montage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HeavyAttack", meta = (AllowPrivateAccess = "true"))
	float PlayRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HeavyAttack", meta = (AllowPrivateAccess = "true"))
	FName StartSection;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HeavyAttack", meta = (AllowPrivateAccess = "true"))
	float StartTime;

	UPROPERTY()
	TWeakObjectPtr<const UAOWeaponInstance> WeaponInstance;

};

UCLASS()
class AEGISODYSSEY_API UAT_WaitHeavyAttackCombatHit : public UAbilityTask
{
	GENERATED_BODY()

public:
	UAT_WaitHeavyAttackCombatHit(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void Activate() override;
	virtual void TickTask(float DeltaTime) override;
	virtual void OnDestroy(bool bInOwnerFinished) override;

	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
	static UAT_WaitHeavyAttackCombatHit* WaitCombatHit(UGA_HeavyAttack* OwningAbility, UAOWeaponInstance* WeaponInstance);

private:
	void ProcessHitsBatch();
	void BeginHitWindow();
	void EndHitWindow();

protected:
	UPROPERTY()
	TWeakObjectPtr<UAOWeaponInstance> Weapon;

	TWeakObjectPtr<UGA_HeavyAttack> HeavyAttack;

private:
	UPROPERTY()
	TArray<UStaticMeshComponent*> CacheStaticMeshComponents;

	UPROPERTY()
	TArray<USkeletalMeshComponent*> CacheSkeletalMeshComponents;
	TMap<TObjectKey<USceneComponent>, FVector> PreviousTraceStartByComponent;
	TMap<TObjectKey<USceneComponent>, FVector> PreviousTraceEndByComponent;

	TArray<FHitResult> HitResultPool;
	TArray<AActor*> UniqueTargetsPool;
	TMap<TObjectKey<AActor>, FHitResult> FirstHitResultByTargetPool;
	TSet<TObjectKey<AActor>> HitTargetsInActiveWindow;
	bool bWasHitWindowActiveLastTick = false;
};
