// Fill out your copyright notice in the Description page of Project Settings.


#include "AOHeroComponent.h"
#include "Engine/Engine.h"
#include "Misc/App.h"
#include "GameFramework/Controller.h"
#include "AOCharacter.h"
#include "AOExtPawnComponent.h"
#include "AOInputBufferComponent.h"
#include "AOPawnData.h"
#include "EnhancedInputSubsystems.h"
#include "AegisOdyssey/AOGameplayTags.h"
#include "AegisOdyssey/AbilitySystem/AOAbilitySystem.h"
#include "AegisOdyssey/GameFeatures/GF_AddInputMapping.h"
#include "AegisOdyssey/Input/AOEnhancedInputComponent.h"
#include "AegisOdyssey/Player/AOLocalPlayer.h"
#include "AegisOdyssey/Player/AOPlayerController.h"
#include "AegisOdyssey/Player/AOPlayerState.h"
#include "Components/GameFrameworkComponentManager.h"
#include "GameFramework/PlayerState.h"
#include "InputMappingContext.h"
#include "AegisOdyssey/Camera/AOCameraMode.h"
#include "AegisOdyssey/Camera/AOCameraComponent.h"
#include "UserSettings/EnhancedInputUserSettings.h"
#include "AegisOdyssey/AOLogChannels.h"
#include "AegisOdyssey/UI/AOHUD.h"
#include "AegisOdyssey/UI/AOHUDViewModelComponent.h"
const FName UAOHeroComponent::NAME_ActorFeatureName("Hero");
const FName UAOHeroComponent::NAME_BindInputsNow("BindInputsNow");
#include UE_INLINE_GENERATED_CPP_BY_NAME(AOHeroComponent)
UAOHeroComponent::UAOHeroComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	
}

// 每个阶段的状态推进判断。
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

	// 在 InitializePlayerInput 这一步之后，才继续允许初始化链推进到下一个阶段。
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

		// 确保并等待 PawnExtComp 的初始化状态至少推进到和当前 Hero 一样。
		return AOPS || Manager->HasFeatureReachedInitState(Pawn, UAOExtPawnComponent::NAME_ActorFeatureName, AOGameplayTags::InitState_DataInitialized);
	}
	else if (CurrentState == AOGameplayTags::InitState_DataInitialized && DesiredState == AOGameplayTags::InitState_GameplayReady)
	{
		return true;
	}
	return false;
}

void UAOHeroComponent::HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState,
	FGameplayTag DesiredState)
{
	APawn* Pawn = GetPawn<APawn>();
		
	const UAOPawnData* PawnData = nullptr;
	
	if (CurrentState == AOGameplayTags::InitState_DataAvailable && DesiredState == AOGameplayTags::InitState_DataInitialized)
	{
		AAOPlayerState* AOPS = GetPlayerState<AAOPlayerState>();
		AAOCharacter* CurrentCharacter = Cast<AAOCharacter>(Pawn);


		if (UAOExtPawnComponent* ExtPawn = UAOExtPawnComponent::FindAOExtPawnComponent(Pawn))
		{
			PawnData = ExtPawn->GetPawnData<UAOPawnData>();
			ExtPawn->InitializeAbilitySystem(CurrentCharacter->GetSourceASC(),AOPS);
		}
		AAOPlayerController* PC = GetController<AAOPlayerController>();
		if (PC)
		{
			if (Pawn->InputComponent != nullptr)
			{
				InitializePlayerInput(Pawn->InputComponent);
			}
		}

		if (PawnData)
		{
			if (UAOCameraComponent* CameraComponent = UAOCameraComponent::FindCameraComponent(Pawn))
			{
				CameraComponent->DetermineCameraModeDelegate.BindUObject(this,&ThisClass::DetermineCameraMode);
			}
		}
	}
	if (CurrentState == AOGameplayTags::InitState_DataInitialized && DesiredState == AOGameplayTags::InitState_GameplayReady)
	{
		const bool bIsLocallyControlled = Pawn->IsLocallyControlled();
		const bool bIsBot = Pawn->IsBotControlled();

		if (bIsLocallyControlled && !bIsBot)
		{
			AAOPlayerController* AOPC = GetController<AAOPlayerController>();
			if (UAOHUDViewModelComponent* AOHUDViewModelComp = AAOHUD::FindHUDOwnedComponent<UAOHUDViewModelComponent>(AOPC))
			{
				AOHUDViewModelComp->CheckDefaultInitialization();  // 手动触发一次 HUD ViewModel 的初始化链检查。
			}
		}
	}
	
}

