// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/GameFrameworkInitStateInterface.h"
#include "Components/PawnComponent.h"
#include "AOHeroComponent.generated.h"

struct FInputMappingContextAndPriority;
struct FInputActionValue;
class UAOCameraMode;

UENUM()
enum EInputType : uint8
{
	None = 0,
	Trigger = 1,
	Start = 2,
	Release = 3,
};

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnPressInputLoad, const FGameplayTag, const EInputType);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnReleaseInputLoad, const FGameplayTag, const EInputType);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnStartInputLoad, const FGameplayTag, const EInputType);

UCLASS(Blueprintable, meta = (BlueprintSpawnableComponent))
class AEGISODYSSEY_API UAOHeroComponent : public UPawnComponent, public IGameFrameworkInitStateInterface
{
	GENERATED_BODY()

public:
	UAOHeroComponent(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintPure, Category = "AOHero")
	static UAOHeroComponent* FindHeroComponent(const AActor* Actor)
	{
		return (Actor ? Actor->FindComponentByClass<UAOHeroComponent>() : nullptr);
	}

	static const FName NAME_ActorFeatureName;
	static const FName NAME_BindInputsNow;

	virtual FName GetFeatureName() const override { return NAME_ActorFeatureName; }
	virtual bool CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) const override;
	virtual void HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) override;
	virtual void OnActorInitStateChanged(const FActorInitStateChangedParams& Params) override;
	virtual void CheckDefaultInitialization() override;

public:
	// 这三组委托表达的是“Hero 观察到了输入事件”，而不是“Hero 已经替某个系统完成了业务处理”。
	// 任何上层系统如果需要感知输入，都应该自己订阅并在内部判断是否关心这次输入。
	FOnPressInputLoad OnPressInputLoad;
	FOnReleaseInputLoad OnReleaseInputLoad;
	FOnStartInputLoad OnStartInputLoad;

	// 对外暴露的统一输入注入口。
	// 这一层只负责把“某个输入标签发生了某种输入事件”广播并送进既有输入链，
	// 不再替技能系统做高层语义判断，让 Hero 稳定保持在“输入发送者/桥接层”的职责上。
	UFUNCTION(BlueprintCallable, Category = "AOHero")
	bool InjectAbilityInputCommand(FGameplayTag InputTag, TEnumAsByte<EInputType> InputType);

protected:
	virtual void OnRegister() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// 这三个函数是 Hero 输入桥接层真正落地到运行时链路的入口。
	// 它们负责维护 InputBuffer、广播输入委托，并把输入送进 ASC。
	// 上层系统如果想感知输入，应该订阅委托，而不是改写这里的基础路由。
	void Input_AbilityInputTagPressed(FGameplayTag InputTag);
	void Input_AbilityInputTagReleased(FGameplayTag InputTag);
	void Input_AbilityInputTagStarted(FGameplayTag InputTag);

	void Input_Move(const FInputActionValue& InputActionValue);
	void LookUp(const struct FInputActionValue& InputActionValue);

	virtual void InitializePlayerInput(UInputComponent* PlayerInputComponent);
	void InitializePlayerInputInternal(UInputComponent* PlayerInputComponent);
	void OnGameFeatureRemove();
	void OnGameFeaturActivate();

	TSubclassOf<UAOCameraMode> DetermineCameraMode() const;

protected:
	/** Camera mode set by an ability. */
	UPROPERTY()
	TSubclassOf<UAOCameraMode> AbilityCameraMode;

	UPROPERTY(EditDefaultsOnly, Category = "Config")
	TArray<FInputMappingContextAndPriority> DefaultInputMappings;
};
