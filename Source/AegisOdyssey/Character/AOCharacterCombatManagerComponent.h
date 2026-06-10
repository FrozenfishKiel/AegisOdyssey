// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/PawnComponent.h"
#include "GameplayTagContainer.h"
#include "AttributeSet.h"
#include "GameplayEffectTypes.h"
#include "AegisOdyssey/AOAbilityTypes.h"
#include "AOCharacterCombatManagerComponent.generated.h"

class UGameplayEffect;
struct FAttackedInfo;
class UAbilitySystemComponent;
class UCharacterMovementComponent;
class AAOCharacter;
struct FAOCombatResultMessage;

UENUM()
enum class EAOHitReactLevel : uint8
{
	Ignore,
	Light,
	Heavy
};

USTRUCT(BlueprintType)
struct FAOCombatMagnetWindowConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Magnet")
	FName WarpTargetName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Magnet", meta = (ClampMin = "0.0"))
	float SearchRadius = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Magnet", meta = (ClampMin = "0.0", ClampMax = "180.0"))
	float SearchHalfAngleDegrees = 45.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Magnet")
	bool bDrawDebug = false;
};

// 防御结算默认参数集合。
// 这一组值描述的是当前战斗系统里“格挡 / 弹反 / 受击 / 破韧”这一整套默认数值语义，
// 它们都集中放在这里，避免把关键平衡参数散写进各个攻击实现。
USTRUCT(BlueprintType)
struct FAOCombatDefenseConfig
{
	GENERATED_BODY()

	// 正面格挡允许的半角。
	// 例如 70 度表示总共 140 度的正面防御扇区，超出这个角度就不能按格挡解释。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Defense")
	float BlockHalfAngleDegrees = 70.0f;

	// 弹反允许的半角。
	// 默认比格挡更严格，要求玩家更正面对准攻击来源。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Defense")
	float ParryHalfAngleDegrees = 55.0f;

	// 部分格挡时的生命伤害倍率。
	// 例如 0.35 表示这次命中仍然成立，但只吃 35% 的生命伤害。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Defense")
	float PartialBlockDamageMultiplier = 0.35f;

	// 正常受击时默认扣除的韧性值。
	// 当前项目中的 Stamina 语义是韧性条，不是动作体力条。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Defense")
	float HitStaminaDamage = 20.0f;

	// 格挡成立时默认扣除的韧性值。
	// 设计要求是“受击就扣韧性，但格挡少扣一点”，这里对应的就是那个较小值。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Defense")
	float BlockStaminaDamage = 12.0f;

	// 部分格挡时扣除的体力值。
	// 当前项目中的 Vigor 语义是动作体力条。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Defense")
	float PartialBlockVigorCost = 12.0f;

	// 完全格挡时扣除的体力值。
	// 完全格挡虽然不掉血，但依然有明确的资源成本。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Defense")
	float FullBlockVigorCost = 8.0f;

	// 弹反成立时防守方支付的体力成本。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Defense")
	float ParryVigorCost = 6.0f;

	// 弹反成功后，对进攻方额外施加的韧性伤害。
	// 这是“弹反强交互性”的关键数值之一。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Defense")
	float ParryAttackerStaminaDamage = 45.0f;

	// 进攻方被成功弹反但尚未破韧时，进入受控硬直状态的时长。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Defense")
	float ParriedDurationSeconds = 0.45f;

	// 破韧成立后，角色被锁定在破韧状态中的持续时间。
	// 时间结束时会按当前方案把韧性条直接回满。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Defense")
	float BrokenDurationSeconds = 1.5f;
};

// 战斗命中统一结算组件。
// 当前所有普攻、重击、技能、投射体、AOE 命中，最终都应该从这里进入正式战斗系统主链。
UCLASS(Blueprintable, meta=(BlueprintSpawnableComponent))
class AEGISODYSSEY_API UAOCharacterCombatManagerComponent : public UPawnComponent
{
	GENERATED_BODY()

public:
	// 构造阶段只负责建立组件自身默认状态。
	UAOCharacterCombatManagerComponent(const FObjectInitializer& OI = FObjectInitializer::Get());

public:
	// 统一战斗结算主入口。
	// 命中采集层只负责把“谁打到了谁”送进来；真正的敌我过滤、防御解释、资源扣除、结果广播都在这里完成。
	void ApplyDamageToTarget(const FAttackedInfo& AttackedInfo);

