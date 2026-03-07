// Fill out your copyright notice in the Description page of Project Settings.


#include "AOVMPawnComponent.h"
#include "AOCharacter.h"
#include "Components/GameFrameworkComponentManager.h"
#include "AOExtPawnComponent.h"
#include "AOHeroComponent.h"
#include "AegisOdyssey/AOGameplayTags.h"
#include "AegisOdyssey/AOLogChannels.h"
#include "AegisOdyssey/AbilitySystem/Attributes/AOHealthAttributeSet.h"
#include "Input/CommonUIActionRouterBase.h"
#include "Net/UnrealNetwork.h"
#include "Net/Core/PushModel/PushModel.h"

const FName UAOVMPawnComponent::NAME_ActorFeatureName("VMPawn");

UAOVMPawnComponent::UAOVMPawnComponent(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bTickEvenWhenPaused = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
	SetIsReplicatedByDefault(true);
	bReplicateUsingRegisteredSubObjectList = true;
}
void UAOVMPawnComponent::BeginPlay()
{
	Super::BeginPlay();
	
	// Notifies state manager that we have spawned, then try rest of default initialization
	//ensure(TryToChangeInitState(AOGameplayTags::InitState_Spawned));  //设定当前的CurrentState为InitState_Spawned然后开始调用一次状态链
	//BindOnActorInitStateChanged(UAOHeroComponent::NAME_ActorFeatureName,AOGameplayTags::InitState_DataInitialized,true);
}


void UAOVMPawnComponent::HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState,
	FGameplayTag DesiredState)
{
	if (CurrentState == AOGameplayTags::InitState_DataAvailable && DesiredState == AOGameplayTags::InitState_DataInitialized)
	{
	}
}

void UAOVMPawnComponent::OnRegister()
{
	Super::OnRegister();
}

void UAOVMPawnComponent::CheckDefaultInitialization()
{
	IGameFrameworkInitStateInterface::CheckDefaultInitialization();
	InitializeViewModel();

	if (UAOExtPawnComponent* ExtPawn = UAOExtPawnComponent::FindAOExtPawnComponent(GetOwner()))
	{
		ExtPawn->CallRegister_OnASCWasAssign(FOnASCWasAssign::FDelegate::CreateUObject(this, &ThisClass::InitializeHUDViewModel));
	}
}

//当其他组件初始化状态完成的时候
void UAOVMPawnComponent::OnActorInitStateChanged(const FActorInitStateChangedParams& Params)
{
	IGameFrameworkInitStateInterface::OnActorInitStateChanged(Params);
	if (Params.FeatureName == UAOHeroComponent::NAME_ActorFeatureName)
	{
		if (Params.FeatureState == AOGameplayTags::InitState_DataInitialized)
		{
			
		}
	}
}

void UAOVMPawnComponent::InitializeViewModel()
{
	if (!CharacterHUDViewModel && GetOwner()->HasAuthority())
	{
		CharacterHUDViewModel = NewObject<UMVVM_HUD>(GetOwner());
		AddReplicatedSubObject(CharacterHUDViewModel);  //让该对象参与网络复制
		// 标记脏数据，触发网络复制
		MARK_PROPERTY_DIRTY_FROM_NAME(UAOVMPawnComponent, CharacterHUDViewModel, this);
	}
	if (!CharacterInventoryViewModel && GetOwner()->HasAuthority())
	{
		CharacterInventoryViewModel = NewObject<UMVVM_InventoryMenu>(GetOwner());
		AddReplicatedSubObject(CharacterInventoryViewModel);  //让该对象参与网络复制
		// 标记脏数据，触发网络复制
		MARK_PROPERTY_DIRTY_FROM_NAME(UAOVMPawnComponent, CharacterInventoryViewModel, this);
	}
}

