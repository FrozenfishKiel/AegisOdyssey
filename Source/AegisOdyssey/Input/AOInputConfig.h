// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTags.h"
#include "AOInputConfig.generated.h"

/**
 * 
 */
class UInputAction;
USTRUCT(BlueprintType)
struct FAOInputAction
{
	GENERATED_BODY()

public:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<const UInputAction> InputAction = nullptr;  //输入的实际Action

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (Categories = "InputTag"))
	FGameplayTag InputTag; //与输入相关联的Tag
};
UCLASS()
class AEGISODYSSEY_API UAOInputConfig : public UDataAsset
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = "Lyra|Pawn")
	const UInputAction* FindNativeInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound = true) const;

	UFUNCTION(BlueprintCallable, Category = "Lyra|Pawn")
	const UInputAction* FindAbilityInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound = true) const;

	/**
	 * 统一按输入标签查询项目当前配置的 InputAction。
	 *
	 * 这层查询的职责不是决定这个输入标签“语义上属于谁”，
	 * 而是单纯把项目里已经配置好的 InputTag -> InputAction 关系稳定暴露出来。
	 *
	 * 技能 UI 后续需要拿这个接口把 SkillSlot 的 InputTag 反查成真实 InputAction，
	 * 再交给 CommonActionWidget 去显示当前玩家实际绑定到哪个物理键。
	 */
	UFUNCTION(BlueprintCallable, Category = "AO|Input")
	const UInputAction* FindInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound = true) const;

public:
	// 由所有者（Owner）使用的输入操作（Input Action）列表。这些输入操作被映射到 Gameplay Tag，且必须进行手动绑定。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (TitleProperty = "InputAction"))
	TArray<FAOInputAction> NativeInputActions;

	// 由所有者（Owner）使用的输入操作列表。这些输入操作映射到 Gameplay Tag，并会自动绑定到具有匹配输入标签（Input Tag）的技能（Abilities）上。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (TitleProperty = "InputAction"))
	TArray<FAOInputAction> AbilityPressedInputActions;
	
	// 特殊情况：单次按下单次触发的信号
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (TitleProperty = "InputAction"))
	TArray<FAOInputAction> AbilityStartInputActions;

	// 松开按下的信号
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (TitleProperty = "InputAction"))
	TArray<FAOInputAction> AbilityReleasedInputActions;
};
