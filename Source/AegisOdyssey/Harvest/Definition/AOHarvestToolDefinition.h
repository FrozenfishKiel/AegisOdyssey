#pragma once

#include "CoreMinimal.h"
#include "AegisOdyssey/Equipment/AOEquipmentDefinition.h"
#include "AOHarvestToolDefinition.generated.h"

class UAOHarvestToolFragment;
class UAOHarvestToolProfile;

// HarvestToolDefinition 继续留在现有 Item / Equipment 体系里。
// 它负责定义“这件可装备物作为采集工具时的静态边界”，不单独发明一套特立独行的资产体系。
// 真正这把具体工具的运行时个体状态，后续应优先落在 UAOHarvestToolInstance 上。
UCLASS(Blueprintable, Const, Abstract, BlueprintType)
class AEGISODYSSEY_API UAOHarvestToolDefinition : public UAOEquipmentDefinition
{
	GENERATED_BODY()

public:
	UAOHarvestToolDefinition(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// 统一从现有 ItemDefinition 的 Fragment 容器里拿采集配置块，
	// 这样工具采集配置仍然遵守项目当前的 Definition + Fragment 风格。
	const UAOHarvestToolFragment* FindHarvestToolFragment() const;
	const UAOHarvestToolProfile* GetHarvestToolProfile() const { return HarvestToolProfile; }

protected:
	// ToolProfile 负责表达这把工具的机械语义身份。
	// 例如未来是不是斧头、镐子、锤头，都应该通过资产扩展，而不是改 C++ 枚举。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Harvest")
	TObjectPtr<UAOHarvestToolProfile> HarvestToolProfile = nullptr;
};
