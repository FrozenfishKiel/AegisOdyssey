#pragma once

#include "CoreMinimal.h"
#include "AOMVVMViewModelBase.h"
#include "AegisOdyssey/AOCombatResultMessage.h"
#include "MVVM_LocalCombatState.generated.h"

// 本地玩家战斗状态 ViewModel。
// 它承接的是“本地玩家现在处于什么战斗状态”，不是资源条，也不是反馈流。
UCLASS(Blueprintable)
class AEGISODYSSEY_API UMVVM_LocalCombatState : public UAOMVVMViewModelBase
{
	GENERATED_BODY()

public:
	UMVVM_LocalCombatState(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	void SetInCombat(bool bInInCombat);
	void SetBlocking(bool bInBlocking);
	void SetBroken(bool bInBroken);
	void SetParried(bool bInParried);
	void SetAbilityInputBlocked(bool bInAbilityInputBlocked);
	void SetLastResultType(EAOCombatResultType InLastResultType);

	UFUNCTION(BlueprintPure, FieldNotify, Category = "AO|Combat State")
	bool IsInCombat() const { return bInCombat; }

	UFUNCTION(BlueprintPure, FieldNotify, Category = "AO|Combat State")
	bool IsBlocking() const { return bBlocking; }

	UFUNCTION(BlueprintPure, FieldNotify, Category = "AO|Combat State")
	bool IsBroken() const { return bBroken; }

	UFUNCTION(BlueprintPure, FieldNotify, Category = "AO|Combat State")
	bool IsParried() const { return bParried; }

	UFUNCTION(BlueprintPure, FieldNotify, Category = "AO|Combat State")
	bool IsAbilityInputBlocked() const { return bAbilityInputBlocked; }

	UFUNCTION(BlueprintPure, FieldNotify, Category = "AO|Combat State")
	EAOCombatResultType GetLastResultType() const { return LastResultType; }

private:
	// 本地玩家当前是否处于战斗显示状态。
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter = IsInCombat, Category = "AO|Combat State", meta = (AllowPrivateAccess))
	bool bInCombat = false;

	// 本地玩家当前是否处于格挡态。
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter = IsBlocking, Category = "AO|Combat State", meta = (AllowPrivateAccess))
	bool bBlocking = false;

	// 本地玩家当前是否处于破韧态。
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter = IsBroken, Category = "AO|Combat State", meta = (AllowPrivateAccess))
	bool bBroken = false;

	// 本地玩家当前是否处于被弹反反应态。
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter = IsParried, Category = "AO|Combat State", meta = (AllowPrivateAccess))
	bool bParried = false;

	// 本地玩家当前是否被战斗系统阻止能力输入。
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter = IsAbilityInputBlocked, Category = "AO|Combat State", meta = (AllowPrivateAccess))
	bool bAbilityInputBlocked = false;

	// 最近一次与本地玩家直接相关的战斗结果类型。
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter = GetLastResultType, Category = "AO|Combat State", meta = (AllowPrivateAccess))
	EAOCombatResultType LastResultType = EAOCombatResultType::None;
};
