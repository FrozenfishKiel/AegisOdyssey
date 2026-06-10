// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AegisOdyssey/AbilitySystem/Abilities/AOGameplayAbility.h"
#include "GameplayTagContainer.h"
#include "GA_HitReact.generated.h"

class UAbilityTask_PlayMontageAndWait;
class UAbilityTask_WaitGameplayEvent;

UENUM()
enum class EAOHitReactDirection : uint8
{
	Forward,
	Backward,
	Left,
	Right,
	ForwardLeft,
	ForwardRight,
	BackwardLeft,
	BackwardRight
};

USTRUCT(BlueprintType)
struct FAOHitReactMontageList
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HitReact", meta = (AllowPrivateAccess = "true"))
	TArray<TSoftObjectPtr<UAnimMontage>> Montages;
};

USTRUCT(BlueprintType)
struct FAOHitReactMontagePoolSet
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HitReact", meta = (AllowPrivateAccess = "true"))
	TMap<EAOHitReactDirection, FAOHitReactMontageList> DirectionPools;
};

USTRUCT(BlueprintType)
struct FHitReactTargetData : public FGameplayAbilityTargetData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HitReact")
	FGameplayTag StateTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HitReact")
	FVector SourceDirection = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HitReact")
	int32 VariantIndex = INDEX_NONE;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HitReact")
	TSoftObjectPtr<UAnimMontage> Montage;

	virtual UScriptStruct* GetScriptStruct() const override
	{
		return FHitReactTargetData::StaticStruct();
	}

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
	{
		Ar << StateTag;
		Ar << SourceDirection;
		Ar << VariantIndex;
		Ar << Montage;
		bOutSuccess = true;
		return true;
	}
};

template<>
struct TStructOpsTypeTraits<FHitReactTargetData> : public TStructOpsTypeTraitsBase2<FHitReactTargetData>
{
	enum
	{
		WithNetSerializer = true,
	};
};

UCLASS()
class AEGISODYSSEY_API UGA_HitReact : public UAOGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_HitReact(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual bool CanActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayTagContainer* SourceTags = nullptr,
		const FGameplayTagContainer* TargetTags = nullptr,
		FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

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
	void OnAllowMoveEvent(FGameplayEventData Payload);

	UFUNCTION()
	void OnFinishEvent(FGameplayEventData Payload);

private:
	void PlayMontageAnimation();

	EAOHitReactDirection ResolveHitReactDirection(const FVector& SourceDirection) const;

	const FAOHitReactMontagePoolSet* FindMontagePoolsByStateTag(const FGameplayTag& StateTag) const;

	const TArray<TSoftObjectPtr<UAnimMontage>>* FindMontagePool(
		const FAOHitReactMontagePoolSet& Pools,
		EAOHitReactDirection Direction) const;

public:
	bool TryResolveHitReactMontage(
		const FGameplayTag& StateTag,
		const FVector& SourceDirection,
		TSoftObjectPtr<UAnimMontage>& OutMontage) const;
	void ReleaseMoveLock() const;

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HitReact|Pools", meta = (AllowPrivateAccess = "true"))
	TMap<FGameplayTag, FAOHitReactMontagePoolSet> MontagePoolsByStateTag;

	UPROPERTY()
	TObjectPtr<UAbilityTask_PlayMontageAndWait> MontageTask;

	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitGameplayEvent> WaitAllowMoveTask;

	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitGameplayEvent> WaitFinishTask;

	UPROPERTY()
	TObjectPtr<UAnimMontage> Montage = nullptr;
};
