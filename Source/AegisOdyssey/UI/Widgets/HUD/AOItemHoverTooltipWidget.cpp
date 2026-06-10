#include "AOItemHoverTooltipWidget.h"

#include "AegisOdyssey/UI/ViewModel/MVVM_ItemHoverTooltip.h"
#include "Blueprint/WidgetTree.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/Border.h"
#include "Components/HorizontalBox.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Input/Events.h"
#include "InputCoreTypes.h"
#include "ModelViewViewModel/Public/MVVMSubsystem.h"
#include "ModelViewViewModel/Public/View/MVVMView.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOItemHoverTooltipWidget)

const FName UAOItemHoverTooltipWidget::TooltipViewModelName(TEXT("ItemHoverTooltip"));

UAOItemHoverTooltipWidget::UAOItemHoverTooltipWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Tooltip 只负责显示，不参与鼠标命中，否则会把来源格子的 Hover 链打断。
	SetVisibility(ESlateVisibility::HitTestInvisible);
}

void UAOItemHoverTooltipWidget::InitializeForTooltip()
{
	RefreshDisplay();
}

void UAOItemHoverTooltipWidget::SetTooltipViewModel(UMVVM_ItemHoverTooltip* InViewModel)
{
	UnbindTooltipSnapshotChanged();
	TooltipViewModel = InViewModel;
	OnTooltipViewModelSet(TooltipViewModel);
	BindViewModelToWidget();
	BindTooltipSnapshotChanged();
}

void UAOItemHoverTooltipWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	BuildDefaultWidgetTreeIfNeeded();
	BindViewModelToWidget();
	BindTooltipSnapshotChanged();
	RefreshDisplay();
}

void UAOItemHoverTooltipWidget::NativeDestruct()
{
	UnbindTooltipSnapshotChanged();
	Super::NativeDestruct();
}

void UAOItemHoverTooltipWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (FadeState != ETooltipFadeState::FadingIn && FadeState != ETooltipFadeState::FadingOut)
	{
		return;
	}

	const float FadeDurationSeconds = ResolveActiveFadeDuration();
	FadeElapsedSeconds = FMath::Min(FadeElapsedSeconds + InDeltaTime, FadeDurationSeconds);
	const float Alpha = FadeDurationSeconds > KINDA_SMALL_NUMBER ? FadeElapsedSeconds / FadeDurationSeconds : 1.0f;

	if (FadeState == ETooltipFadeState::FadingIn)
	{
		ApplyAnimatedOpacity(Alpha);
		if (FadeElapsedSeconds >= FadeDurationSeconds)
		{
			FadeState = ETooltipFadeState::Visible;
		}
		return;
	}

	ApplyAnimatedOpacity(1.0f - Alpha);
	if (FadeElapsedSeconds >= FadeDurationSeconds)
	{
		FadeState = ETooltipFadeState::Hidden;
		SetVisibility(ESlateVisibility::Collapsed);
	}
}

FReply UAOItemHoverTooltipWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	return FReply::Unhandled();
}

void UAOItemHoverTooltipWidget::RefreshDisplay()
{
	if (TooltipViewModel == nullptr)
	{
		FadeState = ETooltipFadeState::Hidden;
		FadeElapsedSeconds = 0.0f;
		ApplyAnimatedOpacity(0.0f);
		SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	if (TooltipViewModel->IsTooltipVisible())
	{
		RefreshHeaderFromViewModel();

		// 进入显示态前先把这次快照内容和位置更新到位，
		// 避免淡入过程中还残留上一条 Tooltip 的旧文本或旧坐标。
		ForceLayoutPrepass();

		if (RootBorder != nullptr)
		{
			RootBorder->SetRenderTranslation(ResolveTooltipCanvasPosition());
		}
		else
		{
			SetPositionInViewport(ResolveTooltipCanvasPosition(), false);
		}

		BeginFadeIn();
		return;
	}

	if (FadeState != ETooltipFadeState::Hidden && FadeState != ETooltipFadeState::FadingOut)
	{
		BeginFadeOut();
	}
}

void UAOItemHoverTooltipWidget::BuildDefaultWidgetTreeIfNeeded()
{
	// 如果宿主蓝图已经提供了完整的 Tooltip 结构，就完全尊重蓝图。
	// 只有在当前类直接被拿来用、并且还没有任何 WidgetTree 时，才补一个最小可见闭环。
	if (WidgetTree == nullptr || WidgetTree->RootWidget != nullptr)
	{
		return;
	}

	RootBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("TooltipRootBorder"));
	WidgetTree->RootWidget = RootBorder;
	RootBorder->SetPadding(FMargin(12.0f));
	RootBorder->SetBrushColor(FLinearColor(0.05f, 0.05f, 0.05f, 0.92f));

	UVerticalBox* RootVerticalBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("TooltipRootVerticalBox"));
	RootBorder->SetContent(RootVerticalBox);

	UHorizontalBox* HeaderBox = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("TooltipHeaderBox"));
	RootVerticalBox->AddChildToVerticalBox(HeaderBox);

	ItemIconImage = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("TooltipItemIconImage"));
	ItemIconImage->SetVisibility(ESlateVisibility::Collapsed);
	HeaderBox->AddChildToHorizontalBox(ItemIconImage);

	ItemNameText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("TooltipItemNameText"));
	ItemNameText->SetAutoWrapText(true);
	HeaderBox->AddChildToHorizontalBox(ItemNameText);

	ItemDescriptionText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("TooltipItemDescriptionText"));
	ItemDescriptionText->SetAutoWrapText(true);
	RootVerticalBox->AddChildToVerticalBox(ItemDescriptionText);
}

