# Lyra的角色类

Lyra的Character由多重继承得来

![v2-5accb0b692f3a6ee1eac9d6eb5bb0064_r](https://pic1.zhimg.com/v2-5accb0b692f3a6ee1eac9d6eb5bb0064_r.jpg)

## AModularCharacter

<u>**AModularCharacter**</u>类是ACharacter的子类 AModularCharacter类用来添加或移除UGameFrameworkComponentManager组件请求接收器，这个类提供了一种机制，允许开发者在运行时动态地添加或移除组件到特定的Actor上。这对于插件和模块化游戏设计特别有用，因为它们可以在运行时添加新的功能，而不需要修改角色类的代码。



![a7d467c8-bbe8-40bd-8015-4e381885a219](file:///C:/Users/frozenfish/Pictures/Typedown/a7d467c8-bbe8-40bd-8015-4e381885a219.png)

<u>**AmodularCharacter</u>**中只重写了三个函数，一个所有包含的组件初始化之前执行的函数，一个游戏开始时运行的函数，一个游戏结束时运行的函数，AmodularCharacter作为父类时，其内部只负责管理订阅角色为组件的接收者，角色在游戏开始时，通知组件其附身的Actor（也就是角色本身）已准备完毕的事件

![9d70079a-8576-4842-b6f3-5f1443eaff54](file:///C:/Users/frozenfish/Pictures/Typedown/9d70079a-8576-4842-b6f3-5f1443eaff54.png)

* **`AddGameFrameworkComponentReceiver`**：注册对象以接收组件事件。

* **`SendGameFrameworkComponentExtensionEvent`**：触发事件，协调组件初始化。

* **`RemoveGameFrameworkComponentReceiver`**：清理监听，防止资源泄漏。
  
  

## ALyraCharacter

![v2-61e617f49f5c7939ea273b7b5b055a7a_r](https://pica.zhimg.com/v2-61e617f49f5c7939ea273b7b5b055a7a_r.jpg)

ALyraCharacter继承自AModularCharacter，实现了[IAbilitySystemInterface](https://zhida.zhihu.com/search?content_id=241325374&content_type=Article&match_order=1&q=IAbilitySystemInterface&zhida_source=entity)和[IGameplayCueInterface](https://zhida.zhihu.com/search?content_id=241325374&content_type=Article&match_order=1&q=IGameplayCueInterface&zhida_source=entity)和IGameplayTagAssetInterface和ILyraTeamAgentInterface这四个接口主要负责处理角色的各种行为和状态 在构造函数中，初始化一些组件，Lyra角色类基本的Component都定义在这里了如ULyraPawnExtensionComponent，ULyraHealthComponent和ULyraCameraComponent。还设置了一些角色的基本属性，如碰撞体积，重力，最大加速度等

实现了IAbilitySystemInterface后，就可以直接从角色身上获取ASC了，**<u>但是ASC并不是在ALyraCharacter里面定义的！</u>**

### ULyraPawnExtensionComponent

![7dae2667-bdad-44e6-b2e2-bd915c05825b](file:///C:/Users/frozenfish/Pictures/Typedown/7dae2667-bdad-44e6-b2e2-bd915c05825b.png)



可以看到，在重写<u>IAbilitySystemInterface</u>的GetAbilitySystemComponent()的函数实现里，获取ASC是交给PawnExtCompoent（**ULyraPawnExtensionComponent**）完成的，ULyraPawnExtensionComponent负责管理ASC的相关事务

![5c875b64-0a15-4e58-9ec0-c28e60ea8d0a](file:///C:/Users/frozenfish/Pictures/Typedown/5c875b64-0a15-4e58-9ec0-c28e60ea8d0a.png)

**<u>ULyraPawnExtensionComponent</u>** 继承自UPawnComponent，一个角色实际上也可以看作一个Pawn，`APawn` 是 [Unreal Engine](https://zhida.zhihu.com/search?content_id=252239834&content_type=Article&match_order=1&q=Unreal+Engine&zhida_source=entity) (UE) 中一个非常重要的类，通常代表 **游戏中的可控制角色** 或 **AI 驱动的对象**。`Pawn` 是 `[AActor]` 的一个子类，是 `Actor` 中最常见的用于**<u>表示可以被玩家或AI控制的实体的类型</u>**。<u>游戏中的玩家角色、敌人 NPC、车辆等通常都使用 `Pawn` 或其子类来实现。（ACharacter就继承自APawn）</u>

#### **`Pawn` 的基本概念回顾**

`Pawn` 类本身不具备控制逻辑，它提供了一个 **基础框架**，用于表示游戏中的 **可移动实体**。玩家或 AI 可以通过控制 `Pawn` 来控制其行为，具体的控制方式是通过 **控制器 (Controller)** 来实现的，因此<u>**PawnComponent **</u>是为 APawn 创建的 actor 组件，用于接收 pawn 事件。





回到ALyraCharacter，**在**构造函数中，初始化一些组件，Lyra角色类基本的Component都定义在这里了如ULyraPawnExtensionComponent，ULyraHealthComponent和ULyraCameraComponent。还设置了一些角色的基本属性，如碰撞体积，重力，最大加速度等

```
回到ALyraCharacter，在ALyraCharacter构造函数中初始化一些组件，Lyra角色类基本的Component都定义在这里了如ULyraPawnExtensionComponent，ULyraHealthComponent和ULyraCameraComponent。还设置了一些角色的基本属性，如碰撞体积，重力，最大加速度等
    ALyraCharacter::ALyraCharacter(const FObjectInitializer& ObjectInitializer)  
        : Super(ObjectInitializer.SetDefaultSubobjectClass<ULyraCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))  
    {  
        // Avoid ticking characters if possible.  
        PrimaryActorTick.bCanEverTick = false;  
        PrimaryActorTick.bStartWithTickEnabled = false;  
        NetCullDistanceSquared = 900000000.0f;  
        UCapsuleComponent* CapsuleComp = GetCapsuleComponent();  
        check(CapsuleComp);  
        CapsuleComp->InitCapsuleSize(40.0f, 90.0f);  
        CapsuleComp->SetCollisionProfileName(NAME_LyraCharacterCollisionProfile_Capsule);  

        USkeletalMeshComponent* MeshComp = GetMesh();  
        check(MeshComp);  
        MeshComp->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));  // Rotate mesh to be X forward since it is exported as Y forward.  
        MeshComp->SetCollisionProfileName(NAME_LyraCharacterCollisionProfile_Mesh);  

        ULyraCharacterMovementComponent* LyraMoveComp = CastChecked<ULyraCharacterMovementComponent>(GetCharacterMovement());  
        LyraMoveComp->GravityScale = 1.0f;  
        LyraMoveComp->MaxAcceleration = 2400.0f;  
        LyraMoveComp->BrakingFrictionFactor = 1.0f;  
        LyraMoveComp->BrakingFriction = 6.0f;  
        LyraMoveComp->GroundFriction = 8.0f;  
        LyraMoveComp->BrakingDecelerationWalking = 1400.0f;  
        LyraMoveComp->bUseControllerDesiredRotation = false;  
        LyraMoveComp->bOrientRotationToMovement = false;  
        LyraMoveComp->RotationRate = FRotator(0.0f, 720.0f, 0.0f);  
        LyraMoveComp->bAllowPhysicsRotationDuringAnimRootMotion = false;  
        LyraMoveComp->GetNavAgentPropertiesRef().bCanCrouch = true;  
        LyraMoveComp->bCanWalkOffLedgesWhenCrouching = true;  
        LyraMoveComp->SetCrouchedHalfHeight(65.0f);  

        PawnExtComponent = CreateDefaultSubobject<ULyraPawnExtensionComponent>(TEXT("PawnExtensionComponent"));  
        PawnExtComponent->OnAbilitySystemInitialized_RegisterAndCall(FSimpleMulticastDelegate::FDelegate::CreateUObject(this, &ThisClass::OnAbilitySystemInitialized));  
        PawnExtComponent->OnAbilitySystemUninitialized_Register(FSimpleMulticastDelegate::FDelegate::CreateUObject(this, &ThisClass::OnAbilitySystemUninitialized));  

        HealthComponent = CreateDefaultSubobject<ULyraHealthComponent>(TEXT("HealthComponent"));  
        HealthComponent->OnDeathStarted.AddDynamic(this, &ThisClass::OnDeathStarted);  
        HealthComponent->OnDeathFinished.AddDynamic(this, &ThisClass::OnDeathFinished);  

        CameraComponent = CreateDefaultSubobject<ULyraCameraComponent>(TEXT("CameraComponent"));  
        CameraComponent->SetRelativeLocation(FVector(-300.0f, 0.0f, 75.0f));  

        bUseControllerRotationPitch = false;  
        bUseControllerRotationYaw = true;  
        bUseControllerRotationRoll = false;  

        BaseEyeHeight = 80.0f;  
        CrouchedEyeHeight = 50.0f;  
    }
```

还实现了以下几个特殊事件的处理，本质上是转发给对应的Component处理，包括<u>[1]Controller变更时的AbilitySystemComponent更新</u>，<u>[2]PlayerState更新</u>。<u>[3]GameplayTag的管理</u>，可以获取和设置角色的GameplayTag用于标记角色的状态和能力，死亡时的逻辑处理，当角色死亡时，它会禁用角色的移动和碰撞，然后在一段时间后销毁角色。移动模式的管理，当角色的移动模式发生变化时，它会更新对应的MovementModeTag，就是在走路还是在游泳，还是在飞行之类的一个枚举。TeamID管理，获取和设置角色的团队ID,当团队ID发生改变的时候广播事件。**实现了一些网络同步的方法**，同步角色的状态，OnRep_ReplicatedAcceleration，OnRep_MyTeamID，OnRep_Controller，OnRep_PlayerState等等。 而Character_Default则是ALyraCharacter的蓝图子类，没有任何其他的额外功能实现，具体实现应该继承Character_Default然后再做拓展



在此角色在<u>服务器上的**Controller**</u>和<u>**PlayerState**</u>更新的时候，会**同步**触发角色控制器和角色状态在客户端的函数，这里通知**PawnExtComponent**去处理控制器的变化和状态的变化，以此实现Controller变更时AbilitySystemComponen的更新

![dd0ac449-8932-4835-aea9-a3b6ae6975ff](file:///C:/Users/frozenfish/Pictures/Typedown/dd0ac449-8932-4835-aea9-a3b6ae6975ff.png)



##### **(1) `PlayerController`（PC）的更新**

* **触发场景**
  
  1. **Pawn被Controller控制时**
     
     * 服务器调用 `Possess()` 或 `SetController()` 为Pawn分配Controller时，`Controller` 变量被修改，触发复制。
     
     * 客户端收到新值后，`OnRep_Controller()` 被调用。
  
  2. **Controller所有权变化**
     
     * 例如：玩家切换角色、死亡后重生，服务器重新分配Controller。
     
     * 当 `Pawn::Controller` 的值在服务器端变化时，客户端通过复制更新。
  
  3. **网络延迟或断线重连**
     
     * 客户端断线后重新连接时，服务器会重新同步Controller状态

##### **(2) `PlayerState`（PS）的更新**

* **触发场景**
  
  1. **玩家加入游戏时**
     
     * 玩家首次加入游戏时，服务器生成 `PlayerState` 并复制到所有客户端，触发 `OnRep_PlayerState()`。
  
  2. **玩家状态变化时**
     
     * 服务器修改 `PlayerState` 的属性（如 `Score`、`PlayerName`），变量标记为 `Replicated` 时会触发复制。
     
     * 客户端收到新值后，`OnRep_PlayerState()` 被调用。
  
  3. **玩家死亡或队伍切换**
     
     * 例如：玩家死亡后 `PlayerState` 的 `bIsDead` 被修改，或切换队伍时 `TeamID` 变化。
       
       

在实际测试的时候，发现角色控制器确实是在控制时，重新生成时，Controller变量被修改时会调用

```ULyraPawnExtensionComponent.cpp
void ULyraPawnExtensionComponent::HandleControllerChanged()
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
```

这个代码在检查ASC是否存在，然后检查ASC所服务的Actor是否是组件拥有者，如果不是，则取消ASC的初始化。

```ULyraPawnExtensionComponent.cpp
void ULyraPawnExtensionComponent::UninitializeAbilitySystem()
{
    if (!AbilitySystemComponent)
    {
        return;
    }

    // Uninitialize the ASC if we're still the avatar actor (otherwise another pawn already did it when they became the avatar actor)
    if (AbilitySystemComponent->GetAvatarActor() == GetOwner())
    {
        FGameplayTagContainer AbilityTypesToIgnore;
        AbilityTypesToIgnore.AddTag(LyraGameplayTags::Ability_Behavior_SurvivesDeath);

        AbilitySystemComponent->CancelAbilities(nullptr, &AbilityTypesToIgnore);
        AbilitySystemComponent->ClearAbilityInput();
        AbilitySystemComponent->RemoveAllGameplayCues();

        if (AbilitySystemComponent->GetOwnerActor() != nullptr)
        {
            AbilitySystemComponent->SetAvatarActor(nullptr);
        }
        else
        {
            // If the ASC doesn't have a valid owner, we need to clear *all* actor info, not just the avatar pairing
            AbilitySystemComponent->ClearActorInfo();
        }

        OnAbilitySystemUninitialized.Broadcast();
    }

    AbilitySystemComponent = nullptr;
}
```

取消初始化函数内部，主要是检查ASC所服务的Actor（也就是玩家角色）是否存在，如果不存在（主要是做不存在的部分）则取消所有角色的技能，技能输入，移除所有GameplayCues，同时发送一个广播，并将ASC置空，清除ASC



PossessdBy 和 UnPossessdBy内部，同样也执行了ASC的注销

```ALyraCharacter.cpp
void ALyraCharacter::PossessedBy(AController* NewController)
{
    const FGenericTeamId OldTeamID = MyTeamID;

    Super::PossessedBy(NewController);

    PawnExtComponent->HandleControllerChanged();

    // Grab the current team ID and listen for future changes
    if (ILyraTeamAgentInterface* ControllerAsTeamProvider = Cast<ILyraTeamAgentInterface>(NewController))
    {
        MyTeamID = ControllerAsTeamProvider->GetGenericTeamId();
        ControllerAsTeamProvider->GetTeamChangedDelegateChecked().AddDynamic(this, &ThisClass::OnControllerChangedTeam);
    }
    ConditionalBroadcastTeamChanged(this, OldTeamID, MyTeamID);
}
```

```ALyraCharacter.cpp
void ALyraCharacter::UnPossessed()
{
    AController* const OldController = Controller;

    // Stop listening for changes from the old controller
    const FGenericTeamId OldTeamID = MyTeamID;
    if (ILyraTeamAgentInterface* ControllerAsTeamProvider = Cast<ILyraTeamAgentInterface>(OldController))
    {
        ControllerAsTeamProvider->GetTeamChangedDelegateChecked().RemoveAll(this);
    }

    Super::UnPossessed();

    PawnExtComponent->HandleControllerChanged();

    // Determine what the new team ID should be afterwards
    MyTeamID = DetermineNewTeamAfterPossessionEnds(OldTeamID);
    ConditionalBroadcastTeamChanged(this, OldTeamID, MyTeamID);
}
```

还有一个值得注意的事情，LyraCharacter绑定了许多组件，但是这些组件是要有加载顺序的，因为一些组件依赖另一些组件，例如ULyraPawnExtensionComponent里加载了ASC组件，若A组件部分属性依赖B组件，但A组件加载得比B组件快，此时就会引发错误，Lyra在SetupPlayerInputComponent函数里检查了这一步骤

```ALyraCharacter.cpp
void ALyraCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    PawnExtComponent->SetupPlayerInputComponent();
}
```

SetupPlayerInputComponent 的调用由引擎自动触发，**发生在角色被控制器拥有且输入组件准备就绪（例如Possess）时**。开发者只需重写此函数并**绑定输入逻辑**，无需关心其调用时机。



```ULyraPawnExtensionComponen.cpp
//检查组件生命周期状态
void ULyraPawnExtensionComponent::SetupPlayerInputComponent()
{
    CheckDefaultInitialization();
}

```

```ULyraPawnExtensionComponent.cpp
void ULyraPawnExtensionComponent::CheckDefaultInitialization()
{
    // 在检查我们的进展之前，尝试推进任何其他我们可能依赖的功能。
    //该函数会尝试推进注册的Actor下其他Component的初始化阶段，之后会继续推进自己的初始化阶段
    CheckDefaultInitializationForImplementers();

    // PawnExt最先加载，所以在这里声明了组件的四种初始化阶段
    static const TArray<FGameplayTag> StateChain = { LyraGameplayTags::InitState_Spawned, LyraGameplayTags::InitState_DataAvailable, LyraGameplayTags::InitState_DataInitialized, LyraGameplayTags::InitState_GameplayReady };

    // This will try to progress from spawned (which is only set in BeginPlay) through the data initialization stages until it gets to gameplay ready
    ContinueInitStateChain(StateChain);
}
```



CheckDefaultInitializationForImplementer这个函数会检查该actor的所有component的初始化阶段，然后会推进自己的初始化阶段，等待该角色初始化完毕后自己才会初始化



综上所述，ULyraPawnExtensionComponent<u>负责执行角色**ASC**的一些业务逻辑</u>，其主要负责管理ASC的初始化，取消初始化，和这两个的分别通知和各项函数调用



**那么ULyraPawnExtensionComponent的ASC是哪里来的呢？**



```ULyraPawnExtensionComponent.cpp
void ULyraPawnExtensionComponent::InitializeAbilitySystem(ULyraAbilitySystemComponent* InASC, AActor* InOwnerActor)
{
    check(InASC);
    check(InOwnerActor);

    if (AbilitySystemComponent == InASC)
    {
        // The ability system component hasn't changed.
        return;
    }

    if (AbilitySystemComponent)
    {
        // Clean up the old ability system component.
        UninitializeAbilitySystem();
    }

    APawn* Pawn = GetPawnChecked<APawn>();
    AActor* ExistingAvatar = InASC->GetAvatarActor();

    UE_LOG(LogLyra, Verbose, TEXT("Setting up ASC [%s] on pawn [%s] owner [%s], existing [%s] "), *GetNameSafe(InASC), *GetNameSafe(Pawn), *GetNameSafe(InOwnerActor), *GetNameSafe(ExistingAvatar));

    if ((ExistingAvatar != nullptr) && (ExistingAvatar != Pawn))
    {
        UE_LOG(LogLyra, Log, TEXT("Existing avatar (authority=%d)"), ExistingAvatar->HasAuthority() ? 1 : 0);

        // There is already a pawn acting as the ASC's avatar, so we need to kick it out
        // This can happen on clients if they're lagged: their new pawn is spawned + possessed before the dead one is removed
        ensure(!ExistingAvatar->HasAuthority());

        if (ULyraPawnExtensionComponent* OtherExtensionComponent = FindPawnExtensionComponent(ExistingAvatar))
        {
            OtherExtensionComponent->UninitializeAbilitySystem();
        }
    }

    AbilitySystemComponent = InASC;
    AbilitySystemComponent->InitAbilityActorInfo(InOwnerActor, Pawn);

    if (ensure(PawnData))
    {
        InASC->SetTagRelationshipMapping(PawnData->TagRelationshipMapping);
    }

    OnAbilitySystemInitialized.Broadcast();
}
```

这个函数负责给 ULyraPawnExtensionComponent内部的ASC属性赋值，赋值来源是参数，**那么这个函数又是谁调用的，参数是从哪来的？**



### ULyraHeroComponent

同样是继承UPawnComponent，这个组件的业务更像是实际管理角色的Pawn相关活动，例如输入注册，输入绑定，技能输入触发，相机组件模式的设置

```根据传入的对应Tag输入触发技能，这里是按下触发
void ULyraHeroComponent::Input_AbilityInputTagPressed(FGameplayTag InputTag)
{
    if (const APawn* Pawn = GetPawn<APawn>())
    {
        if (const ULyraPawnExtensionComponent* PawnExtComp = ULyraPawnExtensionComponent::FindPawnExtensionComponent(Pawn))
        {
            if (ULyraAbilitySystemComponent* LyraASC = PawnExtComp->GetLyraAbilitySystemComponent())
            {
                LyraASC->AbilityInputTagPressed(InputTag);
            }
        }    
    }
}
```

```根据传入的对应Tag输入触发技能，这里是松开触发
void ULyraHeroComponent::Input_AbilityInputTagReleased(FGameplayTag InputTag)
{
    const APawn* Pawn = GetPawn<APawn>();
    if (!Pawn)
    {
        return;
    }

    if (const ULyraPawnExtensionComponent* PawnExtComp = ULyraPawnExtensionComponent::FindPawnExtensionComponent(Pawn))
    {
        if (ULyraAbilitySystemComponent* LyraASC = PawnExtComp->GetLyraAbilitySystemComponent())
        {
            LyraASC->AbilityInputTagReleased(InputTag);
        }
    }
}
```

技能的ASC需求需要交由**PawnExt** 组件完成，所以注意**PawnExt的初始化操作要在LyraHeroComponent之前**



```移动输入

void ULyraHeroComponent::Input_Move(const FInputActionValue& InputActionValue)
{
    APawn* Pawn = GetPawn<APawn>();
    AController* Controller = Pawn ? Pawn->GetController() : nullptr;

    // If the player has attempted to move again then cancel auto running
    if (ALyraPlayerController* LyraController = Cast<ALyraPlayerController>(Controller))
    {
        LyraController->SetIsAutoRunning(false);
    }

    if (Controller)
    {
        const FVector2D Value = InputActionValue.Get<FVector2D>();
        const FRotator MovementRotation(0.0f, Controller->GetControlRotation().Yaw, 0.0f);

        if (Value.X != 0.0f)
        {
            const FVector MovementDirection = MovementRotation.RotateVector(FVector::RightVector);
            Pawn->AddMovementInput(MovementDirection, Value.X);
        }

        if (Value.Y != 0.0f)
        {
            const FVector MovementDirection = MovementRotation.RotateVector(FVector::ForwardVector);
            Pawn->AddMovementInput(MovementDirection, Value.Y);
        }
    }
}
```

然后接下来就是初始化输入配置，这里的许多函数就很熟悉了，**注册IMC**，**绑定输入回调**都在这个函数进行

```
void ULyraHeroComponent::InitializePlayerInput(UInputComponent* PlayerInputComponent)
{
    check(PlayerInputComponent);

    const APawn* Pawn = GetPawn<APawn>();
    if (!Pawn)
    {
        return;
    }

    const APlayerController* PC = GetController<APlayerController>();
    check(PC);

    const ULyraLocalPlayer* LP = Cast<ULyraLocalPlayer>(PC->GetLocalPlayer());
    check(LP);

    UEnhancedInputLocalPlayerSubsystem* Subsystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
    check(Subsystem);

    Subsystem->ClearAllMappings();

    if (const ULyraPawnExtensionComponent* PawnExtComp = ULyraPawnExtensionComponent::FindPawnExtensionComponent(Pawn))
    {
        if (const ULyraPawnData* PawnData = PawnExtComp->GetPawnData<ULyraPawnData>())
        {
            if (const ULyraInputConfig* InputConfig = PawnData->InputConfig)
            {
                for (const FInputMappingContextAndPriority& Mapping : DefaultInputMappings)
                {
                    if (UInputMappingContext* IMC = Mapping.InputMapping.Get())
                    {
                        if (Mapping.bRegisterWithSettings)
                        {
                            if (UEnhancedInputUserSettings* Settings = Subsystem->GetUserSettings())
                            {
                                Settings->RegisterInputMappingContext(IMC);
                            }

                            FModifyContextOptions Options = {};
                            Options.bIgnoreAllPressedKeysUntilRelease = false;
                            // Actually add the config to the local player                            
                            Subsystem->AddMappingContext(IMC, Mapping.Priority, Options);
                        }
                    }
                }

                // The Lyra Input Component has some additional functions to map Gameplay Tags to an Input Action.
                // If you want this functionality but still want to change your input component class, make it a subclass
                // of the ULyraInputComponent or modify this component accordingly.
                ULyraInputComponent* LyraIC = Cast<ULyraInputComponent>(PlayerInputComponent);
                if (ensureMsgf(LyraIC, TEXT("Unexpected Input Component class! The Gameplay Abilities will not be bound to their inputs. Change the input component to ULyraInputComponent or a subclass of it.")))
                {
                    // Add the key mappings that may have been set by the player
                    LyraIC->AddInputMappings(InputConfig, Subsystem);

                    // This is where we actually bind and input action to a gameplay tag, which means that Gameplay Ability Blueprints will
                    // be triggered directly by these input actions Triggered events. 
                    TArray<uint32> BindHandles;
                    LyraIC->BindAbilityActions(InputConfig, this, &ThisClass::Input_AbilityInputTagPressed, &ThisClass::Input_AbilityInputTagReleased, /*out*/ BindHandles);

                    LyraIC->BindNativeAction(InputConfig, LyraGameplayTags::InputTag_Move, ETriggerEvent::Triggered, this, &ThisClass::Input_Move, /*bLogIfNotFound=*/ false);
                    LyraIC->BindNativeAction(InputConfig, LyraGameplayTags::InputTag_Look_Mouse, ETriggerEvent::Triggered, this, &ThisClass::Input_LookMouse, /*bLogIfNotFound=*/ false);
                    LyraIC->BindNativeAction(InputConfig, LyraGameplayTags::InputTag_Look_Stick, ETriggerEvent::Triggered, this, &ThisClass::Input_LookStick, /*bLogIfNotFound=*/ false);
                    LyraIC->BindNativeAction(InputConfig, LyraGameplayTags::InputTag_Crouch, ETriggerEvent::Triggered, this, &ThisClass::Input_Crouch, /*bLogIfNotFound=*/ false);
                    LyraIC->BindNativeAction(InputConfig, LyraGameplayTags::InputTag_AutoRun, ETriggerEvent::Triggered, this, &ThisClass::Input_AutoRun, /*bLogIfNotFound=*/ false);
                }
            }
        }
    }

    if (ensure(!bReadyToBindInputs))
    {
        bReadyToBindInputs = true;
    }

    UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(const_cast<APlayerController*>(PC), NAME_BindInputsNow);
    UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(const_cast<APawn*>(Pawn), NAME_BindInputsNow);
}
```

技能输入的绑定实现在这里执行

```
void ULyraHeroComponent::AddAdditionalInputConfig(const ULyraInputConfig* InputConfig)
{
    TArray<uint32> BindHandles;

    const APawn* Pawn = GetPawn<APawn>();
    if (!Pawn)
    {
        return;
    }

    const APlayerController* PC = GetController<APlayerController>();
    check(PC);

    const ULocalPlayer* LP = PC->GetLocalPlayer();
    check(LP);

    UEnhancedInputLocalPlayerSubsystem* Subsystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
    check(Subsystem);

    if (const ULyraPawnExtensionComponent* PawnExtComp = ULyraPawnExtensionComponent::FindPawnExtensionComponent(Pawn))
    {
        ULyraInputComponent* LyraIC = Pawn->FindComponentByClass<ULyraInputComponent>();
        if (ensureMsgf(LyraIC, TEXT("Unexpected Input Component class! The Gameplay Abilities will not be bound to their inputs. Change the input component to ULyraInputComponent or a subclass of it.")))
        {
            LyraIC->BindAbilityActions(InputConfig, this, &ThisClass::Input_AbilityInputTagPressed, &ThisClass::Input_AbilityInputTagReleased, /*out*/ BindHandles);
        }
    }
}
```

在定义阶段，使用了模板函数 ，根据传入的配置文件并**遍历**，查询对应的**InputAction**和对应的**Tag**是否存在，然后一一进行绑定，这样就实现了**输入按键的时候传入对应的Tag触发对应的GameplayAbility**

```
template<class UserClass, typename PressedFuncType, typename ReleasedFuncType>
void ULyraInputComponent::BindAbilityActions(const ULyraInputConfig* InputConfig, UserClass* Object, PressedFuncType PressedFunc, ReleasedFuncType ReleasedFunc, TArray<uint32>& BindHandles)
{
    check(InputConfig);

    for (const FLyraInputAction& Action : InputConfig->AbilityInputActions)
    {
        if (Action.InputAction && Action.InputTag.IsValid())
        {
            if (PressedFunc)
            {
                BindHandles.Add(BindAction(Action.InputAction, ETriggerEvent::Triggered, Object, PressedFunc, Action.InputTag).GetHandle());
            }

            if (ReleasedFunc)
            {
                BindHandles.Add(BindAction(Action.InputAction, ETriggerEvent::Completed, Object, ReleasedFunc, Action.InputTag).GetHandle());
            }
        }
    }
}
```

#### 初始化state管理

**PawnExtensionComponent**和**LyraHeroComponent**中都定义着这三个虚函数：

```
    virtual bool CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) const override;
    virtual void HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) override;
    virtual void OnActorInitStateChanged(const FActorInitStateChangedParams& Params) override;
```

**1. `CurrentState`（当前状态）**

* **含义**：组件或Actor **当前所处的初始化阶段**。

* **用途**：
  
  * 标识当前初始化进度（例如：`Loading`、`Spawned`、`Ready`）。
  
  * 在状态转换时，作为逻辑判断的依据（例如：是否允许从当前状态切换到目标状态）。

```示例值
FGameplayTag::RequestGameplayTag("InitState.Loading");  // 加载中
FGameplayTag::RequestGameplayTag("InitState.Spawned");  // 已生成但未激活
FGameplayTag::RequestGameplayTag("InitState.Ready");     // 准备就绪
```

##### **2. `DesiredState`（目标状态）**

* **含义**：组件或Actor **想要切换到的目标初始化阶段**。

* **用途**：
  
  * 驱动状态转换的目标（例如：从 `Loading` → `Ready`）。
  
  * 在 `**CanChangeInitState**` 中验证是否允许转换，在 `**HandleChangeInitState**` 中执行转换逻辑。

```示例值
FGameplayTag::RequestGameplayTag("InitState.Active");    // 已激活
FGameplayTag::RequestGameplayTag("InitState.Destroyed"); // 已销毁
```

组件在注册的时候，会**<u>注册状态初始化链</u>**

```
void UAOExtPawnComponent::OnRegister()
{
    Super::OnRegister();
    const APawn* Pawn = GetPawn<APawn>();
    ensureAlwaysMsgf((Pawn != nullptr), TEXT("LyraPawnExtensionComponent on [%s] can only be added to Pawn actors."), *GetNameSafe(GetOwner()));
    //检查玩家拥有的组件是否超过1个
    TArray<UActorComponent*> PawnExtensionComponents;
    Pawn->GetComponents(UAOExtPawnComponent::StaticClass(), PawnExtensionComponents);
    ensureAlwaysMsgf((PawnExtensionComponents.Num() == 1), TEXT("Only one LyraPawnExtensionComponent should exist on [%s]."), *GetNameSafe(GetOwner()));

    RegisterInitStateFeature();  //注册初始化状态链
}

```

* **每个 Actor 或 Component 实例** 会独立维护自己的初始化状态标签（如 `InitState.Spawned`）。

* 例如：
  
  * 角色 Actor A 可能处于 `GameplayReady` 状态。
  
  * 角色 Actor B 可能仍处于 `DataAvailable` 状态。
  
  * 某个武器组件可能还未达到 `DataInitialized` 状态。

* **状态是目标对象的属性**，用于描述该对象当前所处的生命周期阶段。

此外，还需要继承实现IGameFrameworkInitStateInterface接口才能使用这些函数

![3de1d0c5-fdff-4b59-a1de-c1a5bec63020](file:///C:/Users/frozenfish/Pictures/Typedown/3de1d0c5-fdff-4b59-a1de-c1a5bec63020.png)

虚幻的IGameFrameworkInitStateInterface具体信息参考文章[Unreal IGameFrameworkInitStateInterface - 知乎](https://zhuanlan.zhihu.com/p/615720602)



设置一个InitStateChain链为A、B、C、D四个状态，将初始状态设为A，然后调用**ContinueInitStateChain**函数，会依次将该类的初始化状态变更为B、C、D。每次变更都会默认调用**HandleChangeInitState**函数。由于是修改初始化状态，会经过**CanChangeInitState**判断是否可以进行修改。

| 标签名称                            | 标签值                         | 阶段  | 含义                                    |
| ------------------------------- | --------------------------- | --- | ------------------------------------- |
| **`InitState_Spawned`**         | `InitState.Spawned`         | 1   | Actor/组件已生成，但未进行任何初始化（仅存在于场景中，可扩展功能）。 |
| **`InitState_DataAvailable`**   | `InitState.DataAvailable`   | 2   | 所需数据已加载或复制完成（如资源加载、网络数据同步），可开始初始化。    |
| **`InitState_DataInitialized`** | `InitState.DataInitialized` | 3   | 数据已初始化（如配置应用、依赖注入），但未完全准备好参与核心玩法逻辑。   |
| **`InitState_GameplayReady`**   | `InitState.GameplayReady`   | 4   | 完全准备好参与游戏逻辑（如接受玩家输入、与其他系统交互）。         |



**CheckDefaultInitialization()** 的作用就是尝试**推进状态链**，每一次从最初的状态开始，然后依次推进，每次推进都会触发CanChangeInitState检查是否可以推进，并在到达某一个状态的阶段中顺带执行一些事务，如果某个状态没达成会返回false，<u>CheckDefaultInitialization()</u>在很多情况下都会调用，总而言之就是负责检查this实例状态的情况。

**PawnExtComp **的 CheckDefaultInitialization除了正常调用ContinueInitStateChain以外，还调用了**CheckDefaultInitializationForImplementers**，这个函数会调用这个Actors上所有**继承IGameFrameworkInitStateInterface的Components**的**CheckDefaultInitialization**(自己除外，否则死循环)

```
void ULyraPawnExtensionComponent::CheckDefaultInitialization()
{
	// 在检查我们的进展之前，尝试推进任何其他我们可能依赖的功能。
	//该函数会尝试推进注册的Actor下其他Component的初始化阶段，之后会继续推进自己的初始化阶段
	CheckDefaultInitializationForImplementers();

	// PawnExt最先加载，所以在这里声明了组件的四种初始化阶段
	static const TArray<FGameplayTag> StateChain = { LyraGameplayTags::InitState_Spawned, LyraGameplayTags::InitState_DataAvailable, LyraGameplayTags::InitState_DataInitialized, LyraGameplayTags::InitState_GameplayReady };

	//导入状态改变链，按顺序依次变更状态，每次变更都会调用HandleChangeInitState，在此之前要经过CanChangeInitState的检查
	ContinueInitStateChain(StateChain);
}
```

```推进其他ImplementerInterface接口的组件的初始化状态
void IGameFrameworkInitStateInterface::CheckDefaultInitializationForImplementers()
{
	UObject* ThisObject = Cast<UObject>(this);
	AActor* MyActor = GetOwningActor();
	UGameFrameworkComponentManager* Manager = UGameFrameworkComponentManager::GetForActor(MyActor);
	const FName MyFeatureName = GetFeatureName();

	if (Manager)
	{
		TArray<UObject*> Implementers;
		Manager->GetAllFeatureImplementers(Implementers, MyActor, FGameplayTag(), MyFeatureName);

		for (UObject* Implementer : Implementers)
		{
			if (IGameFrameworkInitStateInterface* ImplementerInterface = Cast<IGameFrameworkInitStateInterface>(Implementer))
			{
				ImplementerInterface->CheckDefaultInitialization();
			}
		}
	}
}
```

****

![v2-dd2986e7743c38b802e7441ff7cfdc37_r](https://picx.zhimg.com/v2-dd2986e7743c38b802e7441ff7cfdc37_r.jpg)



PawnExtComp是其他组件需要等待且依赖的核心组件



值得注意的是，在Lyra里，HeroComponent是通过UCLASS(Blueprintable, Meta=(BlueprintSpawnableComponent))在蓝图生成的，这样做的目的是**方便其他Actor也可以添加该组件**，而PawnExtComp则仅附加于我们的角色Actor身上，所以在其构造函数中创建了



### ULyraCharacterMovementComponent

主要**<u>增加</u>了一些特性和行为，实现自定义的角色移动组件**。 重写了CanAttemptJump函数，移除了对角色是否在下蹲状态的检查，这意味着在这个自定义组件中，角色在下蹲状态下也可以尝试跳跃。增加了一个FLyraCharacterGroundInfo结构，用于存储关于角色下方地面的信息。这个信息只在需要时更新，可以通过GetGroundInfo()函数获取。重写了GetDeltaRotation和GetMaxSpeed函数，这两个函数在角色具有TAG_Gameplay_MovementStopped标签时会返回0，当角色被标记为停止移动时，旋转和速度都会被设置为0。



### ULyraCameraComponent-Lyra的CameraMode

ULyraCameraComponent在UCameraComponent投影类型、视场、后处理覆盖基础上进行了扩展，添加了一些自定义的功能.**AddFieldOfViewOffset**函数：这个函数用于添加一个视场偏移量。这个偏移量只对一帧有效，一旦应用就会被清除GetTargetActor函数：这个函数返回摄像机正在查看的目标Actor。默认情况下，它返回的是拥有这个摄像机组件的Actor。STATIC接口FindCameraComponent函数：这个函数用于在指定的Actor中查找ULyraCameraComponent组件。GetBlendInfo函数：这个函数用于获取顶层CameraMode图层的标签和混合权重。



## ALyraPlayerState

上面说了很多ASC，那我们的ASC到底在哪呢？答案是在**PlayerState**里，Lyra作为一个多人竞技比拼项目，PlayserState除了记录玩家的得分，队伍信息以外，还负责管理创建ASC

```PlayerState构造函数
ALyraPlayerState::ALyraPlayerState(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
    , MyPlayerConnectionType(ELyraPlayerConnectionType::Player)
{
    AbilitySystemComponent = ObjectInitializer.CreateDefaultSubobject<ULyraAbilitySystemComponent>(this, TEXT("AbilitySystemComponent"));
    AbilitySystemComponent->SetIsReplicated(true);
    AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

    // These attribute sets will be detected by AbilitySystemComponent::InitializeComponent. Keeping a reference so that the sets don't get garbage collected before that.
    HealthSet = CreateDefaultSubobject<ULyraHealthSet>(TEXT("HealthSet"));
    CombatSet = CreateDefaultSubobject<ULyraCombatSet>(TEXT("CombatSet"));

    // AbilitySystemComponent needs to be updated at a high frequency.
    NetUpdateFrequency = 100.0f;

    MyTeamID = FGenericTeamId::NoTeam;
    MySquadID = INDEX_NONE;
}
```

Lyra的PlayserState继承自AModularPlayerState，在该父类里同样实现了

![a9bca606-f7e3-4f00-a92e-0cf068942039](file:///C:/Users/frozenfish/Pictures/Typedown/a9bca606-f7e3-4f00-a92e-0cf068942039.png)

在 HeroComponent里，**每一次状态的推进**，都会尝试调用PawnExt去**初始化ASC**，在对应PawnExt实现里，只需要检查ASC是否是已经保存的ASC，如果不是就卸载并替换掉旧的ASC，并为新的ASC**初始化ActorInfo**

```LyraHeroComponent.cpp
//每一次推进状态都会执行这个函数
void ULyraHeroComponent::HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState)
{
    //该逻辑在该组件处于数据加载复制完成可开始初始化且下一个状态为数据初始化但尚未准备好参与核心玩法的时候执行
    if (CurrentState == LyraGameplayTags::InitState_DataAvailable && DesiredState == LyraGameplayTags::InitState_DataInitialized)
    {
        APawn* Pawn = GetPawn<APawn>();
        ALyraPlayerState* LyraPS = GetPlayerState<ALyraPlayerState>();
        if (!ensure(Pawn && LyraPS))
        {
            return;
        }

        const ULyraPawnData* PawnData = nullptr;

        if (ULyraPawnExtensionComponent* PawnExtComp = ULyraPawnExtensionComponent::FindPawnExtensionComponent(Pawn))
        {
            PawnData = PawnExtComp->GetPawnData<ULyraPawnData>();

            // The player state holds the persistent data for this player (state that persists across deaths and multiple pawns).
            // The ability system component and attribute sets live on the player state.
            PawnExtComp->InitializeAbilitySystem(LyraPS->GetLyraAbilitySystemComponent(), LyraPS);
        }

        if (ALyraPlayerController* LyraPC = GetController<ALyraPlayerController>())
        {
            if (Pawn->InputComponent != nullptr)
            {
                InitializePlayerInput(Pawn->InputComponent);
            }
        }

        // Hook up the delegate for all pawns, in case we spectate later
        if (PawnData)
        {
            if (ULyraCameraComponent* CameraComponent = ULyraCameraComponent::FindCameraComponent(Pawn))
            {
                CameraComponent->DetermineCameraModeDelegate.BindUObject(this, &ThisClass::DetermineCameraMode);
            }
        }
    }
}
```

```LyraPawnExtensionComponent.cpp
void ULyraPawnExtensionComponent::InitializeAbilitySystem(ULyraAbilitySystemComponent* InASC, AActor* InOwnerActor)
{
    check(InASC);
    check(InOwnerActor);

    if (AbilitySystemComponent == InASC)
    {
        // The ability system component hasn't changed.
        //ASC没有改变
        return;
    }

    if (AbilitySystemComponent)
    {
        // Clean up the old ability system component.
        UninitializeAbilitySystem();
    }

    APawn* Pawn = GetPawnChecked<APawn>();
    AActor* ExistingAvatar = InASC->GetAvatarActor();

    UE_LOG(LogLyra, Verbose, TEXT("Setting up ASC [%s] on pawn [%s] owner [%s], existing [%s] "), *GetNameSafe(InASC), *GetNameSafe(Pawn), *GetNameSafe(InOwnerActor), *GetNameSafe(ExistingAvatar));

    if ((ExistingAvatar != nullptr) && (ExistingAvatar != Pawn))
    {
        UE_LOG(LogLyra, Log, TEXT("Existing avatar (authority=%d)"), ExistingAvatar->HasAuthority() ? 1 : 0);

        // There is already a pawn acting as the ASC's avatar, so we need to kick it out
        // This can happen on clients if they're lagged: their new pawn is spawned + possessed before the dead one is removed
        ensure(!ExistingAvatar->HasAuthority());

        if (ULyraPawnExtensionComponent* OtherExtensionComponent = FindPawnExtensionComponent(ExistingAvatar))
        {
            OtherExtensionComponent->UninitializeAbilitySystem();
        }
    }

    AbilitySystemComponent = InASC;
    //设置此ASC的拥有者为PlayerState，执行者为Pawn
    AbilitySystemComponent->InitAbilityActorInfo(InOwnerActor, Pawn);

    if (ensure(PawnData))
    {
        InASC->SetTagRelationshipMapping(PawnData->TagRelationshipMapping);
    }

    OnAbilitySystemInitialized.Broadcast();
}
```

PawnExt在状态推进调用的HandleChangeInitState里未实现逻辑，HeroPawn里调用PawnExt尝试初始化或替换ASC



角色（LyraCharacter）在被控制的时候，或是解除控制的时候，或是SetupPlayerInputComponent的时候，都会调用PawnExt的HandleControllerChanged函数，在这个函数内部，负责检查ASC对应的OwnerActor是否一致，如果不一致则卸载当前的ASC，否则刷新ASC的Actorinfo，并做一次状态推进

```
void ULyraPawnExtensionComponent::HandleControllerChanged()
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
```

### `RefreshAbilityActorInfo()` 是虚幻引擎 **Gameplay Ability System (GAS)** 中的一个关键方法，用于更新 **AbilitySystemComponent (ASC)** 的关联 Actor 信息。以下是它的核心作用和工作原理：

* * *

#### **1. 核心作用**

当角色的 **控制权（Controller）** 或 **Avatar Actor（如Pawn）** 发生变更时，该方法会 **重新绑定 ASC 的 OwnerActor 和 AvatarActor**，确保技能系统能正确识别以下关键角色：

* **OwnerActor**：技能的“拥有者”（通常是 `PlayerController` 或 `AIController`）。

* **AvatarActor**：技能的“执行者”（通常是 `Pawn` 或 `Character`）。

* * *

#### **2. 典型操作**

在 `RefreshAbilityActorInfo()` 中，通常会执行以下操作：

1. **更新 Actor 引用**  
   将 ASC 的 `OwnerActor` 和 `AvatarActor` 设置为当前最新的 Controller 和 Pawn。
   AbilitySystemComponent->OwnerActor = NewController;
   AbilitySystemComponent->AvatarActor = CurrentPawn;

2. **初始化 AbilityActorInfo**  
   调用 `InitAbilityActorInfo()`，将 ASC 的 `AbilityActorInfo` 结构体（包含 Owner 和 Avatar 的 Actor、Actor 的 `AbilitySystemComponent`、`AnimInstance` 等）更新到最新状态。

3. **通知 AttributeSet 和 Abilities**  
   触发 `OnAbilityActorInfoSet` 事件，通知已注册的 **AttributeSet**（属性集）和 **GameplayAbilities**（技能）更新其绑定的 Actor 信息。

4. **同步网络数据**  
   在多人游戏中，确保客户端和服务器的 ASC 数据同步（如果 ASC 被标记为需要复制）。

* * *

#### **3. 何时需要调用？**

该方法通常在以下场景被调用：

* **Pawn 被 Possess/Unpossess**（控制器切换时）

* **Pawn 初始化完成**（如 `BeginPlay`）

* **Actor 的 Owner 发生变更**（如多人游戏中玩家角色被转移）

* * *

#### **4. 不调用的后果**

若未及时调用 `RefreshAbilityActorInfo()`，会导致：

* 技能的目标（Owner/Avatar）指向错误，无法正确执行

* 属性（Attributes）无法正确绑定到当前 Actor

* 技能效果（GameplayEffects）可能应用到旧 Actor 上

* 多人游戏中出现客户端与服务端数据不一致
  
  

PawnExt实现卸载ASC的方式，在清除所携带的变量之前，要确保删除干净ASC中包含的技能，属性等

```LyraPawnExtensionComponent.cpp
void ULyraPawnExtensionComponent::UninitializeAbilitySystem()
{
    if (!AbilitySystemComponent)
    {
        return;
    }

    // Uninitialize the ASC if we're still the avatar actor (otherwise another pawn already did it when they became the avatar actor)
    if (AbilitySystemComponent->GetAvatarActor() == GetOwner())
    {
        FGameplayTagContainer AbilityTypesToIgnore;
        AbilityTypesToIgnore.AddTag(LyraGameplayTags::Ability_Behavior_SurvivesDeath);

        AbilitySystemComponent->CancelAbilities(nullptr, &AbilityTypesToIgnore);
        AbilitySystemComponent->ClearAbilityInput();
        AbilitySystemComponent->RemoveAllGameplayCues();

        if (AbilitySystemComponent->GetOwnerActor() != nullptr)
        {
            AbilitySystemComponent->SetAvatarActor(nullptr);
        }
        else
        {
            // If the ASC doesn't have a valid owner, we need to clear *all* actor info, not just the avatar pairing
            AbilitySystemComponent->ClearActorInfo();
        }

        OnAbilitySystemUninitialized.Broadcast();
    }

    AbilitySystemComponent = nullptr;
}
```
