// Fill out your copyright notice in the Description page of Project Settings.


#include "GF_AddAbilities.h"

#include "AegisOdyssey/AbilitySystem/AOAbilitySystem.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "UIExtensionSystem.h"
#include "AegisOdyssey/Character/AOCharacter.h"
#include "AegisOdyssey/Player/AOPlayerState.h"
#include "UObject/FastReferenceCollector.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GF_AddAbilities)

void UGF_AddAbilities::OnGameFeatureActivating(FGameFeatureActivatingContext& Context)
{

	FPerContextData& ActivateData = ContextData.FindOrAdd(Context);  //查找或添加当前GF激活上下文
	if (!ensureAlways(ActivateData.ComponentRequestHandles.IsEmpty()))
	{
		Reset(ActivateData);
	}
	
	Super::OnGameFeatureActivating(Context);
	
}

void UGF_AddAbilities::OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context)
{
	Super::OnGameFeatureDeactivating(Context);
	FPerContextData* ActivateData = ContextData.Find(Context);
	if (ensure(ActivateData))
	{
		Reset(*ActivateData);
	}
}

void UGF_AddAbilities::AddToWorld(const FWorldContext& WorldContext,
	const FGameFeatureStateChangeContext& ChangeContext)
{
	UWorld* World = WorldContext.World();
	UGameInstance* GameInstance = WorldContext.OwningGameInstance;
	FPerContextData& ActivateData = ContextData.FindOrAdd(ChangeContext);

	if ((GameInstance!=nullptr) && (World!=nullptr) && World->IsGameWorld())
	{
		if (UGameFrameworkComponentManager* ComponentMan = UGameInstance::GetSubsystem<UGameFrameworkComponentManager>(GameInstance))
		{
			int32 EntryIndex = 0;
			for (const FGameFeatureAbilitiesEntry& Entry : AbilitiesList)
			{
				if (!Entry.ActorClass.IsNull())
				{
					UGameFrameworkComponentManager::FExtensionHandlerDelegate AddAbilitiesDelegate = UGameFrameworkComponentManager::
					FExtensionHandlerDelegate::CreateUObject(this,&UGF_AddAbilities::HandleActorExtension,EntryIndex,ChangeContext);
					TSharedPtr<FComponentRequestHandle> ExtensionRequestHandle = ComponentMan->AddExtensionHandler(
						Entry.ActorClass,AddAbilitiesDelegate);

					ActivateData.ComponentRequestHandles.Add(ExtensionRequestHandle);
					EntryIndex++;
				}
			}
		}
	}
}

void UGF_AddAbilities::Reset(FPerContextData& ActiveData)
{
	while (!ActiveData.ActiveExtensions.IsEmpty())
	{
		auto ExtensionIt = ActiveData.ActiveExtensions.CreateIterator();
	}
	ActiveData.ComponentRequestHandles.Empty();
}

void UGF_AddAbilities::HandleActorExtension(AActor* Actor, FName EventName, int32 EntryIndex,
                                            FGameFeatureStateChangeContext ChangeContext)
{
	FPerContextData* ActiveData = ContextData.Find(ChangeContext);
	if (AbilitiesList.IsValidIndex(EntryIndex) && ActiveData)
	{
		const FGameFeatureAbilitiesEntry& Entry = AbilitiesList[EntryIndex];
		if (EventName == UGameFrameworkComponentManager::NAME_ExtensionRemoved || EventName == UGameFrameworkComponentManager::NAME_ReceiverRemoved)
		{
			RemoveActorAbilities(Actor,*ActiveData);
		}
		else if (EventName == UGameFrameworkComponentManager::NAME_ExtensionAdded || EventName == AAOPlayerState::NAME_AOAbilityReady)
		{
			AddActorAbilities(Actor,Entry,*ActiveData);
		}
	}
}

//添加技能
void UGF_AddAbilities::AddActorAbilities(AActor* Actor, const FGameFeatureAbilitiesEntry& AbilitiesEntry,
	FPerContextData& ActiveData)
{
	//UUIExtensionSubsystem* ExtensionSubsystem = GetWorld()->GetSubsystem<UUIExtensionSubsystem>();
	check(Actor);
	if (!Actor->HasAuthority()) return;//确保函数在服务器运行
	if (ActiveData.ActiveExtensions.Find(Actor) != nullptr) return;  //如果当前角色已经拥有技能，则返回

	if (UAbilitySystemComponent* AbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Actor))
	{
		FActorExtensions AddExtensions;

		//为数组预分配空间，避免不必要的内容扩充
		AddExtensions.Attributes.Reserve(AbilitiesEntry.AttributeClass.Num());
		AddExtensions.AbilitySpecHandles.Reserve(AbilitiesEntry.GrantedAbilitySets.Num());

		for (const FAOAbilityGrant& Abilities : AbilitiesEntry.GrantedAbilities)
		{
			if (!Abilities.AbilityType.IsNull())
			{
				FGameplayAbilitySpec NewAbilitySpec(Abilities.AbilityType.LoadSynchronous()); //在不占用主线程的情况下加载对应的Ability实例
				FGameplayAbilitySpecHandle AbilityHandle = AbilitySystemComponent->GiveAbility(NewAbilitySpec);

				AddExtensions.AbilitySpecHandles.Add(AbilityHandle);
			}
		}

		for (const FAOAttributeSetGrant& Attributes : AbilitiesEntry.AttributeClass)
		{
			if (!Attributes.AttributeSetType.IsNull())
			{
				TSubclassOf<UAttributeSet> SetType = Attributes.AttributeSetType.LoadSynchronous();
				if (SetType)
				{
					UAttributeSet* New_AttributeSet = NewObject<UAttributeSet>(AbilitySystemComponent->GetOwnerActor(),SetType);
					AddExtensions.Attributes.AddUnique(New_AttributeSet);
					AbilitySystemComponent->AddAttributeSetSubobject(New_AttributeSet);
				}
			}
		}
		UAOAbilitySystem* AOASC = CastChecked<UAOAbilitySystem>(AbilitySystemComponent);
		check(AOASC);  //本地ASC必须存在

		for (const TSoftObjectPtr<const UAOAbilitySet>& SetPtr : AbilitiesEntry.GrantedAbilitySets)
		{
			if (const UAOAbilitySet* Set = SetPtr.Get())
			{
				Set->GiveToAbilitySystem(AOASC, &AddExtensions.AbilitySetHandles.AddDefaulted_GetRef());
			}
		}

		// 记录这个 Actor 已经完成过本次 GameFeature 的能力扩展授予。
		// 否则同一个 Actor 再次收到 ExtensionAdded / AOAbilityReady 事件时，
		// 会重复授予同一批能力，进而导致统一交互能力实例重复响应一次输入。
		ActiveData.ActiveExtensions.Add(Actor, MoveTemp(AddExtensions));
	}
}
//移除角色身上的技能

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
