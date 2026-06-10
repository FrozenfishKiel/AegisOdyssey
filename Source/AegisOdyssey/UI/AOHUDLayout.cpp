// Fill out your copyright notice in the Description page of Project Settings.


#include "AOHUDLayout.h"

#include "Input/CommonUIInputTypes.h"
#include "AOHUD.h"
#include "AOHUDViewModelComponent.h"
#include "AOInteractionSessionWidget.h"
#include "CommonUIExtensions.h"
#include "AegisOdyssey/AOGameplayTags.h"
#include "AegisOdyssey/Interaction/AOInteractionSessionComponent.h"
#include "AegisOdyssey/Interaction/Session/AOInteractionSessionModel.h"
#include "AegisOdyssey/UI/ViewModel/MVVM_ItemHoverTooltip.h"
#include "AegisOdyssey/UI/Widgets/HUD/AOItemHoverTooltipWidget.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOHUDLayout)

UAOHUDLayout::UAOHUDLayout(const FObjectInitializer& InObjectInitializer)
{
}

void UAOHUDLayout::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	RegisterUIActionBinding(FBindUIActionArgs(
		FUIActionTag::ConvertChecked(AOGameplayTags::UI_ACTION_ESCAPE),
		false,
		FSimpleDelegate::CreateUObject(this, &ThisClass::HandleEscapeAction)));

	RegisterUIActionBinding(FBindUIActionArgs(
		FUIActionTag::ConvertChecked(AOGameplayTags::UI_ACTION_INVENTORY),
		false,
		FSimpleDelegate::CreateUObject(this, &ThisClass::HandleInventoryMenuAction)));

	EnsureItemHoverTooltipWidget();
	BindInteractionSessionComponent();
}

void UAOHUDLayout::BindInteractionSessionComponent()
{
	if (BoundInteractionSessionComponent && InteractionSessionChangedHandle.IsValid())
	{
		BoundInteractionSessionComponent->GetOnCurrentSessionChanged().Remove(InteractionSessionChangedHandle);
		InteractionSessionChangedHandle.Reset();
	}

	BoundInteractionSessionComponent = nullptr;

	if (APlayerController* OwningPlayerController = GetOwningPlayer())
	{
		if (UAOInteractionSessionComponent* SessionComponent = OwningPlayerController->FindComponentByClass<UAOInteractionSessionComponent>())
		{
			BoundInteractionSessionComponent = SessionComponent;
			InteractionSessionChangedHandle = SessionComponent->GetOnCurrentSessionChanged().AddUObject(
				this, &ThisClass::HandleCurrentInteractionSessionChanged);

			HandleCurrentInteractionSessionChanged(SessionComponent->GetCurrentSessionModel());
		}
	}
}

void UAOHUDLayout::HandleEscapeAction()
{
	if (ensure(!EscapeMenuClass.IsNull()))
	{
		UCommonUIExtensions::PushStreamedContentToLayer_ForPlayer(
			GetOwningLocalPlayer(),
			AOGameplayTags::UI_LAYER_MENU,
			EscapeMenuClass);
	}
}

void UAOHUDLayout::HandleInventoryMenuAction()
{
	if (!ensure(!InventoryMenuClass.IsNull()))
	{
		return;
	}

	// 背包界面需要拿到实际实例，后续对象交互才能选择“复用这份已经打开的界面”。
	// 因此这里改为先同步取到类，再走可返回实例的 PushContentToLayer_ForPlayer。
	if (TSubclassOf<UCommonActivatableWidget> InventoryMenuWidgetClass = InventoryMenuClass.LoadSynchronous())
	{
		if (UCommonActivatableWidget* NewWidget = UCommonUIExtensions::PushContentToLayer_ForPlayer(
			GetOwningLocalPlayer(),
			AOGameplayTags::UI_LAYER_MENU,
			InventoryMenuWidgetClass))
		{
			if (UAOInteractionSessionWidget* InteractionWidget = Cast<UAOInteractionSessionWidget>(NewWidget))
			{
				ActiveInventoryMenuWidget = InteractionWidget;
			}
		}
	}
}