TSubclassOf<UAOCameraMode> UAOHeroComponent::DetermineCameraMode() const
{
	if (AbilityCameraMode)
	{
		return AbilityCameraMode;
	}

	const APawn* Pawn = GetPawn<APawn>();
	if (!Pawn)
	{
		return nullptr;
	}

	if (UAOExtPawnComponent* PawnExtComp = UAOExtPawnComponent::FindAOExtPawnComponent(Pawn))
	{
		if (const UAOPawnData* PawnData = PawnExtComp->GetPawnData<UAOPawnData>())
		{
			return PawnData->DefaultCameraMode;
		}
	}

	return nullptr;
}

//褰揚awnExt鐨勭姸鎬佸彂鐢熸敼鍙樼殑鏃跺€欎細閫氳繃鎺ュ彛璋冪敤杩欎釜鍑芥暟
void UAOHeroComponent::OnActorInitStateChanged(const FActorInitStateChangedParams& Params)
{
	if (Params.FeatureName == UAOExtPawnComponent::NAME_ActorFeatureName)
	{
		if (Params.FeatureState == AOGameplayTags::InitState_DataInitialized)
		{
			// 当 PawnExtComp 进入 DataInitialized 时，继续尝试推进 Hero 自己的初始化链。
			CheckDefaultInitialization();
		}
	}
}

void UAOHeroComponent::OnRegister()
{
	Super::OnRegister();
	RegisterInitStateFeature();  //娉ㄥ唽鍒濆鍖栫姸鎬侀摼
}

void UAOHeroComponent::BeginPlay()
{
	Super::BeginPlay();

	// 鐩戝惉PawnExtComp鐨勭姸鎬佸彉鍖栵紝浼氳皟鐢╰his鐨凮nActorInitStateChanged鍑芥暟
	BindOnActorInitStateChanged(UAOExtPawnComponent::NAME_ActorFeatureName, FGameplayTag(), false);

	// Notifies that we are done spawning, then try the rest of initialization
	ensure(TryToChangeInitState(AOGameplayTags::InitState_Spawned));
	
	CheckDefaultInitialization();
}

void UAOHeroComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
    UnregisterInitStateFeature();
	OnGameFeatureRemove();
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

	const UAOLocalPlayer* LP = Cast<UAOLocalPlayer>(PC->GetLocalPlayer());
	check(LP);

	UEnhancedInputLocalPlayerSubsystem* SubSystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	check(SubSystem);
#if WITH_EDITOR
	if (GIsEditor)
	{
		// 在编辑器 StandAlone 模式下，延迟 0.1 秒再执行一次输入初始化，
		// 避免 EnhancedInput 子系统尚未完全就绪时提前绑定失败。
		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this, PlayerInputComponent]()
		{
			InitializePlayerInputInternal(PlayerInputComponent);
		}, 0.1f, false);
		return;
	}
#endif

	InitializePlayerInputInternal(PlayerInputComponent);
}

void UAOHeroComponent::InitializePlayerInputInternal(UInputComponent* PlayerInputComponent)
{
	check(PlayerInputComponent);

	const APawn* Pawn = GetPawn<APawn>();

	if (!Pawn) return;

	const APlayerController* PC = GetController<APlayerController>();
	check(PC);

	const UAOLocalPlayer* LP = Cast<UAOLocalPlayer>(PC->GetLocalPlayer());
	check(LP);

	UEnhancedInputLocalPlayerSubsystem* SubSystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
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
					// DefaultInputMappings 这里存的是软引用。
					// 如果只调用 Get()，那么当该 IMC 资源此时还没有被预加载进内存时，这里就会直接拿到 nullptr，
					// 后续既不会注册到 UserSettings，也不会真正 Add 到本地玩家输入子系统里。
					//
					// 这正好会导致两条现象同时出现：
					// 1. QueryKeysMappedToAction() 查不到任何运行时键位；
					// 2. FindCurrentMappingForSlot() 也拿不到 UserSettings 里的可改键映射。
					//
					// 所以这里必须显式把软引用解析成真实 IMC 资产，而不是依赖它“碰巧已加载”。
					if (UInputMappingContext* IMC = Mapping.InputMapping.LoadSynchronous())
					{
						// RegisterWithSettings 的职责只应该是“要不要把这份 IMC 暴露给 Enhanced Input User Settings
						// 去参与玩家可改键配置与存档”。
						//
						// 它不应该顺便决定“这份 IMC 要不要真正加到玩家当前输入栈里”。
						// 原来的写法把 AddMappingContext 也包在这个判断里了，导致一旦这里没勾，
						// IMC 连运行时映射都不会生效，UI 和输入逻辑都会一起失效。
						if (Mapping.bRegisterWithSettings)
						{
							if (UEnhancedInputUserSettings* Settings = SubSystem->GetUserSettings())
							{
								Settings->RegisterInputMappingContext(IMC);
							}
						}

						FModifyContextOptions Options = {};
						Options.bIgnoreAllPressedKeysUntilRelease = false;

						// 无论是否注册到可改键设置，只要这是当前 Pawn 默认要生效的 IMC，
						// 都应该把它真正加入本地玩家输入子系统。
						SubSystem->AddMappingContext(IMC, Mapping.Priority, Options);
					}
				}
				
				UAOEnhancedInputComponent* AOIC = CastChecked<UAOEnhancedInputComponent>(PlayerInputComponent);
				
				if (ensureMsgf(AOIC, TEXT("Unexpected Input Component class! The Gameplay Abilities will not be bound to their inputs. Change the input component to ULyraInputComponent or a subclass of it.")))
				{
					TArray<uint32> BindHandles;

					AOIC->AddInputMappings(InputConfig , SubSystem);
					
					AOIC->BindAbilityActions(InputConfig,this,&ThisClass::Input_AbilityInputTagStarted,&ThisClass::Input_AbilityInputTagPressed,&ThisClass::Input_AbilityInputTagReleased,BindHandles);

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

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection =FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, FString::Printf(TEXT("Input X: %f, Y: %f"), InputAxisVector.X, InputAxisVector.Y));

	
	if(APawn*MyPawn = GetPawn<APawn>())
	{
		MyPawn->AddMovementInput(ForwardDirection,InputAxisVector.Y);
		MyPawn->AddMovementInput(RightDirection,InputAxisVector.X);
	}
}