	// 让拥有者进入“被弹反后的硬直反应”。
	// 这只在尚未被直接打空韧性时使用；如果韧性已经被弹反打空，就会直接转进破韧状态。
	void HandleParriedReaction();

	// 让拥有者进入破韧状态。
	// 这里会统一负责：广播破韧结果、上控制锁、打断能力、开启恢复定时器。
	void HandleBrokenState(AActor* InstigatorActor = nullptr, AActor* EffectCauserActor = nullptr, const UObject* SourceObject = nullptr);

	// 正式生命伤害成立后，从这里统一续接普通受击反应。
	// CombatManager 只在这一层解释“这次要不要进 Light / Heavy”，不直接参与播放动画。
	void HandleConfirmedHitReact(const FAOGameplayEffectContext& EffectContext, float FinalDamage);

	// 受击 GA 在允许移动窗口、结束窗口或被取消时，都会从这里统一清理受击状态。
	void EndHitReactState();

	// 被弹反反应 GA 在允许恢复或结束时，都会从这里统一清理被弹反状态。
	void EndParriedReaction();

	// 蒙太奇吸附窗口入口。
	// NotifyState 只负责转发窗口事件，真正的目标搜索和 MotionWarping 目标写入都收口到 CombatManager。
	bool BeginCombatMagnetWindow(const FAOCombatMagnetWindowConfig& WindowConfig);

	// 蒙太奇吸附窗口结束时移除本次写入的 WarpTarget。
	void EndCombatMagnetWindow(FName WarpTargetName);

	float GetPartialBlockDamageMultiplier() const { return DefenseConfig.PartialBlockDamageMultiplier; }
	float GetHitStaminaDamage() const { return DefenseConfig.HitStaminaDamage; }
	float GetBlockStaminaDamage() const { return DefenseConfig.BlockStaminaDamage; }

protected:
	// 生命周期入口，目前主要保留给后续初始化扩展。
	virtual void BeginPlay() override;

public:
	// 组件 Tick 当前没有主逻辑，保留是为了兼容后续战斗状态扩展。
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

private:
	// 结束破韧状态，恢复控制并把韧性条按当前方案直接补满。
	void EndBrokenState();

	// 对指定属性施加增量修改，并返回修改后的最新数值。
	// 统一走这个 helper，避免韧性 / 体力这类即时扣减逻辑散写在各处分支里。
	float ApplyAttributeDelta(UAbilitySystemComponent* AbilitySystemComponent, const FGameplayAttribute& Attribute, float DeltaValue) const;

	// 判断来袭方向是否位于防御扇区内。
	// 格挡和弹反都会复用这个角度判定，只是各自使用不同的半角配置。
	bool IsWithinDefenseAngle(const FVector& DefenderForward, const FVector& IncomingDirection, float HalfAngleDegrees) const;

	// 当前版本的统一敌我收口点。
	// 单人 PVE 下默认只允许“玩家 <-> 敌人”；未来改友伤、阵营、PVP 时，也应继续从这里扩展。
	bool CanResolveCombatBetweenActors(const AActor* SourceActor, const AActor* TargetActor) const;

	// 打断拥有者当前所有能力，并清掉输入缓存。
	// 破韧和被弹反都需要调用它，保证角色不能继续把旧动作硬播完。
	void InterruptAllAbilities(UAbilitySystemComponent* AbilitySystemComponent) const;

	// 直接锁定或恢复角色控制。
	// 这是破韧 / 被弹反这种“角色暂时不能移动和操作”的底层控制入口。
	void SetOwnerControlLocked(bool bLocked) const;

	// 给拥有者增删一组持久状态标签。
	// 通过 SourceId 做来源隔离，避免破韧和被弹反互相覆盖或清错标签。
	void ApplyPersistentControlState(FName SourceId, const FGameplayTagContainer& Tags, bool bApply) const;

	// 受击反应不直接复用全局输入封锁标签。
	// 第一版只锁移动，技能打断和输入限制优先交给受击 GA 自己配置。
	void ApplyHitReactState(const FGameplayTag& StateTag, bool bApply);