void UAOHUDLayout::HandleCurrentInteractionSessionChanged(UAOInteractionSessionModel* NewSessionModel)
{
	// UI 的打开行为发生在玩家客户端，但 push 什么界面由对象侧会话决定。
	if (!NewSessionModel)
	{
		CloseCurrentInteractionWidget();
		return;
	}

	TSubclassOf<UCommonActivatableWidget> SessionWidgetClass = NewSessionModel->GetSessionWidgetClass();
	if (!SessionWidgetClass)
	{
		CloseCurrentInteractionWidget();
		return;
	}

	// 如果对象交互想复用当前已经打开的背包布局，就直接把会话绑定到这份现成实例上。
	if (ActiveInventoryMenuWidget && ActiveInventoryMenuWidget->IsActivated() &&
		ActiveInventoryMenuWidget->GetClass() == SessionWidgetClass)
	{
		ActiveInteractionWidget = ActiveInventoryMenuWidget;
		bIsActiveInteractionWidgetOwnedBySession = false;
		ActiveInteractionWidget->SetInteractionSessionModel(NewSessionModel);
		return;
	}

	// 如果当前已经有同类交互界面，就直接改绑会话，不再重复 push。
	if (ActiveInteractionWidget && ActiveInteractionWidget->IsActivated() &&
		ActiveInteractionWidget->GetClass() == SessionWidgetClass)
	{
		ActiveInteractionWidget->SetInteractionSessionModel(NewSessionModel);
		return;
	}

	CloseCurrentInteractionWidget();

	if (UCommonActivatableWidget* NewWidget = UCommonUIExtensions::PushContentToLayer_ForPlayer(
		GetOwningLocalPlayer(),
		AOGameplayTags::UI_LAYER_MENU,
		SessionWidgetClass))
	{
		if (UAOInteractionSessionWidget* InteractionWidget = Cast<UAOInteractionSessionWidget>(NewWidget))
		{
			ActiveInteractionWidget = InteractionWidget;
			bIsActiveInteractionWidgetOwnedBySession = true;
			InteractionWidget->SetInteractionSessionModel(NewSessionModel);
		}
	}
}

void UAOHUDLayout::CloseCurrentInteractionWidget()
{
	if (!ActiveInteractionWidget)
	{
		return;
	}

	UAOInteractionSessionWidget* WidgetToClose = ActiveInteractionWidget;
	WidgetToClose->SetInteractionSessionModel(nullptr);

	// 只有确实由交互会话主动 push 出来的界面，才在会话结束时一起关掉。
	// 如果只是复用了 Tab 已经打开的背包布局，会话结束时只清掉会话绑定，背包界面本身继续留给玩家。
	if (bIsActiveInteractionWidgetOwnedBySession)
	{
		WidgetToClose->DeactivateWidget();
	}

	ActiveInteractionWidget = nullptr;
	bIsActiveInteractionWidgetOwnedBySession = false;
}

UMVVM_HUD* UAOHUDLayout::GetHUDViewModel() const
{
	if (const ULocalPlayer* OwningLocalPlayer = GetOwningLocalPlayer())
	{
		if (APlayerController* SourcePC = OwningLocalPlayer->GetPlayerController(GetWorld()))
		{
			if (UAOHUDViewModelComponent* HUDViewModelComponent = AAOHUD::FindHUDOwnedComponent<UAOHUDViewModelComponent>(SourcePC))
			{
				return HUDViewModelComponent->GetHUDMVVM();
			}
		}
	}

	return nullptr;
}

UMVVM_ItemHoverTooltip* UAOHUDLayout::GetItemHoverTooltipViewModel() const
{
	if (const UMVVM_HUD* HUDViewModel = GetHUDViewModel())
	{
		return HUDViewModel->GetItemHoverTooltipViewModel();
	}

	return nullptr;
}

void UAOHUDLayout::EnsureItemHoverTooltipWidget()
{
	if (IsValid(ActiveItemHoverTooltipWidget) || ItemHoverTooltipWidgetClass == nullptr || GetOwningPlayer() == nullptr)
	{
		return;
	}

	UMVVM_ItemHoverTooltip* TooltipViewModel = GetItemHoverTooltipViewModel();
	if (TooltipViewModel == nullptr)
	{
		return;
	}

	ActiveItemHoverTooltipWidget = CreateWidget<UAOItemHoverTooltipWidget>(GetOwningPlayer(), ItemHoverTooltipWidgetClass);
	if (ActiveItemHoverTooltipWidget == nullptr)
	{
		return;
	}

	ActiveItemHoverTooltipWidget->SetTooltipViewModel(TooltipViewModel);
	ActiveItemHoverTooltipWidget->InitializeForTooltip();
	ActiveItemHoverTooltipWidget->AddToViewport(1100);
}
