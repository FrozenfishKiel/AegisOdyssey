// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AOExtPawnComponent.h"
#include "AbilitySystemInterface.h"
#include "CombatInterface.h"
#include "ModularCharacter.h"
#include "AegisOdyssey/Enum/EquipState.h"
#include "AegisOdyssey/Interaction/InteractableTarget.h"
#include "AegisOdyssey/Inventory/AOBackPackComponent.h"
#include "AegisOdyssey/Inventory/InventoryInterface.h"
#include "Interfaces/AOBotInterface.h"
#include "AOCharacter.generated.h"

class UAOCharacterCombatManagerComponent;
class ICombatInterface;
class UAOQuickBarComponent;
class UAOWeaponManagerComponent;
class UAOFormalEquipmentManagerComponent;
class UAOFormalEquipmentSlotInventoryComponent;
class UAOInventoryComponent;
class UAOSkillSlotInventoryComponent;
class UAOPersistentStateTagComponent;
class UAOAIDecisionComponent;
class UAOHealthAttributeSet;
class USpringArmComponent;
class UAOCameraComponent;
class UAOSkillComponent;
class UAOCraftingComponent;
class UBoxComponent;
class UMotionWarpingComponent;
/**
 * 
 */
UCLASS()
class AEGISODYSSEY_API AAOCharacter : public AModularCharacter,public IAbilitySystemInterface,public ICombatInterface,public IAOBotInterface, public IInteractableTarget, public IInventoryInterface
{
	GENERATED_BODY()
public:
	static const FName NAME_AOAbilityReady;
	AAOCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	UFUNCTION(BlueprintCallable)
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	virtual UAOAbilitySystem* GetSourceASC() const {return AOSourceASC ? AOSourceASC : nullptr;}

	UFUNCTION(BlueprintCallable, Category = "AOCharacter")
	USceneComponent* GetEquipmentAttachTargetByTag(const FName Tag) const;
	
	UFUNCTION(BlueprintCallable, Category = "AOCharacter")
	EEquipState FindEquipState() const;

	UFUNCTION(BlueprintPure, Category = "AOCharacter")
	UAOCraftingComponent* GetCraftingComponent() const { return CharacterCraftingComponent; }

	UFUNCTION(BlueprintPure, Category = "AOCharacter")
	UMotionWarpingComponent* GetMotionWarpingComponent() const { return MotionWarpingComponent; }

	virtual void GatherInteractionOptions(FInteractionOptionBuilder& OptionBuilder) override;
	virtual bool CanExecuteInteraction(const FGameplayTag& InteractionEventTag, const FGameplayEventData& EventData) const override;
	virtual bool ExecuteInteraction(const FGameplayTag& InteractionEventTag, const FGameplayEventData& EventData) override;
	virtual UAOInventoryComponent* GetInventoryComponent() override;
	const UAOInventoryComponent* GetInventoryComponent() const;

	bool CanOpenInventoryAsContainer() const;
	const FInteractionOption* GetDefaultInventoryInteractionOption() const;
private:
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="AOCharacterConfig",meta=(AllowPrivateAccess=true))
	TObjectPtr<UAOExtPawnComponent> AOExtPawnComp;
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="AOCharacterConfig",meta=(AllowPrivateAccess=true))
	TObjectPtr<UAOAbilitySystem> AOSourceASC;
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="AOCharacterConfig",meta=(AllowPrivateAccess=true))
	TObjectPtr<UAOCameraComponent> AOCameraComponent;
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="AOCharacterConfig",meta=(AllowPrivateAccess=true))
	TObjectPtr<USpringArmComponent> SpringArmComponent;
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="AOCharacterConfig",meta=(AllowPrivateAccess=true))
	TObjectPtr<UAOWeaponManagerComponent> WeaponInventoryManager;
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="AOCharacterConfig",meta=(AllowPrivateAccess=true))
	TObjectPtr<UAOBackPackComponent> CharacterBackPackComponent;
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="AOCharacterConfig",meta=(AllowPrivateAccess=true))
	TObjectPtr<UAOQuickBarComponent> CharacterQuickBar;
	// 角色技能系统的运行时总入口。
	// 后续技能来源接入、槽位管理、输入触发、AI 命令和 UI 请求都会尽量收口到这个组件。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AOCharacterConfig", meta = (AllowPrivateAccess = true))
	TObjectPtr<UAOSkillComponent> CharacterSkillComponent;
	// 技能槽库存适配层。
	// 它让技能槽可以复用现有库存拖拽/交换语义，但不替代 SkillComponent 的运行时真相职责。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AOCharacterConfig", meta = (AllowPrivateAccess = true))
	TObjectPtr<UAOSkillSlotInventoryComponent> CharacterSkillSlotInventoryComponent;
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="AOCharacterConfig",meta=(AllowPrivateAccess=true))
	TObjectPtr<UAOCraftingComponent> CharacterCraftingComponent;
	// 正式装备管理组件只负责头盔/盔甲/手部/项链/鞋子的长期穿戴真相。
	// 武器继续走 WeaponManager，这里不和当前武器激活态混用。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AOCharacterConfig", meta = (AllowPrivateAccess = true))
	TObjectPtr<UAOFormalEquipmentManagerComponent> CharacterFormalEquipmentManagerComponent;
	// 正式装备栏库存外壳。
	// 它只给 UI 和库存交换链一个正式容器入口，运行时真相仍由 FormalEquipmentManager 持有。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AOCharacterConfig", meta = (AllowPrivateAccess = true))
	TObjectPtr<UAOFormalEquipmentSlotInventoryComponent> CharacterFormalEquipmentSlotInventoryComponent;
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="AOCharacterConfig",meta=(AllowPrivateAccess=true))
	TObjectPtr<UAOCharacterCombatManagerComponent> CharacterCombatManagerComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="AOCharacterConfig", meta=(AllowPrivateAccess=true))
	TObjectPtr<UAOPersistentStateTagComponent> PersistentStateTagComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="AOCharacterConfig", meta=(AllowPrivateAccess=true))
	TObjectPtr<UAOAIDecisionComponent> AIDecisionComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="AOCharacterConfig", meta=(AllowPrivateAccess=true))
	TObjectPtr<UMotionWarpingComponent> MotionWarpingComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AO|Interaction", meta = (AllowPrivateAccess = true))
	TObjectPtr<UBoxComponent> InteractionBounds = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AO|Interaction", meta = (AllowPrivateAccess = true))
	TArray<FInteractionOption> InventoryInteractionOptions;

