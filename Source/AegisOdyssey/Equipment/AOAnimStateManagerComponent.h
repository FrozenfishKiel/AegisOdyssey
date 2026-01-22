#pragma once

#include "CoreMinimal.h"
#include "Components/PawnComponent.h"
#include "AOAnimStateManagerComponent.generated.h"

class UAOAnimInstance;
/*介绍：这是一个用于获取角色当前状态的一个管理器组件，该组件主要是管理一个数据文件，这个数据文件包含角色的 状态标签-动画对
 * 在配置文件里可以随意配置标签所对应的动画或多个动画
 * 然后可以在此组件中返回，因为是属于角色（包括抽象的角色）的组件，所以只继承于PawnComponent
 */

UCLASS(MinimalAPI , BlueprintType , meta = (BlueprintSpawnableComponent))
class UAOAnimStateManagerComponent : public UPawnComponent
{
	GENERATED_BODY()
	UAOAnimStateManagerComponent(const FObjectInitializer& ObjectInitializer);
public:
	UFUNCTION(BlueprintCallable)
	const UAOAnimInstance* GetCurrentStateAnimInstance() const;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void LinkPlayerAnimation();
};
