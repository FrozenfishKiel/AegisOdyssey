#include "AOHUDViewModelComponent.h"

#include "AbilitySystemComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "AOHUD.h"
#include "AegisOdyssey/AOCombatMessageSubsystem.h"
#include "AegisOdyssey/AbilitySystem/Attributes/Combat/AOCombatAttributeSet.h"
#include "AegisOdyssey/AbilitySystem/Attributes/Core/AOHealthAttributeSet.h"
#include "AegisOdyssey/Character/AOExtPawnComponent.h"
#include "AegisOdyssey/Crafting/Components/AOCraftingComponent.h"
#include "AegisOdyssey/Inventory/AOInventoryMessageSubsystem.h"
#include "AegisOdyssey/Player/AOPlayerController.h"
#include "AegisOdyssey/SkillSystem/Components/AOSkillComponent.h"
#include "ViewModel/AOCombatFeedbackViewData.h"
#include "Common/Inventory/AOInventoryAcquisitionNotification.h"
#include "ViewModel/MVVM_CombatFeedbackFeed.h"
#include "ViewModel/MVVM_CombatResources.h"
#include "ViewModel/MVVM_Crafting.h"
#include "ViewModel/MVVM_HUD.h"
#include "ViewModel/MVVM_ItemHoverTooltip.h"
#include "ViewModel/MVVM_LocalCombatState.h"
#include "ViewModel/MVVM_TargetHealthBarCollection.h"
#include "WorldHealthBar/AOCombatFloatingTextComponent.h"
#include "WorldHealthBar/AOTargetHealthBarComponent.h"
#include "WorldHealthBar/AOLocalTargetHealthBarObserverComponent.h"
#include "TimerManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOHUDViewModelComponent)

UAOHUDViewModelComponent::UAOHUDViewModelComponent()
{
	bWantsInitializeComponent = true;
}

void UAOHUDViewModelComponent::InitializeComponent()
{
	Super::InitializeComponent();
	InitializeAllViewModels();
	BindCombatMessageSource();
	BindInventoryMessageSource();
	CheckDefaultInitialization();
}

void UAOHUDViewModelComponent::UninitializeComponent()
{
	ClearDefaultInitializationRetry();
	UnbindSkillObservationSource();
	UnbindCraftingObservationSource();
	UnbindCombatMessageSource();
	UnbindInventoryMessageSource();
	Super::UninitializeComponent();
}

void UAOHUDViewModelComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UAOHUDViewModelComponent::OnRegister()
{
	Super::OnRegister();
}

void UAOHUDViewModelComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ClearDefaultInitializationRetry();
	UnbindSkillObservationSource();
	UnbindCraftingObservationSource();
	UnbindCombatMessageSource();
	UnbindInventoryMessageSource();
	Super::EndPlay(EndPlayReason);
}

void UAOHUDViewModelComponent::InitializeAllViewModels()
{
	if (!HUDViewModel)
	{
		HUDViewModel = NewObject<UMVVM_HUD>(this);
		HUDViewModel->InitializeChildViewModels();
	}
}

void UAOHUDViewModelComponent::ClearAllViewModels()
{
	UnbindSkillObservationSource();
	UnbindCraftingObservationSource();
	UnbindCombatMessageSource();
	UnbindInventoryMessageSource();
	HUDViewModel = nullptr;
}

void UAOHUDViewModelComponent::SetHUDViewModelParams(const FPlayerMainHUDViewModelParams& PlayerMainHUDViewModelParams)
{
	if (HUDViewModel)
	{
		HUDViewModel->SetPlayerViewModelParams(PlayerMainHUDViewModelParams);
	}
}

UMVVM_CombatResources* UAOHUDViewModelComponent::GetCombatResourcesViewModel() const
{
	return HUDViewModel != nullptr ? HUDViewModel->GetCombatResourcesViewModel() : nullptr;
}

UMVVM_LocalCombatState* UAOHUDViewModelComponent::GetLocalCombatStateViewModel() const
{
	return HUDViewModel != nullptr ? HUDViewModel->GetLocalCombatStateViewModel() : nullptr;
}

UMVVM_CombatFeedbackFeed* UAOHUDViewModelComponent::GetCombatFeedbackFeedViewModel() const
{
	return HUDViewModel != nullptr ? HUDViewModel->GetCombatFeedbackFeedViewModel() : nullptr;
}

UMVVM_TargetHealthBarCollection* UAOHUDViewModelComponent::GetTargetHealthBarCollectionViewModel() const
{
	return HUDViewModel != nullptr ? HUDViewModel->GetTargetHealthBarCollectionViewModel() : nullptr;
}

UMVVM_Crafting* UAOHUDViewModelComponent::GetCraftingViewModel() const
{
	return HUDViewModel != nullptr ? HUDViewModel->GetCraftingViewModel() : nullptr;
}

UMVVM_ItemHoverTooltip* UAOHUDViewModelComponent::GetItemHoverTooltipViewModel() const
{
	return HUDViewModel != nullptr ? HUDViewModel->GetItemHoverTooltipViewModel() : nullptr;
}

