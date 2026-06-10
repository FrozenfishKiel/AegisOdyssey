#include "AOTargetHealthBarComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AegisOdyssey/AbilitySystem/Attributes/Core/AOHealthAttributeSet.h"
#include "AegisOdyssey/UI/ViewModel/MVVM_TargetHealthBar.h"
#include "AOTargetHealthBarWidget.h"
#include "Components/SceneComponent.h"
#include "GameFramework/Character.h"
#include "ModelViewViewModel/Public/MVVMSubsystem.h"
#include "ModelViewViewModel/Public/View/MVVMView.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOTargetHealthBarComponent)

UAOTargetHealthBarComponent::UAOTargetHealthBarComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UAOTargetHealthBarComponent::BeginPlay()
{
	Super::BeginPlay();

	EnsureTargetHealthBarViewModel();
	BindHealthSource();
	EnsureWidgetComponent();
	BindViewModelToWidget();
	RefreshTargetHealthBarViewModel();
	RefreshRenderVisibility();
}

void UAOTargetHealthBarComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnbindHealthSource();
	ClearViewModelFromWidget();

	if (HealthBarWidgetComponent != nullptr)
	{
		HealthBarWidgetComponent->SetVisibility(false, true);
		HealthBarWidgetComponent->SetHiddenInGame(true);
	}

	Super::EndPlay(EndPlayReason);
}

void UAOTargetHealthBarComponent::SetRequestedVisible(bool bVisible)
{
	if (bRequestedVisible == bVisible)
	{
		return;
	}

	bRequestedVisible = bVisible;
	RefreshRenderVisibility();
}

void UAOTargetHealthBarComponent::BindHealthSource()
{
	UnbindHealthSource();

	AActor* OwnerActor = GetOwner();
	if (OwnerActor == nullptr)
	{
		RefreshTargetHealthBarViewModel();
		return;
	}

	CachedAbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OwnerActor);
	if (CachedAbilitySystemComponent == nullptr)
	{
		RefreshTargetHealthBarViewModel();
		return;
	}

	CachedHealthAttributeSet = const_cast<UAOHealthAttributeSet*>(Cast<const UAOHealthAttributeSet>(
		CachedAbilitySystemComponent->GetAttributeSet(UAOHealthAttributeSet::StaticClass())));
	if (CachedHealthAttributeSet == nullptr)
	{
		RefreshTargetHealthBarViewModel();
		return;
	}

	HealthChangedDelegateHandle =
		CachedAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(CachedHealthAttributeSet->GetHealthAttribute())
		.AddUObject(this, &ThisClass::HandleHealthAttributeChanged);

	MaxHealthChangedDelegateHandle =
		CachedAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(CachedHealthAttributeSet->GetMaxHealthAttribute())
		.AddUObject(this, &ThisClass::HandleMaxHealthAttributeChanged);

	bIsDead = CachedHealthAttributeSet->GetHealth() <= KINDA_SMALL_NUMBER;
	RefreshTargetHealthBarViewModel();
}

void UAOTargetHealthBarComponent::UnbindHealthSource()
{
	if (CachedAbilitySystemComponent != nullptr && CachedHealthAttributeSet != nullptr)
	{
		if (HealthChangedDelegateHandle.IsValid())
		{
			CachedAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(CachedHealthAttributeSet->GetHealthAttribute())
				.Remove(HealthChangedDelegateHandle);
			HealthChangedDelegateHandle.Reset();
		}

		if (MaxHealthChangedDelegateHandle.IsValid())
		{
			CachedAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(CachedHealthAttributeSet->GetMaxHealthAttribute())
				.Remove(MaxHealthChangedDelegateHandle);
			MaxHealthChangedDelegateHandle.Reset();
		}
	}

	CachedHealthAttributeSet = nullptr;
	CachedAbilitySystemComponent = nullptr;
}

void UAOTargetHealthBarComponent::EnsureWidgetComponent()
{
	if (GetOwner() == nullptr || GetWorld() == nullptr || GetWorld()->IsNetMode(NM_DedicatedServer))
	{
		return;
	}

	if (HealthBarWidgetComponent == nullptr)
	{
		HealthBarWidgetComponent = NewObject<UWidgetComponent>(GetOwner(), TEXT("TargetHealthBarWidgetComponent"));
		if (HealthBarWidgetComponent == nullptr)
		{
			return;
		}

		GetOwner()->AddInstanceComponent(HealthBarWidgetComponent);
		HealthBarWidgetComponent->SetWidgetSpace(WidgetSpace);
		HealthBarWidgetComponent->SetDrawAtDesiredSize(false);
		HealthBarWidgetComponent->SetDrawSize(WidgetDrawSize);
		HealthBarWidgetComponent->SetVisibility(false, true);
		HealthBarWidgetComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		HealthBarWidgetComponent->SetGenerateOverlapEvents(false);
		HealthBarWidgetComponent->SetHiddenInGame(true);

		if (HealthBarWidgetClass != nullptr)
		{
			HealthBarWidgetComponent->SetWidgetClass(HealthBarWidgetClass);
		}

		HealthBarWidgetComponent->RegisterComponent();

		if (ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner()))
		{
			if (USkeletalMeshComponent* MeshComponent = OwnerCharacter->GetMesh())
			{
				HealthBarWidgetComponent->AttachToComponent(MeshComponent, FAttachmentTransformRules::KeepRelativeTransform);
			}
		}
		else if (USceneComponent* RootComponent = GetOwner()->GetRootComponent())
		{
			HealthBarWidgetComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
		}

		HealthBarWidgetComponent->SetRelativeLocation(WidgetRelativeOffset);
	}

	if (HealthBarWidgetClass != nullptr && HealthBarWidgetComponent->GetWidgetClass() != HealthBarWidgetClass)
	{
		HealthBarWidgetComponent->SetWidgetClass(HealthBarWidgetClass);
	}

	HealthBarWidgetComponent->InitWidget();
}

