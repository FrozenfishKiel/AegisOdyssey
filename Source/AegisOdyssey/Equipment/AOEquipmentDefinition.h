// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AegisOdyssey/Inventory/AOInventoryItemDefinition.h"
#include "AOEquipmentDefinition.generated.h"

class UAOAbilitySet;
class AActor;
class UAOEquipmentInstance;
class UAOEquipmentFeatureAction;
/**
 * 
 */
USTRUCT(BlueprintType)
struct FAOEquipmentSpawnedConfig
{
     GENERATED_BODY()
public:
     FAOEquipmentSpawnedConfig() {}
 
     UPROPERTY(EditAnywhere , Category = EquipmentConfig)
     TSubclassOf<AActor> ActorSpawnedClass;  //生成的Actor保存的类引用
 
     UPROPERTY(EditAnywhere , Category = EquipmentConfig)
     FName AttachSocketName;  //生成在对应的角色的骨骼位置
 
     UPROPERTY(EditAnywhere , Category = EquipmentConfig)
     FTransform SpawnedTransform;  //生成的对应的旋转配置

	 UPROPERTY(EditAnywhere , Category = EquipmentConfig)
	 FName SpawnedMeshTag;
	
 };
UCLASS(Blueprintable , Const , Abstract , BlueprintType)
class AEGISODYSSEY_API UAOEquipmentDefinition : public UAOInventoryItemDefinition
{
	GENERATED_BODY()

public:
	UAOEquipmentDefinition(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual TSubclassOf<UAOInventoryItemInstance> GetPreferredInstanceType() const override;
public:
	inline const TArray<TObjectPtr<UAOAbilitySet>>& GetAbilitySetsToGrant() const {return AbilitySetsToGrant;}
	inline const TArray<TObjectPtr<UAOEquipmentFeatureAction>>& GetFeatureActionsToGrant() const { return FeatureActions; }
	UPROPERTY(EditDefaultsOnly, Category = EquipmentConfig , meta = (AllowPrivateAccess))
	TArray<FAOEquipmentSpawnedConfig> ActorToSpawn;
private:
	UPROPERTY(EditDefaultsOnly, Category = EquipmentConfig , meta = (AllowPrivateAccess))
	TArray<TObjectPtr<UAOAbilitySet>> AbilitySetsToGrant;

	UPROPERTY(EditDefaultsOnly, Instanced, Category = EquipmentConfig, meta = (AllowPrivateAccess))
	TArray<TObjectPtr<UAOEquipmentFeatureAction>> FeatureActions;

};