//初始化角色HUD的UI数据
void UAOVMPawnComponent::InitializeHUDViewModel()
{
	APawn* Pawn = GetPawn<APawn>();
	AAOPlayerController* AOPC = GetController<AAOPlayerController>();
	AAOCharacter* CurrentCharacter = Cast<AAOCharacter>(Pawn);
	if (UAOExtPawnComponent* ExtPawn = UAOExtPawnComponent::FindAOExtPawnComponent(Pawn))
	{
		FPlayerMainHUDViewModelParams ViewModelParams;
		UAbilitySystemComponent* TargetASC = ExtPawn->GetAbilitySystemComponent();
		if (!TargetASC) return;
		ViewModelParams.ASC = TargetASC;

		APlayerController* SourcePC = Cast<APlayerController>(Pawn->GetController());
		if (!SourcePC) return;
		ViewModelParams.PC = SourcePC;

		APlayerState* SourcePS = Cast<APlayerState>(Pawn->GetPlayerState());
		if (!SourcePS) return;
		ViewModelParams.PS = SourcePS;

		if (CharacterHUDViewModel)
		{
			CharacterHUDViewModel->SetPlayerViewModelParams(ViewModelParams);
		}
	}
}

void UAOVMPawnComponent::InitializeInventoryViewModel()
{
	
}

void UAOVMPawnComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(ThisClass , CharacterHUDViewModel , COND_None , REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(ThisClass , CharacterInventoryViewModel , COND_None , REPNOTIFY_Always);
}

void UAOVMPawnComponent::UninitializeComponent()
{
	Super::UninitializeComponent();

	if (CharacterHUDViewModel)
	{
		CharacterHUDViewModel->MarkAsGarbage();
		CharacterHUDViewModel = nullptr;
	}
	
	if (CharacterInventoryViewModel)
	{
		CharacterInventoryViewModel->MarkAsGarbage();
		CharacterInventoryViewModel = nullptr;
	}
}

void UAOVMPawnComponent::TickComponent(float DeltaTime, enum ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if (LocalPlayer)
	{
		// 获取CommonUI的路由器
		UCommonUIActionRouterBase* ActionRouter = LocalPlayer->GetSubsystem<UCommonUIActionRouterBase>();
		if (ActionRouter)
		{
			ECommonInputMode CurrentInputMode = ActionRouter->GetActiveInputMode();
			FString ModeString;

			switch (CurrentInputMode)
			{
			case ECommonInputMode::Menu:
				ModeString = TEXT("UI Only");
				break;
			case ECommonInputMode::Game:
				ModeString = TEXT("Game Only");
				break;
			case ECommonInputMode::All:
				ModeString = TEXT("Game and UI");
				break;
			default:
				ModeString = TEXT("Unknown");
				break;
			}

			// 获取鼠标在世界上的坐标
			APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
			if (PlayerController)
			{
				FVector WorldLocation;
				FVector WorldDirection;
				PlayerController->DeprojectMousePositionToWorld(WorldLocation, WorldDirection);
				WorldDirection *= 1000.f;

				FVector ViewEnd = WorldLocation + WorldDirection;
				// 设置游戏模式为GameAndUI
				// ActionRouter->SetActiveUIInputConfig(ECommonInputMode::All);
				
				// 绘制鼠标轨迹的Debug球
				//DrawDebugSphere(GetWorld(), ViewEnd, 10.0f, 12, FColor::Red, false, -1.0f, 0, 1.0f);
				
				// 打印到屏幕和输出日志
				//GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Cyan, FString::Printf(TEXT("鼠标世界坐标X: %.2f, Y: %.2f"), WorldLocation.X, WorldLocation.Y));
				//UE_LOG(LogTemp, Display, TEXT("鼠标世界坐标X: %.2f, Y: %.2f"), WorldLocation.X, WorldLocation.Y);
			}
		}
		else
		{
			//UE_LOG(LogTemp, Warning, TEXT("无法获取CommonUIActionRouterBase."));
		}
	}
	else
	{
		///UE_LOG(LogTemp, Warning, TEXT("无法获取LocalPlayer."));
	}
}
void UAOVMPawnComponent::OnRep_CharacterHUDViewModel()
{
	UE_LOG(LogAegisOdyssey, Warning, TEXT("客户端获取CharacterHUDViewModel"));
	InitializeHUDViewModel();
}

void UAOVMPawnComponent::OnRep_CharacterInventoryViewModel()
{
	UE_LOG(LogAegisOdyssey, Warning, TEXT("客户端获取CharacterInventoryViewModel"));
}

