#pragma once

#include "CoreMinimal.h"
#include "AOMVVMViewModelBase.h"
#include "MVVM_TargetHealthBar.generated.h"

class AActor;

// 单个世界目标血条的 ViewModel。
// 它只代表“某一个目标自己”的可绑定血条状态，不代表本地玩家 HUD，也不代表观察者集合。
// 归属边界：
// 1. 目标自身生命值真相归目标侧管理。
// 2. 最终显隐结果会同步进来，供 Widget 直接绑定。
// 3. 它不负责决定谁该看见它，这个资格判断仍然来自本地观察者组件。
UCLASS(Blueprintable)
class AEGISODYSSEY_API UMVVMTargetHealthBar : public UAOMVVMViewModelBase
{
	GENERATED_BODY()

public:
	UMVVMTargetHealthBar(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	void SetTargetActor(AActor* InTargetActor);

	UFUNCTION(BlueprintPure, FieldNotify, Category = "AO|Target Health Bar")
	AActor* GetTargetActor() const { return TargetActor; }

	void SetCurrentHealth(float InCurrentHealth);

	UFUNCTION(BlueprintPure, FieldNotify, Category = "AO|Target Health Bar")
	float GetCurrentHealth() const { return CurrentHealth; }

	void SetMaxHealth(float InMaxHealth);

	UFUNCTION(BlueprintPure, FieldNotify, Category = "AO|Target Health Bar")
	float GetMaxHealth() const { return MaxHealth; }

	UFUNCTION(BlueprintPure, FieldNotify, Category = "AO|Target Health Bar")
	float GetHealthPercent() const;

	void SetDead(bool bInDead);

	UFUNCTION(BlueprintPure, FieldNotify, Category = "AO|Target Health Bar")
	bool IsDead() const { return bDead; }

	void SetVisible(bool bInVisible);

	UFUNCTION(BlueprintPure, FieldNotify, Category = "AO|Target Health Bar")
	bool IsVisible() const { return bVisible; }

private:
	// 这个 ViewModel 对应的目标 Actor。
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter, meta = (AllowPrivateAccess))
	TObjectPtr<AActor> TargetActor = nullptr;

	// 目标当前生命值。
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter, meta = (AllowPrivateAccess))
	float CurrentHealth = 0.0f;

	// 目标最大生命值。
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter, meta = (AllowPrivateAccess))
	float MaxHealth = 0.0f;

	// 目标是否已死亡。
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter = IsDead, meta = (AllowPrivateAccess))
	bool bDead = false;

	// 当前本地玩家视角下，这个目标血条是否应显示。
	// 这个值可以直接给蓝图做显隐绑定。
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter = IsVisible, meta = (AllowPrivateAccess))
	bool bVisible = false;
};
