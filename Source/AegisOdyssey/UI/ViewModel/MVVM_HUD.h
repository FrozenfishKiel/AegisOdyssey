// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AOMVVMViewModelBase.h"
#include "AttributeSet.h"
#include "AegisOdyssey/AbilitySystem/AOAbilitySystem.h"
#include "AegisOdyssey/AbilitySystem/Attributes/AOAttributeSet.h"
#include "AegisOdyssey/AbilitySystem/Attributes/AOHealthAttributeSet.h"
#include "AegisOdyssey/Player/AOPlayerController.h"
#include "AegisOdyssey/Player/AOPlayerState.h"
#include "MVVM_HUD.generated.h"

/**
 * 
 */
USTRUCT()
struct FPlayerMainHUDViewModelParams
{
	GENERATED_USTRUCT_BODY()
	
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> ASC = nullptr;
	UPROPERTY()
	TObjectPtr<APlayerController> PC = nullptr;
	UPROPERTY()
	TObjectPtr<APlayerState> PS = nullptr;
};
UCLASS()
class AEGISODYSSEY_API UMVVM_HUD : public UAOMVVMViewModelBase
{
	GENERATED_BODY()
public:
	UMVVM_HUD(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	void SetPlayerViewModelParams(const FPlayerMainHUDViewModelParams& params);
	UFUNCTION(BlueprintNativeEvent)
	void OnParamSet();
	UAOAbilitySystem* GetSourceASC() const;
	AAOPlayerController* GetSourcePC() const;
	AAOPlayerState* GetSourcePS() const;
public:
	void SetHealth(const float InHealth);
	UFUNCTION(BlueprintPure)
	float GetHealth() const {return Health;}

	void SetMaxHealth(const float InMaxHealth);
	UFUNCTION(BlueprintPure)
	float GetMaxHealth ()const {return MaxHealth;}
	//FieldNotify字段的函数用于方便在视图模型中进行计算或者转换
	UFUNCTION(BlueprintPure, FieldNotify , Category="AO|ViewModel")
	float GetHealthPercent() const;
private:
	UPROPERTY(ReplicatedUsing = OnRep_Health,BlueprintReadOnly , FieldNotify , Setter , Getter , Meta = (AllowPrivateAccess))
	float Health = .0f;

	UPROPERTY(ReplicatedUsing = OnRep_MaxHealth , BlueprintReadOnly , FieldNotify , Setter , Getter , Meta = (AllowPrivateAccess))
	float MaxHealth = .0f;

public:
	UFUNCTION()
	void OnRep_Health();
	UFUNCTION()
	void OnRep_MaxHealth();
private:
	FPlayerMainHUDViewModelParams PlayerViewModelParams;
private:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool IsSupportedForNetworking() const override {return true;}
};
