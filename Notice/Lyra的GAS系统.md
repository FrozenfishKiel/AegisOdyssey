# Lyra的GAS系统

Lyra有两个GAS的保存位置，一个在PlayerState，一个在角色类里

<img title="PlayerState" src="file:///C:/Users/frozenfish/Pictures/Typedown/c39cda6f-52d7-4f60-a3b7-be50e843396b.png" alt="loading-ag-61" data-align="center">

Lyra还写了一个CharacterWithAbilities类，这个类继承LyraCharacter，然后在构造函数中创建了一个ASC

![42ec1f88-403d-4fa2-92a0-eccb775d28db](file:///C:/Users/frozenfish/Pictures/Typedown/42ec1f88-403d-4fa2-92a0-eccb775d28db.png)

PlayerState是每一个玩家独有的，玩家在游戏中无论切换多少个控制的Pawn，使用的ASC是一样的，如果是在角色类里创建，那么每一个控制的角色ASC都是不一样的，具体情况要看游戏的设计方向讨论，而Lyra这两种设计模式都创建了。



## GameplayAbility

Lyra在GameFeature里为角色添加技能（UGameFeatureAction_AddAbilities），整体思路和AddInputMapping差不多。



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

回到UGameFeatureAction_AddAbilities，在该类里有一个AbilitiesList，可以看成是所有包括AbilitySet在内的技能配置的集合

![b01d2871-4bb4-4e8f-ae82-5f2039b4a869](file:///C:/Users/frozenfish/Pictures/Typedown/b01d2871-4bb4-4e8f-ae82-5f2039b4a869.png)

这其中包含一个软类引用（只会保存该类的逻辑标识符，不会具体加载）的Actor类，GameAbility的实例，Attribute的实例，以及刚刚说的AbilitySet

![810dff66-b059-4cfd-81fa-bf0fa80d9200](file:///C:/Users/frozenfish/Pictures/Typedown/810dff66-b059-4cfd-81fa-bf0fa80d9200.png)



和AddInputMapping的GF一样，AddAbilities同样配置了**当GameFeatureAction状态改变的时候是否应应用于存在多个世界或上下文**的情况，记录当前的GFAction状态并配置对应的值，这个值包含**跟踪组件添加时的回调**以及当**前Actor正在激活的技能相关信息**

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

Activating最终都是要调用AddToWorld的，其主要做了遍历所有FGameFeatureAbilitiesEntry并每次绑定，通过 `ComponentMan->AddExtensionHandler()` 注册了一个委托 (`AddAbilitiesDelegate`)，当 **特定 软引用Actor 类**（`Entry.ActorClass`）被创建或初始化到世界中时，会触发 `HandleActorExtension` 方法，HandleActorExtension中完成添加技能到角色的方法。

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

TSharedPtr<FComponentRequestHandle> ExtensionRequestHandle = ComponentMan->AddExtensionHandler(Entry.ActorClass, AddAbilitiesDelegate);是当对应的ActorClass完成初始化的时候就会触发委托回调，Lyra选择了PlayerState，所以可以看到当PlayerState初始化的时候，对应的回调函数就被通知

![e56044cd-b47c-4e97-82c6-d14bc1890a59](file:///C:/Users/frozenfish/Pictures/Typedown/e56044cd-b47c-4e97-82c6-d14bc1890a59.png)

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


