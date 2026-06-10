#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/NoExportTypes.h"
#include "AOInventoryItemDefinition.generated.h"

class UAOInventoryItemInstance;

/**
 * 物品碎片基类。用于给物品实例附加可组合的额外能力或数据。
 */
UCLASS(DefaultToInstanced, EditInlineNew, Abstract)
class AEGISODYSSEY_API UAOInventoryItemFragment : public UObject
{
	GENERATED_BODY()

public:
	virtual void OnInstanceCreated(UAOInventoryItemInstance* InItemInstance) const {}
};

UCLASS(BlueprintType, Blueprintable)
class AEGISODYSSEY_API UAOInventoryItemDefinition : public UObject
{
	GENERATED_BODY()

public:
	UAOInventoryItemDefinition(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
	FName DisplayName;

	// 物品的完整描述原文。
	// 这里不在逻辑层做截断、摘要或手动换行，长文本怎么显示交给 Tooltip 的表现层处理。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
	FText Description;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
	FGameplayTagContainer SemanticTags;

	// 实例类型，优先从 CDO 中获取。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
	TSubclassOf<UAOInventoryItemInstance> PreferredInstanceType = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
	TArray<TObjectPtr<UAOInventoryItemFragment>> Fragments;

public:
	UFUNCTION(BlueprintCallable, Category = "Components", meta = (DeterminesOutputType = "InItemClass"))
	UAOInventoryItemFragment* FindFragmentByClass(const TSubclassOf<UAOInventoryItemFragment> InItemClass) const;

	UFUNCTION(BlueprintPure, Category = "Components")
	bool HasSemanticTag(FGameplayTag Tag, bool bExactMatch = false) const;

	virtual TSubclassOf<UAOInventoryItemInstance> GetPreferredInstanceType() const;

	// 在同一个入口里统一解析最终要生成的 Instance 类。
	// 如果上层传入了显式 override，就使用 override；否则回到 Definition 自己的默认规则。
	static TSubclassOf<UAOInventoryItemInstance> ResolveItemInstanceClass(
		TSubclassOf<UAOInventoryItemDefinition> ItemDefinitionClass,
		TSubclassOf<UAOInventoryItemInstance> ItemInstanceOverrideClass = nullptr);

	template<typename T>
	const T* FindFragmentByClass() const;

	virtual bool IsSupportedForNetworking() const override { return true; }
};

template <typename T>
const T* UAOInventoryItemDefinition::FindFragmentByClass() const
{
	static_assert(TPointerIsConvertibleFromTo<T, const UAOInventoryItemFragment>::Value);
	return (T*)FindFragmentByClass(T::StaticClass());
}

UCLASS()
class AEGISODYSSEY_API UAOBlueprintItemLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, meta = (DeterminesOutputType = FragmentClass))
	static const UAOInventoryItemFragment* FindFragmentByClass(
		TSubclassOf<UAOInventoryItemDefinition> ItemDef,
		TSubclassOf<UAOInventoryItemFragment> FragmentClass);
};
