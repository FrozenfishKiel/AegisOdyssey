#include "AOHUDViewModelComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AOHUD.h"
#include "AegisOdyssey/AOGameplayTags.h"
#include "AegisOdyssey/Character/AOExtPawnComponent.h"
#include "AegisOdyssey/Character/AOHeroComponent.h"
#include "Components/GameFrameworkComponentDelegates.h"
#include "ViewModel/MVVM_HUD.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOHUDViewModelComponent)

UAOHUDViewModelComponent::UAOHUDViewModelComponent()
{
	bWantsInitializeComponent = true;
}

void UAOHUDViewModelComponent::InitializeComponent()
{
	Super::InitializeComponent();
	InitializeAllViewModels();
}

void UAOHUDViewModelComponent::UninitializeComponent()
{
	Super::UninitializeComponent();
}

void UAOHUDViewModelComponent::BeginPlay()
{
	Super::BeginPlay();
	// 监听PawnExtComp的状态变化，会调用this的OnActorInitStateChanged函数
}

void UAOHUDViewModelComponent::InitializeAllViewModels()
{
	if (!HUDViewModel)
	{
		HUDViewModel = NewObject<UMVVM_HUD>(this);
	}
}

void UAOHUDViewModelComponent::ClearAllViewModels()
{
	if (HUDViewModel)
	{
		HUDViewModel = nullptr;
	}
}

void UAOHUDViewModelComponent::SetHUDViewModelParams(FPlayerMainHUDViewModelParams& PlayerMainHUDViewModelParams)
{
	if (HUDViewModel)
	{
		HUDViewModel->SetPlayerViewModelParams(PlayerMainHUDViewModelParams);
	}
}

void UAOHUDViewModelComponent::CheckDefaultInitialization()
{
	AAOHUD* HUD = Cast<AAOHUD>(GetOwner());
	check(HUD);
	FPlayerMainHUDViewModelParams SourceDataParams;
	APlayerController* SourcePC = Cast<APlayerController>(HUD->GetOwningPlayerController());
	check(SourcePC);
	SourceDataParams.PC = SourcePC;
	APlayerState* PS = SourcePC->PlayerState;
	check(PS);
	SourceDataParams.PS = PS;
	UAOExtPawnComponent* ExtPawnComponent =  UAOExtPawnComponent::FindAOExtPawnComponent(SourcePC->GetPawn());
	UAbilitySystemComponent* SourceASC = ExtPawnComponent->GetAbilitySystemComponent();
	check(SourceASC);
	SourceDataParams.ASC = SourceASC;
	SetHUDViewModelParams(SourceDataParams);

}

void UAOHUDViewModelComponent::HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState,
	FGameplayTag DesiredState)
{
	
}

void UAOHUDViewModelComponent::OnRegister()
{
	Super::OnRegister();
	//RegisterInitStateFeature();  //注册初始化状态链
}

void UAOHUDViewModelComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	//UnregisterInitStateFeature();
}

void UAOHUDViewModelComponent::OnActorInitStateChanged(const FActorInitStateChangedParams& Params)
{
	
}
