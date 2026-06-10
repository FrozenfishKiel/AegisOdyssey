// Pawn 运行时基础配置数据，集中声明角色默认能力、输入、成长表和制造配方表。
#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "AegisOdyssey/AbilitySystem/AOAbilitySet.h"
#include "AegisOdyssey/Input/AOInputConfig.h"
#include "Engine/DataAsset.h"
#include "AOPawnData.generated.h"

class UAOCameraMode;
class UAOAnimStateData;
class UGameFeatureAction;
class UCurveTable;
class UDataTable;

UCLASS(BlueprintType, Const, Meta = (DisplayName = "AO Pawn Data", ShortTooltip = "Data asset used to define a Pawn."))
class AEGISODYSSEY_API UAOPawnData : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AO|Pawn")
	TSubclassOf<APawn> PawnClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AO|Pawn")
	TArray<TObjectPtr<UAOAbilitySet>> AbilitySets;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AO|Pawn")
	TObjectPtr<UGameplayAbility> DefaultAbility;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AO|Pawn")
	TObjectPtr<UAOInputConfig> InputConfig;

	// 玩家控制该 Pawn 时默认使用的摄像机模式。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AO|Camera")
	TSubclassOf<UAOCameraMode> DefaultCameraMode;

	// Experience 在加载、激活、停用、卸载过程中要执行的动作列表。
	UPROPERTY(EditDefaultsOnly, Instanced, Category = "Actions")
	TArray<TObjectPtr<UGameFeatureAction>> Actions;

	// ========== 等级系统 ==========

	// 等级经验表，描述每个等级升级所需的经验值。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DataTable|Level")
	TObjectPtr<UCurveTable> LevelUpXPTable;

	// 等级属性点表，描述每个等级奖励多少属性点。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DataTable|Level")
	TObjectPtr<UCurveTable> LevelUpAttributePointsTable;

	int32 GetXPRequiredForLevel(int32 Level) const;
	int32 GetAttributePointsForLevel(int32 Level) const;

	// ========== 属性系统 ==========

	// 等级属性表，描述每个等级固定增长多少属性。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DataTable|AttributeDataTable")
	TObjectPtr<UCurveTable> AttributeDataTable;

	inline TArray<int32> GetAllAttributeValueFromLevel(int32 Level) const;
	int32 GetAttributeValueFromNameAndLevel(const FName AttributeName, const int32 Level) const;

	// 当前角色唯一的制造配方表。
	// 解锁信息、排序信息和配方内容全部收在这一张表里。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DataTable|Crafting")
	TSoftObjectPtr<UDataTable> CraftingRecipeDataTable;

	const UDataTable* GetCraftingRecipeDataTable() const;
};
