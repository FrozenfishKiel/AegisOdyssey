// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "AOGameplayAbility.generated.h"

class UAOAbilitySystem;
class UAOHeroComponent;
class AAOCharacter;
class AAOPlayerController;

UENUM(BlueprintType)
enum class EAOAbilityActivationPolicy : uint8
{
	OnInputTriggered,
	WhileInputActive,
	OnSpawn,
	Start,
};

/** 能力播放失败时，用于对外广播失败上下文。 */
USTRUCT(BlueprintType)
struct FAOAbilityMontageFailureMessage
{
	GENERATED_BODY()

public:
	/** 如果能力系统由玩家拥有，则记录对应的玩家控制器。 */
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<APlayerController> PlayerController = nullptr;

	/** 触发失败的 Avatar Actor。 */
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<AActor> AvatarActor = nullptr;

	/** 本次失败命中的所有失败 Tag。 */
	UPROPERTY(BlueprintReadWrite)
	FGameplayTagContainer FailureTags;

	/** 对应的失败蒙太奇。 */
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<UAnimMontage> FailureMontage = nullptr;
};

UCLASS()
class AEGISODYSSEY_API UAOGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

	friend class UAOAbilitySystem;

public:
	UAOGameplayAbility(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintCallable, Category = "AO|Ability")
	UAOAbilitySystem* GetAOAbilitySystem() const;

	UFUNCTION(BlueprintCallable, Category = "AO|Ability")
	AAOPlayerController* GetAOPlayerControllerFromActorInfo() const;

	UFUNCTION(BlueprintCallable, Category = "AO|Ability")
	AController* GetControllerFromActorInfo() const;

	UFUNCTION(BlueprintCallable, Category = "AO|Ability")
	AAOCharacter* GetLyraCharacterFromActorInfo() const;

	UFUNCTION(BlueprintCallable, Category = "AO|Ability")
	UAOHeroComponent* GetHeroComponentFromActorInfo() const;

	UFUNCTION(BlueprintCallable, Category = "AO|Ability")
	EAOAbilityActivationPolicy GetActivationPolicy() const { return ActivationPolicy; }

	/** 当能力授予或 ActorInfo 刷新后，按策略尝试自动激活能力。 */
	void TryActivateAbilityOnSpawn(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) const;

protected:
	virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	/** 该能力的激活策略。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AO|Ability Activation")
	EAOAbilityActivationPolicy ActivationPolicy;
};
