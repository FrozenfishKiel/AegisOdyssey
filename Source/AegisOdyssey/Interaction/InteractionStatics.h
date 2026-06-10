// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "InteractableTarget.h"
#include "InteractionStatics.generated.h"

/**
 * 交互相关的静态辅助函数集合。
 * 这里不承载业务状态，只负责做交互目标解析、执行和事件数据读取。
 */
UCLASS()
class AEGISODYSSEY_API UInteractionStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UInteractionStatics();

	/** 从交互接口对象中反查所属 Actor。 */
	UFUNCTION(BlueprintCallable, Category = "AO|Interaction")
	static AActor* GetActorFromInteractableTarget(TScriptInterface<IInteractableTarget> InteractableTarget);

	/** 从交互接口对象中提取组件名，供客户端把交互请求转发到服务端时复原目标。 */
	static FName GetComponentNameFromInteractableTarget(TScriptInterface<IInteractableTarget> InteractableTarget);

	/** 根据 Actor 与可选组件名，在服务端重新解析交互目标。 */
	static TScriptInterface<IInteractableTarget> ResolveInteractableTarget(AActor* TargetActor, FName TargetComponentName);

	/** 从一次命中结果中提取可交互的 Actor 或组件目标。 */
	static void AppendInteractableTargetsFromHitResult(const FHitResult& HitResult, TArray<TScriptInterface<IInteractableTarget>>& OutInteractableTargets);

	/** 按统一交互入口执行目标对象的交互逻辑。 */
	static bool TryExecuteInteraction(TScriptInterface<IInteractableTarget> InteractableTarget, const FGameplayTag& InteractionEventTag, FGameplayEventData& Payload);

	/** 从交互事件数据中解析当前被触发的交互选项索引。 */
	static int32 GetInteractionOptionIndexFromEventData(const FGameplayEventData& Payload);
};
