// Fill out your copyright notice in the Description page of Project Settings.


#include "AOExtPawnComponent.h"

#include "AOCharacter.h"
#include "AOHeroComponent.h"
#include "AegisOdyssey/AOGameplayTags.h"
#include "AegisOdyssey/AbilitySystem/Attributes/Core/AOPrimaryAttributeSet.h"
#include "AegisOdyssey/GameFeatures/GF_AddAbilities.h"
#include "Components/GameFrameworkComponentManager.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOExtPawnComponent)

const FName UAOExtPawnComponent::NAME_ActorFeatureName("PawnExtension");

UAOExtPawnComponent::UAOExtPawnComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;

	SetIsReplicatedByDefault(true);

	DefaultPawnData = nullptr;
	AbilitySystemComponent = nullptr;
}

void UAOExtPawnComponent::CheckDefaultInitialization()
{
	CheckDefaultInitializationForImplementers();

	static const TArray<FGameplayTag> StateChain =
	{
		AOGameplayTags::InitState_Spawned,
		AOGameplayTags::InitState_DataAvailable,
		AOGameplayTags::InitState_DataInitialized,
		AOGameplayTags::InitState_GameplayReady
	};

	ContinueInitStateChain(StateChain);
}

bool UAOExtPawnComponent::CanChangeInitState(
	UGameFrameworkComponentManager* Manager,
	FGameplayTag CurrentState,
	FGameplayTag DesiredState) const
{
	check(Manager);

	APawn* Pawn = GetPawn<APawn>();
	if (!CurrentState.IsValid() && DesiredState == AOGameplayTags::InitState_Spawned)
	{
		return Pawn != nullptr;
	}

	if (CurrentState == AOGameplayTags::InitState_Spawned
		&& DesiredState == AOGameplayTags::InitState_DataAvailable)
	{
		if (!DefaultPawnData || Pawn == nullptr)
		{
			return false;
		}

		if (Pawn->HasAuthority() && !GetController<AController>())
		{
			return false;
		}

		return true;
	}

	if (CurrentState == AOGameplayTags::InitState_DataAvailable
		&& DesiredState == AOGameplayTags::InitState_DataInitialized)
	{
		return Manager->HaveAllFeaturesReachedInitState(Pawn, AOGameplayTags::InitState_DataAvailable);
	}

	if (CurrentState == AOGameplayTags::InitState_DataInitialized
		&& DesiredState == AOGameplayTags::InitState_GameplayReady)
	{
		if (UAOHeroComponent* HeroComp = GetOwner()->FindComponentByClass<UAOHeroComponent>())
		{
			HeroComp->CheckDefaultInitialization();
		}

		return true;
	}

	return false;
}

void UAOExtPawnComponent::HandleChangeInitState(
	UGameFrameworkComponentManager* Manager,
	FGameplayTag CurrentState,
	FGameplayTag DesiredState)
{
	IGameFrameworkInitStateInterface::HandleChangeInitState(Manager, CurrentState, DesiredState);
}

void UAOExtPawnComponent::OnActorInitStateChanged(const FActorInitStateChangedParams& Params)
{
	if (Params.FeatureName != NAME_ActorFeatureName
		&& Params.FeatureState == AOGameplayTags::InitState_DataAvailable)
	{
		CheckDefaultInitialization();
	}
}

void UAOExtPawnComponent::SetupPlayerInputComponent()
{
	CheckDefaultInitialization();
}

void UAOExtPawnComponent::OnRegister()
{
	Super::OnRegister();

	const APawn* Pawn = GetPawn<APawn>();
	ensureAlwaysMsgf(Pawn != nullptr, TEXT("LyraPawnExtensionComponent on [%s] can only be added to Pawn actors."), *GetNameSafe(GetOwner()));

	TArray<UActorComponent*> PawnExtensionComponents;
	if (Pawn)
	{
		Pawn->GetComponents(UAOExtPawnComponent::StaticClass(), PawnExtensionComponents);
	}

	ensureAlwaysMsgf(PawnExtensionComponents.Num() == 1, TEXT("Only one LyraPawnExtensionComponent should exist on [%s]."), *GetNameSafe(GetOwner()));

	RegisterInitStateFeature();
}

