# Lyra的GAS系统01

## GameplayAbility

Lyra在GameFeature里为角色添加技能（UGameFeatureAction_AddAbilities），整体思路和AddInputMapping差不多。

关于GameFeature插件可以去看大钊的文章：[《InsideUE5》GameFeatures架构（一）发展由来 - 知乎](https://zhuanlan.zhihu.com/p/467236675)



### 基本配置：

先来到LyraAbilitySet，我们得知道技能，效果，属性这些配置都存到了哪

![fa6af94e-f5a8-4c07-a4c8-c77cc313ec00](file:///C:/Users/frozenfish/Pictures/Typedown/fa6af94e-f5a8-4c07-a4c8-c77cc313ec00.png)

```
USTRUCT(BlueprintType)
struct FLyraAbilitySet_GameplayAbility
{
    GENERATED_BODY()

public:

    // Gameplay ability to grant.
    UPROPERTY(EditDefaultsOnly)
    TSubclassOf<ULyraGameplayAbility> Ability = nullptr;

    // Level of ability to grant.
    UPROPERTY(EditDefaultsOnly)
    int32 AbilityLevel = 1;

    // Tag used to process input for the ability.
    UPROPERTY(EditDefaultsOnly, Meta = (Categories = "InputTag"))
    FGameplayTag InputTag;
};
```

技能的结构配置，包含技能的类型，技能等级，以及触发技能的GamePlayTag，然后使用TArray进行储存。

![a1350197-1a05-476d-804a-5339f7d59d01](file:///C:/Users/frozenfish/Pictures/Typedown/a1350197-1a05-476d-804a-5339f7d59d01.png)

这里随便打开了一个表，可以看到技能的类型和触发的Tag

![c8900454-1767-4069-9945-00eb0441b951](file:///C:/Users/frozenfish/Pictures/Typedown/c8900454-1767-4069-9945-00eb0441b951.png)

回到**UGameFeatureAction_AddAbilities**，在该类里有一个AbilitiesList，可以看成是所有**包括AbilitySet在内的技能配置的集合**

![b01d2871-4bb4-4e8f-ae82-5f2039b4a869](file:///C:/Users/frozenfish/Pictures/Typedown/b01d2871-4bb4-4e8f-ae82-5f2039b4a869.png)

这其中包含一个**软类引用**（只会保存该类的逻辑标识符，不会具体加载）的Actor类，GameAbility的实例，Attribute的实例，以及刚刚说的AbilitySet

![810dff66-b059-4cfd-81fa-bf0fa80d9200](file:///C:/Users/frozenfish/Pictures/Typedown/810dff66-b059-4cfd-81fa-bf0fa80d9200.png)



和**AddInputMapping**的GF一样，AddAbilities同样配置了**当GameFeatureAction状态改变的时候是否应应用于存在多个世界或上下文**的情况，记录当前的GFAction状态并配置对应的值，这个值包含**跟踪组件添加时的回调**以及当**前Actor正在激活的技能相关信息**

理解起来有点抽象，总结就是`FComponentRequestHandle` 是一个用于 **管理组件（Component）或游戏功能（Game Feature）动态操作请求的句柄类**，用于 **安全地跟踪和控制操作的执行状态与生命周期**

![6fa0a0ce-e390-424c-9dd9-db75e540007a](file:///C:/Users/frozenfish/Pictures/Typedown/6fa0a0ce-e390-424c-9dd9-db75e540007a.png)

OnGameFeatureActivating中调用了Reset函数，这个函数是每次GF激活的时候检查当前激活GF状态的上下文中，是否含有之前激活过该GF的数据残留，如果有那就在逻辑中移除赋予角色的Ability，确保没有残留内存

取消GF激活的时候也是同理的操作

```
void UGameFeatureAction_AddAbilities::OnGameFeatureActivating(FGameFeatureActivatingContext& Context)
{
    FPerContextData& ActiveData = ContextData.FindOrAdd(Context);

    if (!ensureAlways(ActiveData.ActiveExtensions.IsEmpty()) ||
        !ensureAlways(ActiveData.ComponentRequests.IsEmpty()))
    {
        Reset(ActiveData);
    }
    Super::OnGameFeatureActivating(Context);
}

void UGameFeatureAction_AddAbilities::OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context)
{
    Super::OnGameFeatureDeactivating(Context);
    FPerContextData* ActiveData = ContextData.Find(Context);

    if (ensure(ActiveData))
    {
        Reset(*ActiveData);
    }
}
```

```
void UGameFeatureAction_AddAbilities::Reset(FPerContextData& ActiveData)
{
    while (!ActiveData.ActiveExtensions.IsEmpty())
    {
        auto ExtensionIt = ActiveData.ActiveExtensions.CreateIterator();
        RemoveActorAbilities(ExtensionIt->Key, ActiveData);
    }

    ActiveData.ComponentRequests.Empty();
}
```

Activating最终都是要调用**AddToWorld**的，其主要做了遍历所有FGameFeatureAbilitiesEntry并每次绑定，通过 `ComponentMan->AddExtensionHandler()` 注册了一个委托 (`AddAbilitiesDelegate`)，当 **特定 软引用Actor 类**（`Entry.ActorClass`）被创建或初始化到世界中时，会触发 `HandleActorExtension` 方法，HandleActorExtension中完成添加技能到角色的方法。

```
void UGameFeatureAction_AddAbilities::AddToWorld(const FWorldContext& WorldContext, const FGameFeatureStateChangeContext& ChangeContext)
{
    UWorld* World = WorldContext.World();
    UGameInstance* GameInstance = WorldContext.OwningGameInstance;
    FPerContextData& ActiveData = ContextData.FindOrAdd(ChangeContext);

    if ((GameInstance != nullptr) && (World != nullptr) && World->IsGameWorld())
    {
        if (UGameFrameworkComponentManager* ComponentMan = UGameInstance::GetSubsystem<UGameFrameworkComponentManager>(GameInstance))
        {            
            int32 EntryIndex = 0;
            for (const FGameFeatureAbilitiesEntry& Entry : AbilitiesList)
            {
                if (!Entry.ActorClass.IsNull())
                {
                    UGameFrameworkComponentManager::FExtensionHandlerDelegate AddAbilitiesDelegate = UGameFrameworkComponentManager::FExtensionHandlerDelegate::CreateUObject(
                        this, &UGameFeatureAction_AddAbilities::HandleActorExtension, EntryIndex, ChangeContext);
                    TSharedPtr<FComponentRequestHandle> ExtensionRequestHandle = ComponentMan->AddExtensionHandler(Entry.ActorClass, AddAbilitiesDelegate);

                    ActiveData.ComponentRequests.Add(ExtensionRequestHandle);
                    EntryIndex++;
                }
            }
        }
    }
}

```

```
void UGameFeatureAction_AddAbilities::HandleActorExtension(AActor* Actor, FName EventName, int32 EntryIndex, FGameFeatureStateChangeContext ChangeContext)
{
    FPerContextData* ActiveData = ContextData.Find(ChangeContext);
    if (AbilitiesList.IsValidIndex(EntryIndex) && ActiveData)
    {
        const FGameFeatureAbilitiesEntry& Entry = AbilitiesList[EntryIndex];
        if ((EventName == UGameFrameworkComponentManager::NAME_ExtensionRemoved) || (EventName == UGameFrameworkComponentManager::NAME_ReceiverRemoved))
        {
            RemoveActorAbilities(Actor, *ActiveData);
        }
        else if ((EventName == UGameFrameworkComponentManager::NAME_ExtensionAdded) || (EventName == ALyraPlayerState::NAME_LyraAbilityReady))
        {
            AddActorAbilities(Actor, Entry, *ActiveData);
        }
    }
}
```

TSharedPtr<FComponentRequestHandle> ExtensionRequestHandle = ComponentMan->AddExtensionHandler(Entry.ActorClass, AddAbilitiesDelegate);是当对应的ActorClass完成初始化的时候就会触发委托回调，Lyra选择了PlayerState，所以可以看到**当PlayerState初始化的时候，对应的回调函数就被通知**

![e56044cd-b47c-4e97-82c6-d14bc1890a59](file:///C:/Users/frozenfish/Pictures/Typedown/e56044cd-b47c-4e97-82c6-d14bc1890a59.png)

包括在**PlayerState里的SetPawnData**也通过**手动调用**的方式来激活委托回调函数

![c4747869-6220-482e-bb7e-0ca99eef2a1a](file:///C:/Users/frozenfish/Pictures/Typedown/c4747869-6220-482e-bb7e-0ca99eef2a1a.png)

所以，AddToWorld主要是**遍历了包含FGameFeatureAbilitiesEntry的数组**，数组的每个成员都包含了单个ActorClass对应的技能表，属性表，还有AbilitySets，然后每一次都将该ActorClass注册到FExtensionHandlerDelegate委托中，于是当**该ActorClass在世界中创建实例并初始化**的时候，就会触发对应的函数（HandleActorExtension）回调。

接下来是AddActorAbilities的解析：

```
void UGameFeatureAction_AddAbilities::AddActorAbilities(AActor* Actor, const FGameFeatureAbilitiesEntry& AbilitiesEntry, FPerContextData& ActiveData)
{
    check(Actor);
    if (!Actor->HasAuthority())
    {
        return;
    }

    // early out if Actor already has ability extensions applied
    if (ActiveData.ActiveExtensions.Find(Actor) != nullptr)
    {
        return;    
    }

    if (UAbilitySystemComponent* AbilitySystemComponent = FindOrAddComponentForActor<UAbilitySystemComponent>(Actor, AbilitiesEntry, ActiveData))
    {
        FActorExtensions AddedExtensions;
        AddedExtensions.Abilities.Reserve(AbilitiesEntry.GrantedAbilities.Num());
        AddedExtensions.Attributes.Reserve(AbilitiesEntry.GrantedAttributes.Num());
        AddedExtensions.AbilitySetHandles.Reserve(AbilitiesEntry.GrantedAbilitySets.Num());

        for (const FLyraAbilityGrant& Ability : AbilitiesEntry.GrantedAbilities)
        {
            if (!Ability.AbilityType.IsNull())
            {
                FGameplayAbilitySpec NewAbilitySpec(Ability.AbilityType.LoadSynchronous());
                FGameplayAbilitySpecHandle AbilityHandle = AbilitySystemComponent->GiveAbility(NewAbilitySpec);

                AddedExtensions.Abilities.Add(AbilityHandle);
            }
        }

        for (const FLyraAttributeSetGrant& Attributes : AbilitiesEntry.GrantedAttributes)
        {
            if (!Attributes.AttributeSetType.IsNull())
            {
                TSubclassOf<UAttributeSet> SetType = Attributes.AttributeSetType.LoadSynchronous();
                if (SetType)
                {
                    UAttributeSet* NewSet = NewObject<UAttributeSet>(AbilitySystemComponent->GetOwner(), SetType);
                    if (!Attributes.InitializationData.IsNull())
                    {
                        UDataTable* InitData = Attributes.InitializationData.LoadSynchronous();
                        if (InitData)
                        {
                            NewSet->InitFromMetaDataTable(InitData);
                        }
                    }

                    AddedExtensions.Attributes.Add(NewSet);
                    AbilitySystemComponent->AddAttributeSetSubobject(NewSet);
                }
            }
        }

        ULyraAbilitySystemComponent* LyraASC = CastChecked<ULyraAbilitySystemComponent>(AbilitySystemComponent);
        for (const TSoftObjectPtr<const ULyraAbilitySet>& SetPtr : AbilitiesEntry.GrantedAbilitySets)
        {
            if (const ULyraAbilitySet* Set = SetPtr.Get())
            {
                Set->GiveToAbilitySystem(LyraASC, &AddedExtensions.AbilitySetHandles.AddDefaulted_GetRef());
            }
        }

        ActiveData.ActiveExtensions.Add(Actor, AddedExtensions);
    }
    else
    {
        UE_LOG(LogGameFeatures, Error, TEXT("Failed to find/add an ability component to '%s'. Abilities will not be granted."), *Actor->GetPathName());
    }
}
```

理一下思路：FGameFeatureAbilitiesEntry结构体保存的是角色**可被赋予的**技能和属性，FPerContextData保存的是当前Actor**已被赋予的**技能和属性

所以上面的函数先是检查了**是否必须是在服务器运行**，然后**检查当前Actor是否已经拥有了被赋予的技能和属性**，检查完毕以后就开始为角色依次添加技能，LoadSynchronous在**不占用主线程**的情况下**异步地加载**Ability实例，后面的添加属性集AttributeSet也是一样的流程不赘述，不要忘记调用角色的AbilitySystemComponent的AddAttributeSetSubobject去添加AttributeSet



接着，还获取了自定义的子类ASC（以下称LyraASC），然后将刚刚的数据单独存到了AbilitySet中

![24cc8766-eeb6-4170-8ae2-2f253db553ba](file:///C:/Users/frozenfish/Pictures/Typedown/24cc8766-eeb6-4170-8ae2-2f253db553ba.png)



```
void UAOAbilitySet::GiveToAbilitySystem(UAOAbilitySystem* InAOASC, FAOAbilitySet_GrantedHandles* OutGrantedHandles,
                                        UObject* SourceObject) const
{
    check(InAOASC);

    if (!InAOASC->IsOwnerActorAuthoritative()) return;

    for (int32 SetIndex = 0 ; SetIndex < AttributeSets.Num() ; SetIndex++)
    {
        const FAOAbilitySet_AttributeSet& SetToGrant = AttributeSets[SetIndex];

        if (!IsValid(SetToGrant.AttributeSet))
        {
            continue;
        }

        UAttributeSet* NewSet = NewObject<UAttributeSet>(InAOASC, SetToGrant.AttributeSet);
        InAOASC->AddAttributeSetSubobject(NewSet);

        if (OutGrantedHandles)
        {
            OutGrantedHandles->AddAttributeSet(NewSet);
        }
    }

    for (int32 AbilityIndex = 0 ; AbilityIndex < GrantedGameplayAbilities.Num() ; AbilityIndex++)
    {
        const FAOAbilitySet_GameplayAbility& AbilityToGrant = GrantedGameplayAbilities[AbilityIndex];

        if (!IsValid(AbilityToGrant.Ability))
        {
            continue;
        }

        UAOGameplayAbility* AbilityCDO = AbilityToGrant.Ability->GetDefaultObject<UAOGameplayAbility>();

        FGameplayAbilitySpec AbilitySpec(AbilityCDO,AbilityToGrant.AbilityLevel);
        AbilitySpec.SourceObject = SourceObject;
        AbilitySpec.GetDynamicSpecSourceTags().AddTag(AbilityToGrant.InputTag);

        const FGameplayAbilitySpecHandle Spec = InAOASC->GiveAbility(AbilitySpec);

        if (OutGrantedHandles)
        {
            OutGrantedHandles->AddAbilitySpecHandle(Spec);
        }
    }

    for (int32 EffectIndex = 0 ; EffectIndex < GrantedGameplayEffects.Num() ; EffectIndex++)
    {
        const FAOAbilitySet_GameplayEffect& EffectToGrant = GrantedGameplayEffects[EffectIndex];

        if (!IsValid(EffectToGrant.GameplayEffect))
        {
            UE_LOG(LogTemp, Error, TEXT("%s"), *GetName());
            continue;
        }

        const UGameplayEffect* GameplayEffect = EffectToGrant.GameplayEffect->GetDefaultObject<UGameplayEffect>();
        const FActiveGameplayEffectHandle GameplayEffectHandle = InAOASC->ApplyGameplayEffectToSelf(GameplayEffect,EffectToGrant.EffectLevel,InAOASC->MakeEffectContext());

        if (OutGrantedHandles)
        {
            OutGrantedHandles->AddGameplayEffectHandle(GameplayEffectHandle);
        }
    }
}
```

这段函数的目的是为LyraASC添加上技能（GameplayAbility）和属性集（AttributeSet），还有应用游戏效果（GameplayEffect）

```
void UGF_AddAbilities::RemoveActorAbilities(AActor* Actor, FPerContextData& ActiveData)
{
    if (FActorExtensions* ActorExtensions = ActiveData.ActiveExtensions.Find(Actor))
    {
        if (UAbilitySystemComponent* AbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Actor))
        {
            for (UAttributeSet* AttribSetInstance : ActorExtensions->Attributes)
            {
                AbilitySystemComponent->RemoveSpawnedAttribute(AttribSetInstance);
            }

            for (FGameplayAbilitySpecHandle AbilityHandle : ActorExtensions->AbilitySpecHandles)
            {
                AbilitySystemComponent->SetRemoveAbilityOnEnd(AbilityHandle);
            }

            UAOAbilitySystem* LyraASC = CastChecked<UAOAbilitySystem>(AbilitySystemComponent);
            for (FAOAbilitySet_GrantedHandles& SetHandle : ActorExtensions->AbilitySetHandles)
            {
                SetHandle.TakeFromAbilitySystem(LyraASC);
            }
        }

        ActiveData.ActiveExtensions.Remove(Actor);
    }
}
```

移除Abilities会调用一个TakeFromAbilitySystem的函数，这个函数在之后的物品-装备系统会用到，物品装备系统先不说

```
void FLyraAbilitySet_GrantedHandles::TakeFromAbilitySystem(ULyraAbilitySystemComponent* LyraASC)
{
    check(LyraASC);

    if (!LyraASC->IsOwnerActorAuthoritative())
    {
        // Must be authoritative to give or take ability sets.
        return;
    }

    for (const FGameplayAbilitySpecHandle& Handle : AbilitySpecHandles)
    {
        if (Handle.IsValid())
        {
            LyraASC->ClearAbility(Handle);
        }
    }

    for (const FActiveGameplayEffectHandle& Handle : GameplayEffectHandles)
    {
        if (Handle.IsValid())
        {
            LyraASC->RemoveActiveGameplayEffect(Handle);
        }
    }

    for (UAttributeSet* Set : GrantedAttributeSets)
    {
        LyraASC->RemoveSpawnedAttribute(Set);
    }

    AbilitySpecHandles.Reset();
    GameplayEffectHandles.Reset();
    GrantedAttributeSets.Reset();
}

```

这段函数就是清除了技能，激活的Effect，生成的Attribute，然后清空了保存的数组的成员，但**仍保留内存容量**



在LyraAbility





至此，Lyra的Ability初始化到此完毕，之后只需要加载并激活GF即可。



### ULyraGameplayAbility

![3e26d22f-e726-4490-ad6f-40751aa2402c](file:///C:/Users/frozenfish/Pictures/Typedown/3e26d22f-e726-4490-ad6f-40751aa2402c.png)

前面的几个**Getter**就不阐述了，我们看几个比较重要的函数：

```
void ULyraGameplayAbility::TryActivateAbilityOnSpawn(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) const
{
    // Try to activate if activation policy is on spawn.
    if (ActorInfo && !Spec.IsActive() && (ActivationPolicy == ELyraAbilityActivationPolicy::OnSpawn))
    {
        UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
        const AActor* AvatarActor = ActorInfo->AvatarActor.Get();

        // If avatar actor is torn off or about to die, don't try to activate until we get the new one.
        if (ASC && AvatarActor && !AvatarActor->GetTearOff() && (AvatarActor->GetLifeSpan() <= 0.0f))
        {
            const bool bIsLocalExecution = (NetExecutionPolicy == EGameplayAbilityNetExecutionPolicy::LocalPredicted) || (NetExecutionPolicy == EGameplayAbilityNetExecutionPolicy::LocalOnly);
            const bool bIsServerExecution = (NetExecutionPolicy == EGameplayAbilityNetExecutionPolicy::ServerOnly) || (NetExecutionPolicy == EGameplayAbilityNetExecutionPolicy::ServerInitiated);

            const bool bClientShouldActivate = ActorInfo->IsLocallyControlled() && bIsLocalExecution;
            const bool bServerShouldActivate = ActorInfo->IsNetAuthority() && bIsServerExecution;

            if (bClientShouldActivate || bServerShouldActivate)
            {
                ASC->TryActivateAbility(Spec.Handle);
            }
        }
    }
}
```

这个函数会在**许多条件完成的情况下尝试激活GA**，主要判断是否应该本地客户端（检查是否是本地控制器且枚举条件是本地预测）和是否应该是服务器（检查对象是否含有服务器权限且枚举条件是否为仅仅服务器），这个函数的调用时机实际上是在HeroComponent的HandleChangeInitState上也就是当前状态为**InitState_DataAvailable**和目标状态为**InitState_DataInitialized**的情况；ULyraGameplayAbility的**OnGiveAbility**也会存在，当**ASC->GiveAbility**的时候会触发。



CanActivateAbility负责判断当前GA实例是否可以被TryActivateAbility。

```
bool ULyraGameplayAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!ActorInfo || !ActorInfo->AbilitySystemComponent.IsValid())
	{
		return false;
	}

	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	//@TODO Possibly remove after setting up tag relationships
	ULyraAbilitySystemComponent* LyraASC = CastChecked<ULyraAbilitySystemComponent>(ActorInfo->AbilitySystemComponent.Get());
	if (LyraASC->IsActivationGroupBlocked(ActivationGroup))
	{
		if (OptionalRelevantTags)
		{
			OptionalRelevantTags->AddTag(LyraGameplayTags::Ability_ActivateFail_ActivationGroup);
		}
		return false;
	}

	return true;
}
```



OnRemoveAbility会在ClearAbility的时候触发。

K2_OnAbilityRemoved()是特意命名用于只在蓝图执行的函数（BlueprintImplementableEvent）

```
void ULyraGameplayAbility::OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	K2_OnAbilityRemoved();

	Super::OnRemoveAbility(ActorInfo, Spec);
}
```

```ULyraGameplayAbility.h

    // Additional costs that must be paid to activate this ability
    UPROPERTY(EditDefaultsOnly, Instanced, Category = Costs)
    TArray<TObjectPtr<ULyraAbilityCost>> AdditionalCosts;

    // Map of failure tags to simple error messages
    UPROPERTY(EditDefaultsOnly, Category = "Advanced")
    TMap<FGameplayTag, FText> FailureTagToUserFacingMessages;

    // Map of failure tags to anim montages that should be played with them
    UPROPERTY(EditDefaultsOnly, Category = "Advanced")
    TMap<FGameplayTag, TObjectPtr<UAnimMontage>> FailureTagToAnimMontage;ntage = nullptr;
};
```

这三个配置是技能激活失败的时候，会使用**其中的内容**去完成某些事情

Abilityfailure：

<img title="" src="file:///E:/DownLoad/deepseek_mermaid_20250531_5378dd.png" alt="deepseek_mermaid_20250531_5378dd" style="zoom:25%;">



例如，在GA_Weapon_Fire蓝图实现中，会监听当前技能失败的信息，然后根据回调去触发播放动画蒙太奇，但不会执行其他操作。



![aa5eabcb-fc7a-4721-8e75-dd644c49b948](file:///C:/Users/frozenfish/Pictures/Typedown/aa5eabcb-fc7a-4721-8e75-dd644c49b948.png)

![ad7a9042-543e-4dbe-9679-d7f79d626464](file:///C:/Users/frozenfish/Pictures/Typedown/ad7a9042-543e-4dbe-9679-d7f79d626464.png)

### 触发方式：

接下来就是触发阶段，Lyra的技能激活是由输入触发的，确切来说是Input->GameplayTag->GameplayAbiliy，当然也不全是这样的情况，

在上面的代码我们得知，Lyra做了一个  AbilitySpec.GetDynamicSpecSourceTags().AddTag(AbilityToGrant.InputTag); 手段，目的是**为当前技能Spec添加一个动态的Tag**，之后要触发这个Tag，就可以通过查询输入激活的inputTag和DynamicSpecSourceTag里所保存的Tag是否一致。

在HeroComponent中，有：

![912677af-768a-4558-9f5f-d424ebd344cf](file:///C:/Users/frozenfish/Pictures/Typedown/912677af-768a-4558-9f5f-d424ebd344cf.png)

InputConfig是保存InputAction和对应GameplayTag的数据表（ULyraInputConfig），详情自己去查看，这里不再赘述；BindAbilityActions是自定义的模板函数，将查询的Tag和对应的回调函数绑定

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

这样就实现了InputAction->GameplayTag->GameplayAbility的流程，接下来是回调函数的具体逻辑：

```
void ULyraAbilitySystemComponent::AbilityInputTagPressed(const FGameplayTag& InputTag)
{
    if (InputTag.IsValid())
    {
        for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
        {
            if (AbilitySpec.Ability && (AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag)))
            {
                InputPressedSpecHandles.AddUnique(AbilitySpec.Handle);
                InputHeldSpecHandles.AddUnique(AbilitySpec.Handle);
            }
        }
    }
}

void ULyraAbilitySystemComponent::AbilityInputTagReleased(const FGameplayTag& InputTag)
{
    if (InputTag.IsValid())
    {
        for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
        {
            if (AbilitySpec.Ability && (AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag)))
            {
                InputReleasedSpecHandles.AddUnique(AbilitySpec.Handle);
                InputHeldSpecHandles.Remove(AbilitySpec.Handle);
            }
        }
    }
}
```

就是我们可以看到添加进了一个输入句柄，这里遍历了ASC里的已经激活的技能通过Tag识别将当前输入触发的内容存进池子里

这个池子需要有应用的，而关键函数就是**ProcessAbilityInput**

```
void UAOAbilitySystem::ProcessAbilityInput(float DeltaTime, bool bGamePaused)
{
        if (HasMatchingGameplayTag(TAG_Gameplay_AbilityInputBlocked))
    {
        ClearAbilityInput();
        return;
    }

    static TArray<FGameplayAbilitySpecHandle> AbilitiesToActivate;
    AbilitiesToActivate.Reset();

    //@TODO: See if we can use FScopedServerAbilityRPCBatcher ScopedRPCBatcher in some of these loops

    //
    // Process all abilities that activate when the input is held.
    //
    for (const FGameplayAbilitySpecHandle& SpecHandle : InputHeldSpecHandles)
    {
        if (const FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(SpecHandle))
        {
            if (AbilitySpec->Ability && !AbilitySpec->IsActive())
            {
                const UAOGameplayAbility* LyraAbilityCDO = Cast<UAOGameplayAbility>(AbilitySpec->Ability);
                if (LyraAbilityCDO)
                {
                    AbilitiesToActivate.AddUnique(AbilitySpec->Handle);
                }
            }
        }
    }

    //
    // Process all abilities that had their input pressed this frame.
    //
    for (const FGameplayAbilitySpecHandle& SpecHandle : InputPressedSpecHandles)
    {
        if (FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(SpecHandle))
        {
            if (AbilitySpec->Ability)
            {
                AbilitySpec->InputPressed = true;

                if (AbilitySpec->IsActive())
                {
                    // Ability is active so pass along the input event.
                    AbilitySpecInputPressed(*AbilitySpec);
                }
                else
                {
                    const UAOGameplayAbility* LyraAbilityCDO = Cast<UAOGameplayAbility>(AbilitySpec->Ability);

                    if (LyraAbilityCDO)
                    {
                        AbilitiesToActivate.AddUnique(AbilitySpec->Handle);
                    }
                }
            }
        }
    }

    //
    //尝试激活所有来自点击和按下的能力。我们一次完成所有操作，这样按下的输入不会激活能力，然后也会因为点击而向能力发送输入事件。
    //
    for (const FGameplayAbilitySpecHandle& AbilitySpecHandle : AbilitiesToActivate)
    {
        TryActivateAbility(AbilitySpecHandle);
    }

    //
    // Process all abilities that had their input released this frame.
    //
    for (const FGameplayAbilitySpecHandle& SpecHandle : InputReleasedSpecHandles)
    {
        if (FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(SpecHandle))
        {
            if (AbilitySpec->Ability)
            {
                AbilitySpec->InputPressed = false;

                if (AbilitySpec->IsActive())
                {
                    // Ability is active so pass along the input event.
                    AbilitySpecInputReleased(*AbilitySpec);
                }
            }
        }
    }

    //
    // Clear the cached ability handles.
    //
    InputPressedSpecHandles.Reset();
    InputReleasedSpecHandles.Reset();
}
```

这个函数由Controller的PostProcessInput，从字面意思理解是后处理输入，但是我们只需要知道他是**每一帧都会调用的函数**，它**每一帧**都会调用LyraASC的ProcessAbilityInput（就上面那块函数）

```

void ALyraPlayerController::PostProcessInput(const float DeltaTime, const bool bGamePaused)
{
    if (ULyraAbilitySystemComponent* LyraASC = GetLyraAbilitySystemComponent())
    {
        LyraASC->ProcessAbilityInput(DeltaTime, bGamePaused);
    }

    Super::PostProcessInput(DeltaTime, bGamePaused);
}
```

以下是三个池子的保存，分别是按下**按键技能的池子**，**松开按键技能的池子**，**长按按键技能的池子**，在LyraASC的头文件里

![bf2c2252-78fa-4f21-9b0c-f5a634b1c060](file:///C:/Users/frozenfish/Pictures/Typedown/bf2c2252-78fa-4f21-9b0c-f5a634b1c060.png)

所以整个流程理解起来就是：当我们按下输入的时候，会将我们输入的Tag对应的技能Handle放到池子里，然后**每一帧**都会对池子进行处理，对池子进行遍历，遍历的每一个按下**输入的池子**，**长按的池子**，**松开的池子**都拿出来放到一个总的大池子**AbilitiesToActivate**里，然后把这个大池子再遍历激活里面的所有技能，**此时角色ASC身上的技能就处于Active状态**，处于Active状态以后就可以真正地去响应**InvokeReplicatedEvent**事件（在AbilitySpecInputPressed或AbilitySpecInputReleased里），**当前技能**的**WaitInputPress节点**就会触发回调，最后再把三个小池子清空成员（但保留内存）

```
    // Additional costs that must be paid to activate this ability
    UPROPERTY(EditDefaultsOnly, Instanced, Category = Costs)
    TArray<TObjectPtr<ULyraAbilityCost>> AdditionalCosts;

    // Map of failure tags to simple error messages
    UPROPERTY(EditDefaultsOnly, Category = "Advanced")
    TMap<FGameplayTag, FText> FailureTagToUserFacingMessages;

    // Map of failure tags to anim montages that should be played with them
    UPROPERTY(EditDefaultsOnly, Category = "Advanced")
    TMap<FGameplayTag, TObjectPtr<UAnimMontage>> FailureTagToAnimMontage;
```