	// 从命中上下文里提取“攻击来自哪边”的世界方向。
	// CombatManager 只负责发送受击语义，不在这里拼动画池键。
	FVector ResolveHitReactSourceDirection(const FAOGameplayEffectContext& EffectContext) const;

	// 根据受击级别解析正式受击状态标签。
	FGameplayTag ResolveHitReactStateTag(EAOHitReactLevel HitReactLevel) const;

	// 判断当前受击级别是否应该被现有受击状态覆盖。
	bool ShouldRejectHitReactLevel(UAbilitySystemComponent* TargetASC, EAOHitReactLevel NewLevel) const;

	// 把正式伤害执行层已经算好的受击强度解释成 Ignore / Light / Heavy。
	EAOHitReactLevel ResolveHitReactLevel(UAbilitySystemComponent* TargetASC, float ResolvedHitReactStrength) const;

	// 解析吸附搜索朝向。
	// 玩家使用控制器朝向，AI 和兜底路径使用角色面朝方向。
	FVector ResolveCombatMagnetSearchForward(const AAOCharacter& OwnerCharacter) const;

	// 在扇形范围内选择最近可吸附目标。
	AActor* FindBestCombatMagnetTarget(const AAOCharacter& OwnerCharacter, const FAOCombatMagnetWindowConfig& WindowConfig, const FVector& SearchForward) const;

	// 统一复用战斗系统里的“存活”过滤。
	bool IsCombatMagnetTargetAlive(const AActor& CandidateActor) const;

	// 可选调试绘制，只服务参数调试，不参与正式逻辑。
	void DrawCombatMagnetDebug(const AAOCharacter& OwnerCharacter, const FAOCombatMagnetWindowConfig& WindowConfig, const FVector& SearchForward, const AActor* SelectedTarget) const;

	// 向世界级战斗消息总线广播统一结果。
	// CombatManager 自己只负责发布真相，不负责决定谁来显示或怎么显示。
	void BroadcastCombatResult(const FAOCombatResultMessage& Message) const;

	// 统一 GameplayCue 出口。
	// Cue 在当前架构里只消费已经定稿的战斗结果，不参与任何命中真相判断。
	void ExecuteCombatCue(
		UAbilitySystemComponent* AbilitySystemComponent,
		const FGameplayTag& CueTag,
		const FGameplayEffectContextHandle* EffectContextHandle,
		const FHitResult* HitResult,
		AActor* InstigatorActor,
		AActor* EffectCauserActor,
		UObject* SourceObject,
		float RawMagnitude) const;

	// 某些正式战斗结果不走伤害 GE 上下文，这里统一补一个最小 CombatEffectContext 给 Cue 层分流用。
	FGameplayEffectContextHandle BuildCombatCueEffectContext(
		AActor* InstigatorActor,
		AActor* EffectCauserActor,
		UObject* SourceObject,
		const FHitResult* HitResult,
		const FGameplayTag& AttackTag,
		const FGameplayTag& SkillTag,
		const FGameplayTag& WeaponTag,
		const FGameplayTagContainer& DamageTypeTags,
		bool bIsCritical,
		bool bWasBlocked,
		bool bWasParried,
		bool bHitInvulnerability,
		bool bTargetBroken) const;

private:
	// 当前战斗系统默认使用的防御语义与数值配置。
	UPROPERTY(EditDefaultsOnly, Category = "AO|Combat Defense")
	FAOCombatDefenseConfig DefenseConfig;

	// 拥有者当前是否正处于“被弹反后的短暂硬直”状态。
	// 这是一个控制态，不等于破韧。
	UPROPERTY()
	bool bIsParriedReacting = false;

	// 拥有者当前是否处于破韧状态。
	// 破韧当前只承担“失衡 / 受控”语义，不应阻止后续命中继续进入正式战斗结算。
	UPROPERTY()
	bool bIsBroken = false;

	// 结束被弹反硬直的定时器。
	FTimerHandle ParriedReactionTimerHandle;

	// 结束破韧并回满韧性的定时器。
	FTimerHandle BrokenStateTimerHandle;
};