void UAOHUDViewModelComponent::CheckDefaultInitialization()
{
	AAOHUD* HUD = Cast<AAOHUD>(GetOwner());
	if (HUD == nullptr)
	{
		ScheduleDefaultInitializationRetry();
		return;
	}

	FPlayerMainHUDViewModelParams SourceDataParams;

	APlayerController* SourcePC = Cast<APlayerController>(HUD->GetOwningPlayerController());
	if (SourcePC == nullptr)
	{
		ScheduleDefaultInitializationRetry();
		return;
	}
	SourceDataParams.PC = TWeakObjectPtr<APlayerController>(SourcePC);

	APawn* ControlledPawn = SourcePC->GetPawn();
	if (APlayerState* PS = SourcePC->PlayerState)
	{
		SourceDataParams.PS = TWeakObjectPtr<APlayerState>(PS);
	}

	if (ControlledPawn == nullptr)
	{
		BindCraftingObservationSource(nullptr);
		SetHUDViewModelParams(SourceDataParams);
		ScheduleDefaultInitializationRetry();
		return;
	}

	if (ControlledPawn != nullptr)
	{
		if (UAOExtPawnComponent* ExtPawnComponent = UAOExtPawnComponent::FindAOExtPawnComponent(ControlledPawn))
		{
			if (UAbilitySystemComponent* SourceASC = ExtPawnComponent->GetAbilitySystemComponent())
			{
				SourceDataParams.ASC = TWeakObjectPtr<UAbilitySystemComponent>(SourceASC);
				if (SourceASC->GetSet<UAOHealthAttributeSet>() == nullptr
					|| SourceASC->GetSet<UAOCombatAttributeSet>() == nullptr)
				{
					ScheduleDefaultInitializationRetry();
				}
				else
				{
					ClearDefaultInitializationRetry();
				}
			}
			else
			{
				ScheduleDefaultInitializationRetry();
			}
		}

		if (UAOSkillComponent* SkillComponent = ControlledPawn->FindComponentByClass<UAOSkillComponent>())
		{
			SourceDataParams.SkillComponent = TWeakObjectPtr<UAOSkillComponent>(SkillComponent);
			BindSkillObservationSource(SkillComponent);
		}

		BindCraftingObservationSource(ControlledPawn->FindComponentByClass<UAOCraftingComponent>());
	}

	SetHUDViewModelParams(SourceDataParams);

	if (HUDViewModel)
	{
		HUDViewModel->RefreshSkillObservationData();
	}
}

void UAOHUDViewModelComponent::ScheduleDefaultInitializationRetry()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			DefaultInitializationRetryTimerHandle,
			this,
			&ThisClass::CheckDefaultInitialization,
			0.1f,
			false);
	}
}

void UAOHUDViewModelComponent::ClearDefaultInitializationRetry()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(DefaultInitializationRetryTimerHandle);
	}
}

void UAOHUDViewModelComponent::OnActorInitStateChanged(const FActorInitStateChangedParams& Params)
{
}

void UAOHUDViewModelComponent::HandleChangeInitState(
	UGameFrameworkComponentManager* Manager,
	FGameplayTag CurrentState,
	FGameplayTag DesiredState)
{
}

void UAOHUDViewModelComponent::BindSkillObservationSource(UAOSkillComponent* SkillComponent)
{
	if (BoundSkillComponent.Get() == SkillComponent && SkillObservationChangedHandle.IsValid())
	{
		return;
	}

	UnbindSkillObservationSource();

	if (!SkillComponent)
	{
		return;
	}

	BoundSkillComponent = SkillComponent;
	SkillObservationChangedHandle = SkillComponent->OnSkillObservationChanged.AddUObject(this, &ThisClass::HandleSkillObservationChanged);
}

void UAOHUDViewModelComponent::UnbindSkillObservationSource()
{
	if (UAOSkillComponent* SkillComponent = BoundSkillComponent.Get())
	{
		if (SkillObservationChangedHandle.IsValid())
		{
			SkillComponent->OnSkillObservationChanged.Remove(SkillObservationChangedHandle);
		}
	}

	SkillObservationChangedHandle.Reset();
	BoundSkillComponent.Reset();
}

void UAOHUDViewModelComponent::HandleSkillObservationChanged()
{
	if (HUDViewModel)
	{
		HUDViewModel->RefreshSkillObservationData();
	}
}

void UAOHUDViewModelComponent::BindCraftingObservationSource(UAOCraftingComponent* CraftingComponent)
{
	if (BoundCraftingComponent.Get() == CraftingComponent)
	{
		return;
	}

	// HUD 这里只负责把当前制造真相源接进 ViewModel。
	// 真正的数据变化监听已经下沉到 CraftingComponent -> MVVM 这条链。
	BoundCraftingComponent = CraftingComponent;

	if (UMVVM_Crafting* CraftingViewModel = GetCraftingViewModel())
	{
		CraftingViewModel->SetObservedCraftingComponent(CraftingComponent);
	}
}

void UAOHUDViewModelComponent::UnbindCraftingObservationSource()
{
	if (UMVVM_Crafting* CraftingViewModel = GetCraftingViewModel())
	{
		CraftingViewModel->SetObservedCraftingComponent(nullptr);
	}

	BoundCraftingComponent.Reset();
}