void UAOExtPawnComponent::BeginPlay()
{
	Super::BeginPlay();

	BindOnActorInitStateChanged(NAME_None, FGameplayTag(), false);

	ensure(TryToChangeInitState(AOGameplayTags::InitState_Spawned));
	CheckDefaultInitialization();
}

void UAOExtPawnComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ClearAbilitySystemInitializationFinalizeTimer();
	UnbindCharacterLevelChanged();
	Super::EndPlay(EndPlayReason);
	UnregisterInitStateFeature();
}

void UAOExtPawnComponent::OnRep_PawnData()
{
	CheckDefaultInitialization();
}

void UAOExtPawnComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UAOExtPawnComponent, DefaultPawnData);
}

void UAOExtPawnComponent::HandleControllerChange()
{
	if (AbilitySystemComponent && (AbilitySystemComponent->GetAvatarActor() == GetPawnChecked<APawn>()))
	{
		ensure(AbilitySystemComponent->AbilityActorInfo->OwnerActor == AbilitySystemComponent->GetOwnerActor());

		if (AbilitySystemComponent->GetOwnerActor() == nullptr)
		{
			UninitializeAbilitySystem();
		}
		else
		{
			AbilitySystemComponent->RefreshAbilityActorInfo();
		}
	}

	CheckDefaultInitialization();
}

void UAOExtPawnComponent::HandlePlayerStateReplicated()
{
	CheckDefaultInitialization();
}

void UAOExtPawnComponent::InitializeAbilitySystem(UAOAbilitySystem* InASC, AActor* InActor)
{
	check(InASC);
	check(InActor);

	if (AbilitySystemComponent == InASC)
	{
		return;
	}

	if (AbilitySystemComponent)
	{
		UninitializeAbilitySystem();
	}

	APawn* Pawn = GetPawn<APawn>();
	AbilitySystemComponent = InASC;
	bAbilitySystemLevelBindingReady = false;
	bAbilitySystemInitializationFinalized = false;
	AbilitySystemComponent->OnAbilitySystemDataChanged().AddUObject(this, &ThisClass::HandleAbilitySystemDataChanged);
	GrantPawnDataAbilitySets();
	AbilitySystemComponent->InitAbilityActorInfo(InActor, Pawn);
	ScheduleAbilitySystemInitializationFinalize();
}

void UAOExtPawnComponent::UninitializeAbilitySystem()
{
	if (!AbilitySystemComponent)
	{
		return;
	}

	UnbindCharacterLevelChanged();
	ClearAbilitySystemInitializationFinalizeTimer();
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->OnAbilitySystemDataChanged().RemoveAll(this);
	}
	PawnDataGrantedHandles.TakeFromAbilitySystem(AbilitySystemComponent);
	AbilitySystemComponent = nullptr;
	bAbilitySystemLevelBindingReady = false;
	bAbilitySystemInitializationFinalized = false;
}

void UAOExtPawnComponent::SetDataPawn(const UAOPawnData* InPawnData)
{
	APawn* Pawn = GetPawnChecked<APawn>();
	DefaultPawnData = InPawnData;

	CheckDefaultInitialization();
	Pawn->ForceNetUpdate();
}

void UAOExtPawnComponent::GrantPawnDataAbilitySets()
{
	check(AbilitySystemComponent);

	if (!AbilitySystemComponent->IsOwnerActorAuthoritative())
	{
		return;
	}

	PawnDataGrantedHandles = FAOAbilitySet_GrantedHandles();
	for (const TObjectPtr<UAOAbilitySet>& SetPtr : DefaultPawnData->AbilitySets)
	{
		SetPtr->GiveToAbilitySystem(AbilitySystemComponent, &PawnDataGrantedHandles, GetOwner());
	}
}

