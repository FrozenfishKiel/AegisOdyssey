#pragma once

#include "CoreMinimal.h"
#include "AOMVVMViewModelBase.h"
#include "MVVM_TargetHealthBarCollection.generated.h"

class UAOLocalTargetHealthBarObserverComponent;

// 本地玩家视角下的目标血条观察集合 ViewModel。
// 它当前只暴露“观察集合入口对象”，不复制每个目标自己的血量真相。
UCLASS(Blueprintable)
class AEGISODYSSEY_API UMVVM_TargetHealthBarCollection : public UAOMVVMViewModelBase
{
	GENERATED_BODY()

public:
	UMVVM_TargetHealthBarCollection(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	void SetObserverComponent(UAOLocalTargetHealthBarObserverComponent* InObserverComponent);

	UFUNCTION(BlueprintPure, FieldNotify, Category = "AO|Target Health Bar")
	UAOLocalTargetHealthBarObserverComponent* GetObserverComponent() const { return ObserverComponent; }

private:
	// 本地观察者侧组件入口。
	// 集合型 ViewModel 只把“观察集合归谁管理”这件事暴露清楚，不缓存目标自己的血量真相。
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter = GetObserverComponent, Category = "AO|Target Health Bar", meta = (AllowPrivateAccess))
	TObjectPtr<UAOLocalTargetHealthBarObserverComponent> ObserverComponent = nullptr;
};