void UAOHeroComponent::LookUp(const struct FInputActionValue& InputActionValue)
{
	FVector2D LookAxisVector = InputActionValue.Get<FVector2D>();  //鑾峰彇鏁版嵁浼犲叆鐨?D鍚戦噺
	AAOCharacter *OwingCharacter = Cast<AAOCharacter>(GetPawn<APawn>());
	check(OwingCharacter);
	OwingCharacter->AddControllerYawInput(LookAxisVector.X);
	OwingCharacter->AddControllerPitchInput(-(LookAxisVector.Y));
}

void UAOHeroComponent::Input_AbilityInputTagPressed(FGameplayTag InputTag)
{
	if (const APawn* Pawn = GetPawn<APawn>())
	{
		if (UAOInputBufferComponent* InputBufferComponent = UAOInputBufferComponent::FindOInputBufferComponent(Pawn))
		{
			InputBufferComponent->SetBufferedInput(InputTag,EInputType::Trigger);  // 设置预输入。
		}
		OnPressInputLoad.Broadcast(InputTag,EInputType::Trigger);
		if (const UAOExtPawnComponent* PawnExtComp = UAOExtPawnComponent::FindAOExtPawnComponent(Pawn))
		{
			if (UAOAbilitySystem* AOASC = PawnExtComp->GetAOAbilitySystemComponent())
			{
				AOASC->AbilityInputTagPressed(InputTag);
			}
		}
	}
}

void UAOHeroComponent::Input_AbilityInputTagReleased(FGameplayTag InputTag)
{
	if (const APawn* Pawn = GetPawn<APawn>())
	{
		if (UAOInputBufferComponent* InputBufferComponent = UAOInputBufferComponent::FindOInputBufferComponent(Pawn))
		{
			InputBufferComponent->SetBufferedInput(InputTag,EInputType::Release);  // 设置预输入。
		}
		OnReleaseInputLoad.Broadcast(InputTag,EInputType::Release);
		if (const UAOExtPawnComponent* PawnExtComp = UAOExtPawnComponent::FindAOExtPawnComponent(Pawn))
		{
			if (UAOAbilitySystem* AOASC = PawnExtComp->GetAOAbilitySystemComponent())
			{
				AOASC->AbilityInputTagReleased(InputTag);
			}
		}
	}
}

void UAOHeroComponent::Input_AbilityInputTagStarted(FGameplayTag InputTag)
{
	if (const APawn* Pawn = GetPawn<APawn>())
	{
		if (UAOInputBufferComponent* InputBufferComponent = UAOInputBufferComponent::FindOInputBufferComponent(Pawn))
		{
			InputBufferComponent->SetBufferedInput(InputTag,EInputType::Start);  // 设置预输入。
		}
		OnStartInputLoad.Broadcast(InputTag,EInputType::Start);
		if (const UAOExtPawnComponent* PawnExtComp = UAOExtPawnComponent::FindAOExtPawnComponent(Pawn))
		{
			if (UAOAbilitySystem* AOASC = PawnExtComp->GetAOAbilitySystemComponent())
			{
				AOASC->AbilityInputTagStarted(InputTag);
			}
		}
	}
}