void UAOItemHoverTooltipWidget::BindViewModelToWidget()
{
	if (TooltipViewModel == nullptr)
	{
		return;
	}

	if (UMVVMView* View = UMVVMSubsystem::GetViewFromUserWidget(this))
	{
		TScriptInterface<INotifyFieldValueChanged> ViewModelInterface(TooltipViewModel);
		View->SetViewModel(TooltipViewModelName, ViewModelInterface);
	}

	TooltipViewModel->BroadcastCurrentSnapshot();
}

void UAOItemHoverTooltipWidget::BindTooltipSnapshotChanged()
{
	if (TooltipViewModel == nullptr || TooltipSnapshotChangedHandle.IsValid())
	{
		return;
	}

	TooltipSnapshotChangedHandle = TooltipViewModel->OnTooltipSnapshotChanged().AddUObject(this, &ThisClass::HandleTooltipSnapshotChanged);
}

void UAOItemHoverTooltipWidget::UnbindTooltipSnapshotChanged()
{
	if (TooltipViewModel != nullptr && TooltipSnapshotChangedHandle.IsValid())
	{
		TooltipViewModel->OnTooltipSnapshotChanged().Remove(TooltipSnapshotChangedHandle);
	}

	TooltipSnapshotChangedHandle.Reset();
}

void UAOItemHoverTooltipWidget::HandleTooltipSnapshotChanged()
{
	RefreshDisplay();
}

void UAOItemHoverTooltipWidget::BeginFadeIn()
{
	SetVisibility(ESlateVisibility::HitTestInvisible);

	// 如果旧 Tooltip 正在淡出，这里直接从当前透明度反向接回淡入，
	// 保证新悬浮不会先被旧淡出清空一帧，再重新弹出来。
	const float StartOpacity =
		(FadeState == ETooltipFadeState::FadingOut || FadeState == ETooltipFadeState::Visible)
			? CurrentOpacity
			: 0.0f;

	const float FadeDurationSeconds = FMath::Max(0.0f, FadeInDurationSeconds);
	ApplyAnimatedOpacity(StartOpacity);
	FadeElapsedSeconds = FadeDurationSeconds * FMath::Clamp(StartOpacity, 0.0f, 1.0f);

	if (FadeDurationSeconds <= KINDA_SMALL_NUMBER)
	{
		ApplyAnimatedOpacity(1.0f);
		FadeState = ETooltipFadeState::Visible;
		return;
	}

	FadeState = ETooltipFadeState::FadingIn;
}

void UAOItemHoverTooltipWidget::BeginFadeOut()
{
	// 隐藏前先走一段淡出，而不是立刻 Collapsed，
	// 这样 Tooltip 在鼠标离开时不会生硬闪断。
	const float FadeDurationSeconds = FMath::Max(0.0f, FadeOutDurationSeconds);
	FadeElapsedSeconds = FadeDurationSeconds * FMath::Clamp(1.0f - CurrentOpacity, 0.0f, 1.0f);
	SetVisibility(ESlateVisibility::HitTestInvisible);

	if (FadeDurationSeconds <= KINDA_SMALL_NUMBER)
	{
		ApplyAnimatedOpacity(0.0f);
		FadeState = ETooltipFadeState::Hidden;
		SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	FadeState = ETooltipFadeState::FadingOut;
}

void UAOItemHoverTooltipWidget::ApplyAnimatedOpacity(float InOpacity)
{
	CurrentOpacity = FMath::Clamp(InOpacity, 0.0f, 1.0f);
	SetRenderOpacity(CurrentOpacity);
}

float UAOItemHoverTooltipWidget::ResolveActiveFadeDuration() const
{
	switch (FadeState)
	{
	case ETooltipFadeState::FadingIn:
		return FMath::Max(0.0f, FadeInDurationSeconds);
	case ETooltipFadeState::FadingOut:
		return FMath::Max(0.0f, FadeOutDurationSeconds);
	default:
		return 0.0f;
	}
}

void UAOItemHoverTooltipWidget::RefreshHeaderFromViewModel()
{
	if (TooltipViewModel == nullptr)
	{
		return;
	}

	if (ItemNameText != nullptr)
	{
		ItemNameText->SetText(TooltipViewModel->GetItemDisplayName());
	}

	if (ItemDescriptionText != nullptr)
	{
		ItemDescriptionText->SetText(TooltipViewModel->GetItemDescription());
	}

	if (ItemIconImage != nullptr)
	{
		ItemIconImage->SetBrush(TooltipViewModel->GetItemIconBrush());
		ItemIconImage->SetVisibility(
			TooltipViewModel->HasValidItemIcon() ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}
}

FVector2D UAOItemHoverTooltipWidget::ResolveTooltipCanvasPosition() const
{
	if (TooltipViewModel == nullptr || GetOwningPlayer() == nullptr)
	{
		return FVector2D::ZeroVector;
	}

	const FGeometry PlayerScreenGeometry = UWidgetLayoutLibrary::GetPlayerScreenWidgetGeometry(GetOwningPlayer());
	return PlayerScreenGeometry.AbsoluteToLocal(TooltipViewModel->GetScreenSpacePosition());
}