void UAOExtPawnComponent::TryFinalizeAbilitySystemInitialization()
{
	if (AbilitySystemComponent == nullptr)
	{
		return;
	}

	const AAOCharacter* Character = GetPawn<AAOCharacter>();
	if (Character == nullptr)
	{
		ScheduleAbilitySystemInitializationFinalize();
		return;
	}

	const UAOPrimaryAttributeSet* PrimaryAttributeSet = AbilitySystemComponent->GetSet<UAOPrimaryAttributeSet>();
	if (PrimaryAttributeSet == nullptr)
	{
		ScheduleAbilitySystemInitializationFinalize();
		return;
	}

	ClearAbilitySystemInitializationFinalizeTimer();

	if (!bAbilitySystemLevelBindingReady)
	{
		BindCharacterLevelChanged();
		bAbilitySystemLevelBindingReady = true;
	}

	RefreshPrimaryAttributesFromCurrentLevel();
	AbilitySystemComponent->TryActivateAbilitiesOnSpawn();

	if (!bAbilitySystemInitializationFinalized)
	{
		bAbilitySystemInitializationFinalized = true;
		OnASCWasAssignDelegate.Broadcast();
	}
}

void UAOExtPawnComponent::ScheduleAbilitySystemInitializationFinalize()
{
	if (AbilitySystemComponent == nullptr || bAbilitySystemInitializationFinalized)
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			AbilitySystemInitializationFinalizeTimerHandle,
			this,
			&ThisClass::TryFinalizeAbilitySystemInitialization,
			0.1f,
			false);
	}
}

void UAOExtPawnComponent::ClearAbilitySystemInitializationFinalizeTimer()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(AbilitySystemInitializationFinalizeTimerHandle);
	}
}

void UAOExtPawnComponent::HandleAbilitySystemDataChanged()
{
	TryFinalizeAbilitySystemInitialization();
}

void UAOExtPawnComponent::RefreshPrimaryAttributesFromCurrentLevel()
{
	if (AbilitySystemComponent == nullptr)
	{
		return;
	}

	const AAOCharacter* Character = GetPawn<AAOCharacter>();
	if (Character == nullptr)
	{
		return;
	}

	const UAOPrimaryAttributeSet* PrimaryAttributeSet = AbilitySystemComponent->GetSet<UAOPrimaryAttributeSet>();
	if (PrimaryAttributeSet == nullptr)
	{
		return;
	}

	PrimaryAttributeSet->RefreshPrimaryAttributesFromLevel(DefaultPawnData, Character->GetCharacterLevel());
}

void UAOExtPawnComponent::HandleCharacterLevelChanged(int32 OldLevel, int32 NewLevel)
{
	RefreshPrimaryAttributesFromCurrentLevel();
}

void UAOExtPawnComponent::BindCharacterLevelChanged()
{
	AAOCharacter* Character = GetPawn<AAOCharacter>();
	if (Character == nullptr)
	{
		return;
	}

	Character->OnCharacterLevelChangedDelegate.RemoveAll(this);
	Character->OnCharacterLevelChangedDelegate.AddUObject(this, &ThisClass::HandleCharacterLevelChanged);
}

void UAOExtPawnComponent::UnbindCharacterLevelChanged()
{
	if (AAOCharacter* Character = GetPawn<AAOCharacter>())
	{
		Character->OnCharacterLevelChangedDelegate.RemoveAll(this);
	}

	bAbilitySystemLevelBindingReady = false;
}

void UAOExtPawnComponent::CallRegister_OnASCWasAssign(FOnASCWasAssign::FDelegate&& OnASC)
{
	OnASCWasAssignDelegate.Add(MoveTemp(OnASC));
}
