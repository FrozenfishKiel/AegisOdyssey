// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AegisOdyssey/AbilitySystem/AOAbilitySystem.h"
#include "AegisOdyssey/AbilitySystem/Attributes/AOAttributeSet.h"
#include "AegisOdyssey/Player/AOPlayerController.h"
#include "AegisOdyssey/UI/ViewModel/MVVM_HUD.h"
#include "AegisOdyssey/Player/AOPlayerState.h"
#include "AegisOdyssey/UI/ViewModel/Inventory/MVVM_InventoryMenu.h"
#include "Components/PawnComponent.h"
#include "Components/GameFrameworkInitStateInterface.h"
#include "AOVMPawnComponent.generated.h"

/**
 * 
 */
//这是用来管理角色Viewmodel的组件，这个组件里每个角色的ViewModel都是独有的，同样他也需要等待初始化必要的组件初始化完毕以后才会开始
//依次地创建自己的ViewModel，主要是创建和角色相关的ViewModel
UCLASS(Blueprintable, Meta=(BlueprintSpawnableComponent))
class AEGISODYSSEY_API UAOVMPawnComponent : public UPawnComponent , public IGameFrameworkInitStateInterface
{
	GENERATED_BODY()
public:
	UAOVMPawnComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	virtual void OnActorInitStateChanged(const FActorInitStateChangedParams& Params) override;
	virtual void BeginPlay() override;
	virtual void UninitializeComponent() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) override;
	virtual void OnRegister() override;
	virtual void CheckDefaultInitialization() override;

	static const FName NAME_ActorFeatureName;
	virtual FName GetFeatureName() const override {return NAME_ActorFeatureName;}
public:
	UFUNCTION(BlueprintPure)
	inline UMVVM_HUD* GetCharacterHUDViewModel() const {return CharacterHUDViewModel;}
	UFUNCTION(BlueprintPure)
	inline UMVVM_InventoryMenu* GetCharacterInventoryViewModel() const {return CharacterInventoryViewModel;}
private:
	void InitializeViewModel();
	void InitializeHUDViewModel();
	void InitializeInventoryViewModel();
private:
	UFUNCTION()
	void OnRep_CharacterHUDViewModel();
	UFUNCTION()
	void OnRep_CharacterInventoryViewModel();
private:
	UPROPERTY(ReplicatedUsing = OnRep_CharacterHUDViewModel)
	TObjectPtr<UMVVM_HUD> CharacterHUDViewModel;
	UPROPERTY(ReplicatedUsing = OnRep_CharacterInventoryViewModel)
	TObjectPtr<UMVVM_InventoryMenu> CharacterInventoryViewModel;

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
};
