// Fill out your copyright notice in the Description page of Project Settings.


#include "AOHeroComponent.h"
#include "GameFramework/Controller.h"
#include "AOCharacter.h"
#include "AOExtPawnComponent.h"
#include "AOPawnData.h"
#include "EnhancedInputSubsystems.h"
#include "AegisOdyssey/AOGameplayTags.h"
#include "AegisOdyssey/GameFeatures/GF_AddInputMapping.h"
#include "AegisOdyssey/Input/AOEnhancedInputComponent.h"
#include "AegisOdyssey/Player/AOPlayerController.h"
#include "AegisOdyssey/Player/AOPlayerState.h"
#include "Components/GameFrameworkComponentManager.h"
#include "GameFramework/PlayerState.h"
#include "UserSettings/EnhancedInputUserSettings.h"
const FName UAOHeroComponent::NAME_ActorFeatureName("Hero");
const FName UAOHeroComponent::NAME_BindInputsNow("BindInputsNow");
#include UE_INLINE_GENERATED_CPP_BY_NAME(AOHeroComponent)
UAOHeroComponent::UAOHeroComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	
}

//每阶段的状态推断
bool UAOHeroComponent::CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState,
                                          FGameplayTag DesiredState) const
{
	check(Manager);

	APawn* Pawn = GetPawn<APawn>();

	if (!CurrentState.IsValid() && DesiredState == AOGameplayTags::InitState_Spawned)
	{
		if (Pawn)
		{
			return true;
		}
	}
	else if (CurrentState == AOGameplayTags::InitState_Spawned && DesiredState == AOGameplayTags::InitState_DataAvailable)
	{
		if (Pawn->GetLocalRole() != ROLE_SimulatedProxy)
		{
			AController* Controller = GetController<AController>();

			const bool bHasControllerPairedWithPS = (Controller != nullptr) && \
				(Controller->PlayerState != nullptr) && \
				(Controller->PlayerState->GetOwner() == Controller);

			if (!bHasControllerPairedWithPS)
			{
				return false;
			}
		}

		const bool bIsLocallyControlled = Pawn->IsLocallyControlled();
		const bool bIsBot = Pawn->IsBotControlled();

		if (bIsLocallyControlled && !bIsBot)
		{
			AAOPlayerController* AOPC = GetController<AAOPlayerController>();

			if (!Pawn->InputComponent || !AOPC || !AOPC->GetLocalPlayer())
			{
				return false;
			}
		}
		return true;
	}
	else if (CurrentState == AOGameplayTags::InitState_DataAvailable && DesiredState == AOGameplayTags::InitState_DataInitialized)
	{
		AAOPlayerState* AOPS = GetPlayerState<AAOPlayerState>();
		//确保并等待PawnExtComp的初始化状态提前于当前this的状态
		return AOPS || Manager->HasFeatureReachedInitState(Pawn,UAOExtPawnComponent::NAME_ActorFeatureName,AOGameplayTags::InitState_DataInitialized);
	}
	else if (CurrentState == AOGameplayTags::InitState_DataInitialized && DesiredState == AOGameplayTags::InitState_GameplayReady)
	{
		return true;
	}
	return false;
}

//若状态能够推进，则执行该函数
void UAOHeroComponent::HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState,
	FGameplayTag DesiredState)
{
	if (CurrentState == AOGameplayTags::InitState_DataAvailable && DesiredState == AOGameplayTags::InitState_DataInitialized)
	{
		APawn* Pawn = GetPawn<APawn>();
		AAOPlayerController* AOPC = GetController<AAOPlayerController>();
		AAOCharacter* CurrentCharacter = Cast<AAOCharacter>(Pawn);
		if (!ensure(Pawn && AOPC))
		{
			return;
		}

		if (UAOExtPawnComponent* ExtPawn = UAOExtPawnComponent::FindAOExtPawnComponent(Pawn))
		{
			ExtPawn->InitializeAbilitySystem(CurrentCharacter->GetSourceASC(),AOPC);
		}

		if (AAOPlayerController* PC = GetController<AAOPlayerController>())
		{
			if (Pawn->InputComponent != nullptr)
			{
				InitializePlayerInput(Pawn->InputComponent);
			}
		}
	}
}

//当PawnExt的状态发生改变的时候会通过接口调用这个函数
void UAOHeroComponent::OnActorInitStateChanged(const FActorInitStateChangedParams& Params)
{
	if (Params.FeatureName == UAOExtPawnComponent::NAME_ActorFeatureName)
	{
		if (Params.FeatureState == AOGameplayTags::InitState_DataInitialized)
		{
			//当PawnExtComp进入DataInitialized的时候继续推进this的下一个状态
			CheckDefaultInitialization();
		}
	}
}

void UAOHeroComponent::OnRegister()
{
	Super::OnRegister();
	RegisterInitStateFeature();  //注册初始化状态链
}

