#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AOTargetHealthBarWidget.generated.h"

class UMVVMTargetHealthBar;

// 目标世界血条的基础 Widget。
// 运行时只接两类输入：目标血条 ViewModel，以及已经完成本地路由的战斗反馈。
UCLASS(Abstract, Blueprintable)
class AEGISODYSSEY_API UAOTargetHealthBarWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "AO|Target Health Bar")
	void SetTargetHealthBarViewModel(UMVVMTargetHealthBar* InViewModel);

	UFUNCTION(BlueprintPure, Category = "AO|Target Health Bar")
	UMVVMTargetHealthBar* GetTargetHealthBarViewModel() const { return TargetHealthBarViewModel; }

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "AO|Target Health Bar")
	void OnTargetHealthBarViewModelSet(UMVVMTargetHealthBar* InViewModel);

private:
	UPROPERTY(BlueprintReadOnly, Category = "AO|Target Health Bar", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMVVMTargetHealthBar> TargetHealthBarViewModel = nullptr;
};