bool UAOHeroComponent::InjectAbilityInputCommand(FGameplayTag InputTag, TEnumAsByte<EInputType> InputType)
{
	if (!InputTag.IsValid())
	{
		return false;
	}

	// Hero 在这里坚持只做输入桥接：
	// 1. 按输入类型把事件送进 InputBuffer / 输入委托 / ASC；
	// 2. 不判断这是不是技能槽输入；
	// 3. 把是否关心这次输入的决定权交给订阅者自己处理。
	switch (InputType.GetValue())
	{
	case EInputType::Trigger:
		Input_AbilityInputTagPressed(InputTag);
		break;

	case EInputType::Start:
		Input_AbilityInputTagStarted(InputTag);
		break;

	case EInputType::Release:
		Input_AbilityInputTagReleased(InputTag);
		break;

	default:
		return false;
	}

	if (const APawn* Pawn = GetPawn<APawn>())
	{
		if (Pawn->IsBotControlled())
		{
			if (const UAOExtPawnComponent* PawnExtComp = UAOExtPawnComponent::FindAOExtPawnComponent(Pawn))
			{
				if (UAOAbilitySystem* AOASC = PawnExtComp->GetAOAbilitySystemComponent())
				{
					// AI 没有本地玩家那套持续输入泵，因此注入命令后要主动补一次 ProcessAbilityInput，
					// 让 StateTree 在这一帧发出的输入能被 ASC 立刻消费，而不是等不到后续玩家输入驱动。
					const UWorld* World = GetWorld();
					AOASC->ProcessAbilityInput(0.0f, World ? World->IsPaused() : false);
				}
			}
		}
	}

	return true;
}

void UAOHeroComponent::OnGameFeatureRemove()
{
	// 鑾峰彇 PawnData
	const UAOPawnData* PawnData = nullptr;
	if (UAOExtPawnComponent* ExtPawn = UAOExtPawnComponent::FindAOExtPawnComponent(GetOwner()))
	{
		PawnData = ExtPawn->GetPawnData<UAOPawnData>();
	}
    
	if (PawnData)
	{
		// 鎵ц GameFeature Action 鐨勫仠鐢ㄩ€昏緫
		FGameFeatureDeactivatingContext DeactivatingContext(
			TEXT("AOGameCore"),
			[](FStringView) {}
		);
        
		const FWorldContext* ExistingWorldContext = GEngine->GetWorldContextFromWorld(GetWorld());
		if (ExistingWorldContext)
		{
			DeactivatingContext.SetRequiredWorldContextHandle(ExistingWorldContext->ContextHandle);
		}
        
		for (UGameFeatureAction* Action : PawnData->Actions)
		{
			if (Action)
			{
				Action->OnGameFeatureUnregistering();
				Action->OnGameFeatureUnloading();
				Action->OnGameFeatureDeactivating(DeactivatingContext);
			}
		}
	}
}

void UAOHeroComponent::OnGameFeaturActivate()
{
	const APawn* Pawn = GetPawn<APawn>();
	const UAOPawnData* PawnData = nullptr;
	
	if (UAOExtPawnComponent* ExtPawn = UAOExtPawnComponent::FindAOExtPawnComponent(Pawn))
	{
		PawnData = ExtPawn->GetPawnData<UAOPawnData>();
	}

	const bool bIsLocallyControlled = Pawn->IsLocallyControlled();
	const bool bIsBot = Pawn->IsBotControlled();
	if (bIsLocallyControlled && bIsBot)
	{
		// 鍒濆鍖栧繀瑕佹暟鎹嵆灏嗗畬姣曠殑鏃跺€欏紑濮嬪姞杞紸I鐨勭浉鍏崇殑GameFeature From PawnData鏁版嵁
		if (PawnData)
		{
			FGameFeatureActivatingContext Context;
			// Only apply to our specific world context if set
			// 只选取与当前世界相关的上下文，避免把激活范围扩散到别的世界实例。
			const FWorldContext* ExistingWorldContext = GEngine->GetWorldContextFromWorld(GetWorld());
			if (ExistingWorldContext)
			{
				Context.SetRequiredWorldContextHandle(ExistingWorldContext->ContextHandle);  //纭繚鍦ㄦ纭殑涓栫晫鍔犺浇
			}
	
			auto ActivationListOfActions = [&Context] (const TArray<UGameFeatureAction*>& InActions)
			{
				for (UGameFeatureAction* Action : InActions)
				{
					if (Action != nullptr)
					{
						Action->OnGameFeatureRegistering();
						Action->OnGameFeatureLoading();
						Action->OnGameFeatureActivating(Context);
					}
				}
			};
			ActivationListOfActions(PawnData->Actions);   // 逐步激活 PawnData 里配置的 GameFeature。
		}
	}
}


