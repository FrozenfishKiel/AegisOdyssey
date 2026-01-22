// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "AOGameplayAbility.generated.h"

class UAOHeroComponent;
class AAOCharacter;
/**
 * 
 */
UENUM(BlueprintType)
enum class EAOAbilityActivationPolicy : uint8
{
	//尝试在输入被触发时激活该能力
	OnInputTriggered,

	//在输入激活时不断尝试激活该能力
	WhileInputActive,

	//尝试激活该能力当Avatar被分配时
	OnSpawn
	
};
/** 失败时可以用来播放动画集锦的原因 */
USTRUCT(BlueprintType)
struct FAOAbilityMontageFailureMessage
{
	GENERATED_BODY()

public:
	// Player controller that failed to activate the ability, if the AbilitySystemComponent was player owned
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<APlayerController> PlayerController = nullptr;

	// Avatar actor that failed to activate the ability
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<AActor> AvatarActor = nullptr;

	// All the reasons why this ability has failed
	UPROPERTY(BlueprintReadWrite)
	FGameplayTagContainer FailureTags;  //当前配置的Tag组

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<UAnimMontage> FailureMontage = nullptr;  //当前GA配置的动画蒙太奇
};
class AAOPlayerController;
UCLASS()
class AEGISODYSSEY_API UAOGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()
	friend class UAOAbilitySystem;
public:

	UAOGameplayAbility(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintCallable, Category = "AO|Ability")
	UAOAbilitySystem* GetAOAbilitySystem() const;

	UFUNCTION(BlueprintCallable , Category = "AO|Ability")
	AAOPlayerController* GetAOPlayerControllerFromActorInfo() const;

	UFUNCTION(BlueprintCallable, Category = "AO|Ability")
	AController* GetControllerFromActorInfo() const;

	UFUNCTION(BlueprintCallable, Category = "AO|Ability")
	AAOCharacter* GetLyraCharacterFromActorInfo() const;

	UFUNCTION(BlueprintCallable, Category = "AO|Ability")
	UAOHeroComponent* GetHeroComponentFromActorInfo() const;

	UFUNCTION(BlueprintCallable, Category = "AO|Ability")
	EAOAbilityActivationPolicy GetActivationPolicy() const {return ActivationPolicy;}

	void TryActivateAbilityOnSpawn(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) const;
protected:
	virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
protected:
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly, Category = "AO|Ability Activation")
	EAOAbilityActivationPolicy ActivationPolicy;

};
