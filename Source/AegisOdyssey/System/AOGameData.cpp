#include "AOGameData.h"

#include "AOAssetManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOGameData)

UAOGameData::UAOGameData()
{
	
}

inline const UAOGameData& UAOGameData::Get()
{
	// 临时解决方案：如果AOGameData为空，返回一个默认实例
	// 这样编辑器就能正常启动，你可以在编辑器中配置正确的资产路径
	UAOAssetManager& AssetManager = UAOAssetManager::Get();
	
	// 检查AOGameData是否为空
	if (AssetManager.AOGameData.IsNull())
	{
		// 创建一个临时的默认AOGameData对象
		static UAOGameData* DefaultGameData = NewObject<UAOGameData>();
		return *DefaultGameData;
	}
	
	return AssetManager.GetGameData();
}