void UAOHeroComponent::BeginPlay()
{
	Super::BeginPlay();

	// 监听PawnExtComp的状态变化，会调用this的OnActorInitStateChanged函数
	BindOnActorInitStateChanged(UAOExtPawnComponent::NAME_ActorFeatureName, FGameplayTag(), false);

	// Notifies that we are done spawning, then try the rest of initialization
	ensure(TryToChangeInitState(AOGameplayTags::InitState_Spawned));
	
	CheckDefaultInitialization();
}

void UAOHeroComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnregisterInitStateFeature();
	Super::EndPlay(EndPlayReason);
}



void UAOHeroComponent::CheckDefaultInitialization()
{
	static const TArray<FGameplayTag> StateChain = { AOGameplayTags::InitState_Spawned, AOGameplayTags::InitState_DataAvailable,
		AOGameplayTags::InitState_DataInitialized, AOGameplayTags::InitState_GameplayReady };

	// This will try to progress from spawned (which is only set in BeginPlay) through the data initialization stages until it gets to gameplay ready
	ContinueInitStateChain(StateChain);
}

void UAOHeroComponent::InitializePlayerInput(UInputComponent* PlayerInputComponent)
{
	check(PlayerInputComponent);

	const APawn* Pawn = GetPawn<APawn>();

	if (!Pawn) return;

	const APlayerController* PC = GetController<APlayerController>();
	check(PC);
	

	UEnhancedInputLocalPlayerSubsystem* SubSystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer());
	check(SubSystem);

	SubSystem->ClearAllMappings();

	if (const UAOExtPawnComponent* AOExtPawn = UAOExtPawnComponent::FindAOExtPawnComponent(Pawn))
	{
		if (const UAOPawnData* PawnData = AOExtPawn->GetPawnData<UAOPawnData>())
		{
			if (const UAOInputConfig* InputConfig = PawnData->InputConfig)
			{
				for (const FInputMappingContextAndPriority& Mapping : DefaultInputMappings)
				{
					if (UInputMappingContext* IMC = Mapping.InputMapping.Get())
					{
						if (Mapping.bRegisterWithSettings)
						{
							if (UEnhancedInputUserSettings* Settings = SubSystem->GetUserSettings())
							{
								Settings->RegisterInputMappingContext(IMC);
							}
							FModifyContextOptions Options = {};
							Options.bIgnoreAllPressedKeysUntilRelease = false;
							// Actually add the config to the local player							
							SubSystem->AddMappingContext(IMC, Mapping.Priority, Options);
						}
					}
				}
				
				UAOEnhancedInputComponent* AOIC = CastChecked<UAOEnhancedInputComponent>(PlayerInputComponent);
				
				if (ensureMsgf(AOIC, TEXT("Unexpected Input Component class! The Gameplay Abilities will not be bound to their inputs. Change the input component to ULyraInputComponent or a subclass of it.")))
				{
					TArray<uint32> BindHandles;

					AOIC->BindAbilityActions(InputConfig,this,&ThisClass::Input_AbilityInputTagPressed,&ThisClass::Input_AbilityInputTagReleased,BindHandles);

					AOIC->BindNativeAction(InputConfig,AOGameplayTags::Input_Move,ETriggerEvent::Triggered,this,&ThisClass::Input_Move,false);
					AOIC->BindNativeAction(InputConfig,AOGameplayTags::Input_LookUp,ETriggerEvent::Triggered,this,&ThisClass::LookUp,false);
				}
			}
		}
	}




	UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(const_cast<APlayerController*>(PC), NAME_BindInputsNow);
	UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(const_cast<APawn*>(Pawn), NAME_BindInputsNow);
}


void UAOHeroComponent::Input_Move(const FInputActionValue& InputActionValue)
{
	AAOPlayerController* PC = GetController<AAOPlayerController>();
	const FVector2D InputAxisVector = InputActionValue.Get<FVector2D>();
	const FRotator Rotation = PC->GetControlRotation();
	const FRotator YawRotation(0.f,Rotation.Yaw,0.f);

	/*获取角色向量在平面方向的坐标*/
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection =FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, FString::Printf(TEXT("Input X: %f, Y: %f"), InputAxisVector.X, InputAxisVector.Y));

	
	if(APawn*MyPawn = GetPawn<APawn>())
	{
		/*添加移动输入操作*/
		MyPawn->AddMovementInput(ForwardDirection,InputAxisVector.Y);
		MyPawn->AddMovementInput(RightDirection,InputAxisVector.X);
	}
}

void UAOHeroComponent::LookUp(const struct FInputActionValue& InputActionValue)
{
	FVector2D LookAxisVector = InputActionValue.Get<FVector2D>();  //获取数据传入的2D向量
	AAOCharacter *OwingCharacter = Cast<AAOCharacter>(GetPawn<APawn>());
	check(OwingCharacter);
	OwingCharacter->AddControllerYawInput(LookAxisVector.X);
	OwingCharacter->AddControllerPitchInput(-(LookAxisVector.Y));
}

void UAOHeroComponent::Input_AbilityInputTagPressed(FGameplayTag InputTag)
{
	
}

void UAOHeroComponent::Input_AbilityInputTagReleased(FGameplayTag InputTag)
{
	
}