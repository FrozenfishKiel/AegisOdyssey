#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Actor.h"
#include "AOCombatGameplayCueNotify_WeaponTrail.generated.h"

class UNiagaraComponent;
class UNiagaraSystem;
class UAOWeaponInstance;
class USceneComponent;

USTRUCT(BlueprintType)
struct FAOCombatWeaponTrailChannel
{
	GENERATED_BODY()

	// 仅用于编辑器识别与排查，不参与运行时匹配。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Trail")
	FName ChannelName = NAME_None;

	// 这条 trail 通道实际驱动的 Niagara 资源。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Trail")
	TObjectPtr<UNiagaraSystem> NiagaraSystem = nullptr;

	// Trail 起点与终点的 socket 名。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Trail")
	FName StartSocketName = NAME_None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Trail")
	FName EndSocketName = NAME_None;

	// 默认按 Niagara 常见双点 trail 的用户参数命名约定来驱动。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Trail")
	FName StartPointVariableName = TEXT("User.TrailStart");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Trail")
	FName EndPointVariableName = TEXT("User.TrailEnd");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Trail")
	FVector RelativeScale = FVector(1.0f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Trail")
	bool bEnabled = true;
};

USTRUCT(BlueprintType)
struct FAOCombatWeaponTrailSamplingSettings
{
	GENERATED_BODY()

	// 当前简单版保留占位结构，避免旧 GC 资产在属性布局上断裂。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sampling", meta = (ClampMin = "0.001"))
	float Reserved = 0.016f;
};

USTRUCT(BlueprintType)
struct FAOCombatWeaponTrailDebugSettings
{
	GENERATED_BODY()

	// 是否绘制当前帧 start/end 连线与端点。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Debug")
	bool bDrawCurrentFrame = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Debug", meta = (EditCondition = "bDrawCurrentFrame", ClampMin = "0.0"))
	float CurrentLineThickness = 2.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Debug", meta = (EditCondition = "bDrawCurrentFrame", ClampMin = "0.0"))
	float CurrentPointSize = 8.0f;

	// 当前简单版不再绘制历史样本轨迹，保留占位字段仅做旧资产兼容。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Debug")
	bool bDrawHistory = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Debug", meta = (EditCondition = "bDrawHistory", ClampMin = "0.0"))
	float HistoryLineThickness = 1.5f;
};

USTRUCT()
struct FAOWeaponTrailRuntimeEntry
{
	GENERATED_BODY()

	// 每条激活中的 trail 通道对应一个运行时条目，负责持有实例化后的 Niagara 组件与历史缓存。
	UPROPERTY(Transient)
	TObjectPtr<UNiagaraComponent> NiagaraComponent = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<USceneComponent> StartComponent = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<USceneComponent> EndComponent = nullptr;

	FName StartSocketName = NAME_None;
	FName EndSocketName = NAME_None;
	FName StartPointVariableName = NAME_None;
	FName EndPointVariableName = NAME_None;
};

/**
 * Combat 专用武器 trail GameplayCue。
 * 这一层不再把 WeaponSwingLoop 理解成“单点挂一个持续 Niagara”，
 * 而是按 GC 资产里配置的多条 trail 通道，持续把起点/终点 socket 世界坐标喂给 Niagara。
 */
UCLASS(Blueprintable)
class AEGISODYSSEY_API AAOCombatGameplayCueNotify_WeaponTrail : public AGameplayCueNotify_Actor
{
	GENERATED_BODY()

public:
	AAOCombatGameplayCueNotify_WeaponTrail();
	virtual void PostLoad() override;

	virtual bool OnActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) override;
	virtual bool WhileActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) override;
	virtual bool OnRemove_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) override;
	virtual void Tick(float DeltaSeconds) override;

protected:
	// 武器 trail 表现配置直接归 GC 资产所有，武器定义不再承载这层表现数据。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Trail")
	TArray<FAOCombatWeaponTrailChannel> TrailChannels;

	// 第四阶段开始，运行时采样参数统一收口到 SamplingSettings。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Trail", meta = (ShowOnlyInnerProperties))
	FAOCombatWeaponTrailSamplingSettings SamplingSettings;

	// 第四阶段开始，调试显示参数统一收口到 DebugSettings。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Debug", meta = (ShowOnlyInnerProperties))
	FAOCombatWeaponTrailDebugSettings DebugSettings;

private:
	UPROPERTY()
	bool bDrawDebugTrail_DEPRECATED = true;

	UPROPERTY()
	float DebugLineThickness_DEPRECATED = 2.0f;

	UPROPERTY()
	float DebugPointSize_DEPRECATED = 8.0f;

	UPROPERTY()
	bool bDrawDebugTrailHistory_DEPRECATED = true;

	UPROPERTY()
	float DebugHistoryLineThickness_DEPRECATED = 1.5f;

	bool ActivateTrail(AActor* MyTarget, const FGameplayCueParameters& Parameters);
	const UAOWeaponInstance* ResolveWeaponInstance(AActor* MyTarget, const FGameplayCueParameters& Parameters) const;
	USceneComponent* ResolveAttachComponent(const UAOWeaponInstance* WeaponInstance, FName SocketName) const;
	static FVector ResolveSocketWorldLocation(const USceneComponent* Component, FName SocketName);
	void DrawDebugTrailLine(const FVector& StartLocation, const FVector& EndLocation) const;
	void UpdateTrailEntries();

	UPROPERTY(Transient)
	TArray<FAOWeaponTrailRuntimeEntry> ActiveTrailEntries;
};
