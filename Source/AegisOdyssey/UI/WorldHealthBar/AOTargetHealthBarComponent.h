#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/WidgetComponent.h"
#include "AegisOdyssey/AbilitySystem/Attributes/Core/AOHealthAttributeSet.h"
#include "AOTargetHealthBarComponent.generated.h"

class UAbilitySystemComponent;
class UAOHealthAttributeSet;
class UMVVMTargetHealthBar;
class UWidgetComponent;
struct FOnAttributeChangeData;

// 挂在目标身上的世界血条组件。
// 它管理目标自己的生命值真相、血条 ViewModel，以及目标侧世界表现入口。
UCLASS(ClassGroup = (AO), meta = (BlueprintSpawnableComponent))
class AEGISODYSSEY_API UAOTargetHealthBarComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAOTargetHealthBarComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintCallable, Category = "AO|Target Health Bar")
	void SetRequestedVisible(bool bVisible);

	UFUNCTION(BlueprintPure, Category = "AO|Target Health Bar")
	bool IsRequestedVisible() const { return bRequestedVisible; }

	UFUNCTION(BlueprintPure, Category = "AO|Target Health Bar")
	bool IsDead() const { return bIsDead; }

	UFUNCTION(BlueprintPure, Category = "AO|Target Health Bar")
	bool IsWorldHealthBarEnabled() const { return bEnableObservedWorldHealthBar; }

	UFUNCTION(BlueprintPure, Category = "AO|Target Health Bar")
	UWidgetComponent* GetWidgetComponent() const { return HealthBarWidgetComponent; }

	UFUNCTION(BlueprintPure, Category = "AO|Target Health Bar")
	UMVVMTargetHealthBar* GetTargetHealthBarViewModel() const { return TargetHealthBarViewModel; }

protected:
	void BindHealthSource();
	void UnbindHealthSource();
	void EnsureWidgetComponent();
	void EnsureTargetHealthBarViewModel();
	void BindViewModelToWidget();
	void ClearViewModelFromWidget();
	void RefreshTargetHealthBarViewModel();
	void RefreshRenderVisibility();
	bool ShouldRenderWorldHealthBar() const;
	void HandleHealthAttributeChanged(const FOnAttributeChangeData& ChangeData);
	void HandleMaxHealthAttributeChanged(const FOnAttributeChangeData& ChangeData);

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AO|Target Health Bar", meta = (AllowPrivateAccess = "true"))
	bool bEnableObservedWorldHealthBar = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AO|Target Health Bar", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UUserWidget> HealthBarWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AO|Target Health Bar", meta = (AllowPrivateAccess = "true"))
	FName TargetHealthBarViewModelName = TEXT("TargetHealthBar");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AO|Target Health Bar", meta = (AllowPrivateAccess = "true"))
	EWidgetSpace WidgetSpace = EWidgetSpace::Screen;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AO|Target Health Bar", meta = (AllowPrivateAccess = "true"))
	FVector WidgetRelativeOffset = FVector(0.0f, 0.0f, 120.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AO|Target Health Bar", meta = (AllowPrivateAccess = "true", ClampMin = "1", UIMin = "1"))
	FIntPoint WidgetDrawSize = FIntPoint(180, 24);

	UPROPERTY(Transient)
	TObjectPtr<UAbilitySystemComponent> CachedAbilitySystemComponent = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UAOHealthAttributeSet> CachedHealthAttributeSet = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UWidgetComponent> HealthBarWidgetComponent = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UMVVMTargetHealthBar> TargetHealthBarViewModel = nullptr;

	UPROPERTY(Transient)
	bool bRequestedVisible = false;

	UPROPERTY(Transient)
	bool bIsDead = false;

	FDelegateHandle HealthChangedDelegateHandle;
	FDelegateHandle MaxHealthChangedDelegateHandle;
};
