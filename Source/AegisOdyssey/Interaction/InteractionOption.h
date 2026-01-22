// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "InteractionOption.generated.h"

class IInteractableTarget;
class UUserWidget;

//交互的对象Option
USTRUCT(BlueprintType)
struct FInteractionOption
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite)
	TScriptInterface<IInteractableTarget> InteractableTarget;

	UPROPERTY(EditAnywhere , BlueprintReadWrite)
	FText Text;

	UPROPERTY(EditAnywhere , BlueprintReadWrite)
	FText SubText;

	UPROPERTY(EditAnywhere , BlueprintReadWrite)
	TSubclassOf<UGameplayAbility> InteractionAbilityToGrant;  //交互某些道具时希望瞬间作用给Actor的技能模板
	
	UPROPERTY(BlueprintReadOnly)
	FGameplayAbilitySpecHandle TargetInteractionAbilityHandle;


	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UAbilitySystemComponent> TargetAbilitySystem;

	UPROPERTY(EditAnywhere , BlueprintReadWrite)
	TSoftClassPtr<UUserWidget> InteractionWidgetClass;

public:
	//两个FInteractionOption对象的对比
	FORCEINLINE bool operator==(const FInteractionOption& Other) const
	{
		return InteractableTarget == Other.InteractableTarget &&
			InteractionAbilityToGrant == Other.InteractionAbilityToGrant&&
			TargetAbilitySystem == Other.TargetAbilitySystem &&
			TargetInteractionAbilityHandle == Other.TargetInteractionAbilityHandle &&
			InteractionWidgetClass == Other.InteractionWidgetClass &&
			Text.IdenticalTo(Other.Text) &&
			SubText.IdenticalTo(Other.SubText);
	}

	FORCEINLINE bool operator!=(const FInteractionOption& Other) const
	{
		return !operator==(Other);
	}

	FORCEINLINE bool operator<(const FInteractionOption& Other) const
	{
		return InteractableTarget.GetInterface() < Other.InteractableTarget.GetInterface();
	}
};