void UAOTargetHealthBarComponent::EnsureTargetHealthBarViewModel()
{
	if (TargetHealthBarViewModel == nullptr)
	{
		TargetHealthBarViewModel = NewObject<UMVVMTargetHealthBar>(this);
	}
}

void UAOTargetHealthBarComponent::BindViewModelToWidget()
{
	if (HealthBarWidgetComponent == nullptr)
	{
		return;
	}

	HealthBarWidgetComponent->InitWidget();

	if (UUserWidget* UserWidget = HealthBarWidgetComponent->GetUserWidgetObject())
	{
		if (UMVVMView* View = UMVVMSubsystem::GetViewFromUserWidget(UserWidget))
		{
			TScriptInterface<INotifyFieldValueChanged> ViewModelInterface(TargetHealthBarViewModel);
			View->SetViewModel(TargetHealthBarViewModelName, ViewModelInterface);
		}

		if (UAOTargetHealthBarWidget* TargetHealthBarWidget = Cast<UAOTargetHealthBarWidget>(UserWidget))
		{
			TargetHealthBarWidget->SetTargetHealthBarViewModel(TargetHealthBarViewModel);
		}
	}
}

void UAOTargetHealthBarComponent::ClearViewModelFromWidget()
{
	if (HealthBarWidgetComponent == nullptr)
	{
		return;
	}

	HealthBarWidgetComponent->InitWidget();

	if (UUserWidget* UserWidget = HealthBarWidgetComponent->GetUserWidgetObject())
	{
		if (UMVVMView* View = UMVVMSubsystem::GetViewFromUserWidget(UserWidget))
		{
			TScriptInterface<INotifyFieldValueChanged> EmptyViewModelInterface;
			View->SetViewModel(TargetHealthBarViewModelName, EmptyViewModelInterface);
		}

		if (UAOTargetHealthBarWidget* TargetHealthBarWidget = Cast<UAOTargetHealthBarWidget>(UserWidget))
		{
			TargetHealthBarWidget->SetTargetHealthBarViewModel(nullptr);
		}
	}
}

void UAOTargetHealthBarComponent::RefreshTargetHealthBarViewModel()
{
	EnsureTargetHealthBarViewModel();
	if (TargetHealthBarViewModel == nullptr)
	{
		return;
	}

	TargetHealthBarViewModel->SetTargetActor(GetOwner());

	float CurrentHealth = 0.0f;
	float MaxHealth = 0.0f;
	if (CachedHealthAttributeSet != nullptr)
	{
		CurrentHealth = CachedHealthAttributeSet->GetHealth();
		MaxHealth = CachedHealthAttributeSet->GetMaxHealth();
	}

	TargetHealthBarViewModel->SetCurrentHealth(CurrentHealth);
	TargetHealthBarViewModel->SetMaxHealth(MaxHealth);
	TargetHealthBarViewModel->SetDead(bIsDead || CurrentHealth <= KINDA_SMALL_NUMBER);
}

bool UAOTargetHealthBarComponent::ShouldRenderWorldHealthBar() const
{
	return bEnableObservedWorldHealthBar && bRequestedVisible && !bIsDead;
}

void UAOTargetHealthBarComponent::RefreshRenderVisibility()
{
	const bool bShouldRender = ShouldRenderWorldHealthBar();

	if (HealthBarWidgetComponent != nullptr)
	{
		HealthBarWidgetComponent->SetVisibility(bShouldRender, true);
		HealthBarWidgetComponent->SetHiddenInGame(!bShouldRender);
	}

	if (TargetHealthBarViewModel != nullptr)
	{
		TargetHealthBarViewModel->SetVisible(bShouldRender);
	}
}

void UAOTargetHealthBarComponent::HandleHealthAttributeChanged(const FOnAttributeChangeData& ChangeData)
{
	bIsDead = ChangeData.NewValue <= KINDA_SMALL_NUMBER;
	if (bIsDead)
	{
		bRequestedVisible = false;
	}

	RefreshTargetHealthBarViewModel();
	RefreshRenderVisibility();
}

void UAOTargetHealthBarComponent::HandleMaxHealthAttributeChanged(const FOnAttributeChangeData& ChangeData)
{
	RefreshTargetHealthBarViewModel();
	RefreshRenderVisibility();
}
