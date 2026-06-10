#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AOItemHoverTooltipWidget.generated.h"

class UBorder;
class UHorizontalBox;
class UImage;
class UTextBlock;
class UVerticalBox;
class UMVVM_ItemHoverTooltip;

// HUD 上全局唯一的物品悬浮信息框 Widget。
// 这一层只消费 Tooltip ViewModel 当前提供的显示快照，不自己追溯库存或制造来源。
UCLASS()
class AEGISODYSSEY_API UAOItemHoverTooltipWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UAOItemHoverTooltipWidget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// 每次 Tooltip 被重新显示或复用时，按当前 ViewModel 快照刷新显示。
	void InitializeForTooltip();

	UFUNCTION(BlueprintCallable, Category = "AO|Item Tooltip")
	void SetTooltipViewModel(UMVVM_ItemHoverTooltip* InViewModel);

	UFUNCTION(BlueprintPure, Category = "AO|Item Tooltip")
	UMVVM_ItemHoverTooltip* GetTooltipViewModel() const { return TooltipViewModel; }

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	// 悬浮信息框本体不能抢鼠标命中，否则会导致来源条目收到 Leave 后反复闪烁。
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	UFUNCTION(BlueprintImplementableEvent, Category = "AO|Item Tooltip")
	void OnTooltipViewModelSet(UMVVM_ItemHoverTooltip* InViewModel);

	void RefreshDisplay();
	void BuildDefaultWidgetTreeIfNeeded();

protected:
	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UBorder> RootBorder = nullptr;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UImage> ItemIconImage = nullptr;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ItemNameText = nullptr;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ItemDescriptionText = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "AO|Item Tooltip", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMVVM_ItemHoverTooltip> TooltipViewModel = nullptr;

	// Tooltip 淡入时长只在当前 Widget 自身暴露，方便直接在 Tooltip UI 蓝图上调节。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AO|Item Tooltip|Animation", meta = (ClampMin = "0.0", UIMin = "0.0", AllowPrivateAccess = "true"))
	float FadeInDurationSeconds = 0.2f;

	// Tooltip 淡出时长只在当前 Widget 自身暴露，不把动画参数扩散到 HUD 或 ViewModel。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AO|Item Tooltip|Animation", meta = (ClampMin = "0.0", UIMin = "0.0", AllowPrivateAccess = "true"))
	float FadeOutDurationSeconds = 0.2f;

private:
	// 把外部注入的 Tooltip ViewModel 注册给 MVVM 视图。
	// 蓝图只消费这份统一快照，不自己回查物品来源。
	void BindViewModelToWidget();
	void BindTooltipSnapshotChanged();
	void UnbindTooltipSnapshotChanged();
	void HandleTooltipSnapshotChanged();
	void BeginFadeIn();
	void BeginFadeOut();
	void ApplyAnimatedOpacity(float InOpacity);
	void RefreshHeaderFromViewModel();
	float ResolveActiveFadeDuration() const;

	// 把悬浮进入时记录的屏幕绝对坐标，转换成玩家屏幕根 Widget 的本地坐标。
	// 当前方案里 Tooltip 弹出后不跟随鼠标，因此这里只在显示刷新时解一次位置。
	FVector2D ResolveTooltipCanvasPosition() const;

private:
	enum class ETooltipFadeState : uint8
	{
		Hidden,
		FadingIn,
		Visible,
		FadingOut
	};

	static const FName TooltipViewModelName;
	FDelegateHandle TooltipSnapshotChangedHandle;
	ETooltipFadeState FadeState = ETooltipFadeState::Hidden;
	float CurrentOpacity = 0.0f;
	float FadeElapsedSeconds = 0.0f;
};