protected:
	void DisableMovementAndCollision();
	void HandleStateTreeChange();
	bool CanOpenInventoryForInteractor(const APawn* InteractingPawn) const;
	bool IsDeadForInventoryInteraction() const;
	const FInteractionOption* FindInventoryInteractionOptionByIndex(int32 InteractionOptionIndex) const;
protected:
	virtual void PossessedBy(AController* NewController) override;
	virtual void UnPossessed() override;

	virtual void Reset() override;
	virtual void OnRep_Controller() override;
	virtual void OnRep_PlayerState() override;
	virtual void PostInitializeComponents() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	//UFUNCTION(BlueprintCallable, Category="AOCharacter")
	//void TestFunc(UPARAM(ref) int32& InOutValue , bool Testbool);
protected:
	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="AOCharacterConfig")
	TMap<FGameplayTag, EEquipState> TagToEquipState;
private:
	UPROPERTY()
	TObjectPtr<const UAOHealthAttributeSet> HealthAttributes;
	
	EEquipState EquipState;
	
	
	/*等级系统*/
public:
	UFUNCTION(BlueprintCallable, Category = "AO|Level")
	int32 GetCharacterLevel() const { return CharacterLevel; }
	
	UFUNCTION(BlueprintCallable, Category = "AO|Level")
	int32 GetCharacterXP() const { return CharacterXP; }
	
	UFUNCTION(BlueprintCallable, Category = "AO|Level")
	int32 GetAvailableAttributePoints() const { return AvailableAttributePoints; }
	
	void SetCharacterLevel(int32 NewLevel);
	void AddToCharacterLevel(int32 DeltaLevel);
	
	void SetCharacterXP(int32 NewXP);
	void AddToCharacterXP(int32 DeltaXP);
	
	void SetAvailableAttributePoints(int32 NewPoints);
	void AddToAvailableAttributePoints(int32 DeltaPoints);
	
	bool TryLevelUp();
	int32 GetXPRequiredForLevel(int32 Level) const;
	int32 GetAttributePointsForLevel(int32 Level) const;
	
	DECLARE_MULTICAST_DELEGATE_TwoParams(FOnCharacterLevelChanged, int32, int32);
	FOnCharacterLevelChanged OnCharacterLevelChangedDelegate;
	
	DECLARE_MULTICAST_DELEGATE_TwoParams(FOnCharacterXPChanged, int32, int32);
	FOnCharacterXPChanged OnCharacterXPChangedDelegate;
	
	DECLARE_MULTICAST_DELEGATE_TwoParams(FOnAttributePointsChanged, int32, int32);
	FOnAttributePointsChanged OnAttributePointsChangedDelegate;
	
	virtual const UAOPawnData* GetPawnData() const override {return BotPawnData;}
protected:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void OnRep_CharacterLevel(int32 OldLevel);
	
	UFUNCTION()
	void OnRep_CharacterXP(int32 OldXP);
	
	UFUNCTION()
	void OnRep_AvailableAttributePoints(int32 OldPoints);
	
	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_CharacterLevel, Category = "AO|Level")
	int32 CharacterLevel = 1;
	
	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_CharacterXP, Category = "AO|Level")
	int32 CharacterXP = 0;
	
	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_AvailableAttributePoints, Category = "AO|Level")
	int32 AvailableAttributePoints = 0;
	
	/*Interface*/
protected:
	virtual void ApplyDamageToTarget(const FAttackedInfo& AttackedInfo) override;
private:
	UPROPERTY(EditDefaultsOnly , meta = (AllowPrivateAccessor) , Category = "AI|PawnData")
	TObjectPtr<const UAOPawnData> BotPawnData;
};
