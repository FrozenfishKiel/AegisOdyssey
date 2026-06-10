#pragma once

#include "CoreMinimal.h"
#include "AegisOdyssey/SkillSystem/Core/AOSkillExecutionDefinition.h"
#include "AOSkillExecutionDefinition_Projectile.generated.h"

/**
 * 投射体执行定义。
 *
 * 适用于“技能激活后生成一个投射体 Actor，后续飞行/碰撞由投射体自己负责”的执行链。
 */
UCLASS(BlueprintType, Blueprintable, DefaultToInstanced, EditInlineNew)
class AEGISODYSSEY_API UAOSkillProjectileExecutionDefinition : public UAOSkillExecutionDefinition
{
	GENERATED_BODY()

public:
	UAOSkillProjectileExecutionDefinition(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// 投射体生成原点配置。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
	FAOSkillOriginConfig SpawnOrigin;

	// true 表示最终发射朝向优先跟随玩家当前镜头/控制朝向，
	// 没有可用视角信息时再退回角色当前朝向，而不是完全吃 Socket 朝向。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
	bool bUseAvatarFacing = true;

	// 投射体飞行速度。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
	FScalableFloat ProjectileSpeed = FScalableFloat(1600.0f);

	// 最大飞行距离。大于 0 时，投射体飞出这段距离后会自动销毁。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
	FScalableFloat MaxTravelDistance = FScalableFloat(3000.0f);

	// 投射体自身碰撞半径。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
	FScalableFloat CollisionRadius = FScalableFloat(24.0f);

	// 命中后的爆炸半径。
	// 大于 0 时，火球会在命中点按这个半径收集目标，而不是只打首个目标。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
	FScalableFloat ExplosionRadius = FScalableFloat(0.0f);

	// 兜底寿命。即使投射体自己不销毁，也会在这个时间后被清掉。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
	FScalableFloat ProjectileLifetime = FScalableFloat(5.0f);
};