void UAOHUDViewModelComponent::BindCombatMessageSource()
{
	if (BoundCombatMessageSubsystem != nullptr)
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		if (UAOCombatMessageSubsystem* CombatMessageSubsystem = World->GetSubsystem<UAOCombatMessageSubsystem>())
		{
			BoundCombatMessageSubsystem = CombatMessageSubsystem;
			CombatMessageSubsystem->OnCombatResultMessage.AddUObject(this, &ThisClass::HandleCombatResultMessage);
		}
	}
}

void UAOHUDViewModelComponent::UnbindCombatMessageSource()
{
	if (BoundCombatMessageSubsystem != nullptr)
	{
		BoundCombatMessageSubsystem->OnCombatResultMessage.RemoveAll(this);
		BoundCombatMessageSubsystem = nullptr;
	}
}

void UAOHUDViewModelComponent::BindInventoryMessageSource()
{
	if (BoundInventoryMessageSubsystem != nullptr)
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		if (UAOInventoryMessageSubsystem* InventoryMessageSubsystem = World->GetSubsystem<UAOInventoryMessageSubsystem>())
		{
			BoundInventoryMessageSubsystem = InventoryMessageSubsystem;
			InventoryMessageSubsystem->OnInventoryAcquisitionMessage.AddDynamic(this, &ThisClass::HandleInventoryAcquisitionMessage);
		}
	}
}

void UAOHUDViewModelComponent::UnbindInventoryMessageSource()
{
	if (BoundInventoryMessageSubsystem != nullptr)
	{
		BoundInventoryMessageSubsystem->OnInventoryAcquisitionMessage.RemoveDynamic(this, &ThisClass::HandleInventoryAcquisitionMessage);
		BoundInventoryMessageSubsystem = nullptr;
	}
}

AActor* UAOHUDViewModelComponent::GetLocalPlayerActor() const
{
	if (const AAOHUD* HUD = Cast<AAOHUD>(GetOwner()))
	{
		if (APlayerController* OwningPlayerController = HUD->GetOwningPlayerController())
		{
			return OwningPlayerController->GetPawn();
		}
	}

	return nullptr;
}

bool UAOHUDViewModelComponent::BuildLocalCombatFeedbackViewData(
	const FAOCombatResultMessage& Message,
	FAOCombatFeedbackViewData& OutFeedback) const
{
	const AActor* LocalPlayerActor = GetLocalPlayerActor();
	OutFeedback.ApplyCombatResult(Message, 0, GetWorld() != nullptr ? GetWorld()->GetTimeSeconds() : 0.0f, LocalPlayerActor);
	return OutFeedback.bShouldEnqueueForHUD;
}

void UAOHUDViewModelComponent::HandleCombatResultMessage(const FAOCombatResultMessage& Message)
{
	if (HUDViewModel == nullptr)
	{
		return;
	}

	FAOCombatFeedbackViewData FeedbackViewData;
	if (!BuildLocalCombatFeedbackViewData(Message, FeedbackViewData))
	{
		return;
	}

	HUDViewModel->ApplyCombatFeedbackViewData(FeedbackViewData);

	if (const AAOHUD* HUD = Cast<AAOHUD>(GetOwner()))
	{
		if (AAOPlayerController* OwningPlayerController = Cast<AAOPlayerController>(HUD->GetOwningPlayerController()))
		{
			if (UAOLocalTargetHealthBarObserverComponent* TargetObserverComponent = OwningPlayerController->GetLocalTargetHealthBarObserverComponent())
			{
				TargetObserverComponent->TrackObservedTargetFromCombatFeedback(FeedbackViewData);
			}
		}
	}

	if (FeedbackViewData.bShouldEnqueueForWorldFloatingText)
	{
		if (AActor* TargetActor = FeedbackViewData.Target.Get())
		{
			if (UAOCombatFloatingTextComponent* CombatFloatingTextComponent = TargetActor->FindComponentByClass<UAOCombatFloatingTextComponent>())
			{
				CombatFloatingTextComponent->DisplayWorldCombatFeedback(FeedbackViewData);
			}
		}
	}
}

bool UAOHUDViewModelComponent::BuildLocalInventoryAcquisitionNotification(
	const FAOInventoryAcquisitionMessage& Message,
	FAOInventoryAcquisitionNotification& OutNotification) const
{
	const AActor* LocalPlayerActor = GetLocalPlayerActor();
	OutNotification.ApplyInventoryAcquisition(Message, 0, GetWorld() != nullptr ? GetWorld()->GetTimeSeconds() : 0.0f, LocalPlayerActor);
	return OutNotification.bIsLocalRelevant;
}

void UAOHUDViewModelComponent::HandleInventoryAcquisitionMessage(FAOInventoryAcquisitionMessage Message)
{
	if (HUDViewModel == nullptr)
	{
		return;
	}

	FAOInventoryAcquisitionNotification Notification;
	if (!BuildLocalInventoryAcquisitionNotification(Message, Notification))
	{
		return;
	}

	HUDViewModel->ApplyInventoryAcquisitionNotification(Notification);
}
