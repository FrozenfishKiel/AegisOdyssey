// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/GameFrameworkInitStateInterface.h"
#include "Components/PawnComponent.h"
#include "AOHeroComponent.generated.h"

/**
 * 
 */
struct FInputMappingContextAndPriority;
struct FInputActionValue;
UCLASS(Blueprintable , meta = (BlueprintSpawnableComponent))
class AEGISODYSSEY_API UAOHeroComponent : public UPawnComponent , public IGameFrameworkInitStateInterface
{
	GENERATED_BODY()
public:
	UAOHeroComponent(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintPure,Category = "AOHero")
	static UAOHeroComponent* FindHeroComponent(const AActor* Actor){return (Actor ? Actor->FindComponentByClass<UAOHeroComponent>() : nullptr);}
	static const FName NAME_ActorFeatureName;
	static const FName NAME_BindInputsNow;
	virtual FName GetFeatureName() const override { return NAME_ActorFeatureName; }
	virtual bool CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) const override;
	virtual void HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) override;
	virtual void OnActorInitStateChanged(const FActorInitStateChangedParams& Params) override;

	virtual void CheckDefaultInitialization() override;
protected:
	virtual void OnRegister() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void Input_AbilityInputTagPressed(FGameplayTag InputTag);
	void Input_AbilityInputTagReleased(FGameplayTag InputTag);

	void Input_Move(const FInputActionValue& InputActionValue);
	void LookUp(const struct FInputActionValue&InputActionValue);

	virtual void InitializePlayerInput(UInputComponent* PlayerInputComponent);
protected:
	UPROPERTY(EditDefaultsOnly , Category="Config")
	TArray<FInputMappingContextAndPriority> DefaultInputMappings;
};

