#pragma once

#include "CoreMinimal.h"
#include "AOMVVMViewModelBase.h"
#include "MVVM_CombatResources.generated.h"

// 本地玩家战斗资源 ViewModel。
// 它只承接“本地玩家自己”的常驻资源数值，不承接战斗反馈流，也不承接目标集合观察。
UCLASS(Blueprintable)
class AEGISODYSSEY_API UMVVM_CombatResources : public UAOMVVMViewModelBase
{
	GENERATED_BODY()

public:
	UMVVM_CombatResources(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	void SetHealth(float InHealth);
	void SetMaxHealth(float InMaxHealth);

	void SetVigor(float InVigor);
	void SetMaxVigor(float InMaxVigor);

	void SetStamina(float InStamina);
	void SetMaxStamina(float InMaxStamina);

	UFUNCTION(BlueprintPure, FieldNotify, Category = "AO|Combat Resources")
	float GetHealth() const { return Health; }

	UFUNCTION(BlueprintPure, FieldNotify, Category = "AO|Combat Resources")
	float GetMaxHealth() const { return MaxHealth; }

	UFUNCTION(BlueprintPure, FieldNotify, Category = "AO|Combat Resources")
	float GetHealthPercent() const;

	UFUNCTION(BlueprintPure, FieldNotify, Category = "AO|Combat Resources")
	float GetVigor() const { return Vigor; }

	UFUNCTION(BlueprintPure, FieldNotify, Category = "AO|Combat Resources")
	float GetMaxVigor() const { return MaxVigor; }

	UFUNCTION(BlueprintPure, FieldNotify, Category = "AO|Combat Resources")
	float GetVigorPercent() const;

	UFUNCTION(BlueprintPure, FieldNotify, Category = "AO|Combat Resources")
	float GetStamina() const { return Stamina; }

	UFUNCTION(BlueprintPure, FieldNotify, Category = "AO|Combat Resources")
	float GetMaxStamina() const { return MaxStamina; }

	UFUNCTION(BlueprintPure, FieldNotify, Category = "AO|Combat Resources")
	float GetStaminaPercent() const;

private:
	// 本地玩家当前生命值。
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter, Category = "AO|Combat Resources", meta = (AllowPrivateAccess))
	float Health = 0.0f;

	// 本地玩家最大生命值。
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter, Category = "AO|Combat Resources", meta = (AllowPrivateAccess))
	float MaxHealth = 0.0f;

	// 本地玩家当前体力值。
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter, Category = "AO|Combat Resources", meta = (AllowPrivateAccess))
	float Vigor = 0.0f;

	// 本地玩家最大体力值。
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter, Category = "AO|Combat Resources", meta = (AllowPrivateAccess))
	float MaxVigor = 0.0f;

	// 本地玩家当前韧性值。
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter, Category = "AO|Combat Resources", meta = (AllowPrivateAccess))
	float Stamina = 0.0f;

	// 本地玩家最大韧性值。
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter, Category = "AO|Combat Resources", meta = (AllowPrivateAccess))
	float MaxStamina = 0.0f;
};
