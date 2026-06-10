// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AegisOdyssey/Equipment//AOEquipmentInstance.h"
#include "AOWeaponInstance.generated.h"

class UAOAttackEffectProfile;
class USceneComponent;

UCLASS()
class AEGISODYSSEY_API UAOWeaponInstance : public UAOEquipmentInstance
{
	GENERATED_BODY()

public:
	UAOWeaponInstance(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	virtual void SetItemDef(TSubclassOf<UAOInventoryItemDefinition> InDef) override;

	// 当前有效的攻击表现 Profile 统一从这里取。
	// 现阶段先回落到武器定义默认值，后续覆盖层也继续走这个入口。
	const UAOAttackEffectProfile* GetEffectiveAttackEffectProfile() const;

	// 武器表现层不要直接硬引用某个蓝图对象。
	// 统一从当前 WeaponInstance 取它运行时真正生成出来的武器 Actor。
	void GetSpawnedWeaponActors(TArray<AActor*>& OutActors) const;

	// 持续特效后续如果需要挂 socket，统一从这里拿“当前武器可用的附着组件列表”。
	// 这样上层配置的是“当前武器”，不是某把具体蓝图里的某个具体对象。
	void GetSpawnedWeaponAttachComponents(TArray<USceneComponent*>& OutComponents) const;

	// 在当前武器实例生成出来的组件范围内解析 socket，不做全局 Actor 乱搜。
	USceneComponent* FindSpawnedWeaponAttachComponentBySocket(FName SocketName) const;
};
