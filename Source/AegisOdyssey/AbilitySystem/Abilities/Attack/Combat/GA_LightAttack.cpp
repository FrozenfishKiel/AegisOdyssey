// Fill out your copyright notice in the Description page of Project Settings.


#include "GA_LightAttack.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AegisOdyssey/StateTree/AOStateTreeComponentBase.h"
#include "AegisOdyssey/StateTree/CombatStateTree/AOCombatStateTree.h"
#include "AegisOdyssey/Character/AOCharacter.h"
#include "AegisOdyssey/Animation/AOAnimInstance.h"
#include "AegisOdyssey/Animation/NotifyState/AOCombatWindow.h"
#include "AegisOdyssey/AOLogChannels.h"
#include "AbilitySystemGlobals.h"
#include "InterchangeTranslatorBase.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Kismet/KismetSystemLibrary.h"
#include "AegisOdyssey/AbilitySystem/Tasks/AT_WaitMovementInput.h"
#include "AegisOdyssey/AbilitySystem/Tasks/AT_WaitRotateToDirection.h"
#include "AegisOdyssey/Equipment/AOWeaponManagerComponent.h"
#include "AegisOdyssey/Equipment/Weapons/AOWeaponDefinition.h"
#include "AegisOdyssey/Equipment/Weapons/AOWeaponInstance.h"
#include "GameFramework/Controller.h"
#include "GameFramework/CharacterMovementComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GA_LightAttack)

namespace AOLightAttackTracePrivate
{
	constexpr float TraceRadius = 12.0f;
	constexpr float DebugDrawSeconds = 0.03f;

	static FString BuildHitActorSummary(const TArray<FHitResult>& Hits)
	{
		TArray<FString> ActorNames;
		ActorNames.Reserve(Hits.Num());
		for (const FHitResult& Hit : Hits)
		{
			ActorNames.Add(GetNameSafe(Hit.GetActor()));
		}

		return ActorNames.Num() > 0
			? FString::Join(ActorNames, TEXT(","))
			: FString(TEXT("None"));
	}

	static void AppendTraceHits(
		UObject* WorldContextObject,
		const FVector& CurrentTraceStart,
		const FVector& CurrentTraceEnd,
		const TArray<TEnumAsByte<EObjectTypeQuery>>& ObjectTypes,
		const TArray<AActor*>& IgnoreActors,
		TArray<FHitResult>& OutHits)
	{
		UKismetSystemLibrary::SphereTraceMultiForObjects(
			WorldContextObject,
			CurrentTraceStart,
			CurrentTraceEnd,
			TraceRadius,
			ObjectTypes,
			false,
			IgnoreActors,
			EDrawDebugTrace::ForOneFrame,
			OutHits,
			true,
			FLinearColor::Red,
			FLinearColor::Green,
			DebugDrawSeconds);
	}

	static void AppendContinuousTraceHits(
		UObject* WorldContextObject,
		const USceneComponent* TraceComponent,
		const FVector& CurrentTraceStart,
		const FVector& CurrentTraceEnd,
		const TArray<TEnumAsByte<EObjectTypeQuery>>& ObjectTypes,
		const TArray<AActor*>& IgnoreActors,
		TMap<TObjectKey<USceneComponent>, FVector>& PreviousTraceStartByComponent,
		TMap<TObjectKey<USceneComponent>, FVector>& PreviousTraceEndByComponent,
		TArray<FHitResult>& OutHits,
		int32& OutBridgeTraceCount,
		int32& OutDirectHitCount,
		int32& OutBridgeHitCount)
	{
		OutBridgeTraceCount = 0;
		OutDirectHitCount = 0;
		OutBridgeHitCount = 0;

		TArray<FHitResult> DirectHits;
		AppendTraceHits(
			WorldContextObject,
			CurrentTraceStart,
			CurrentTraceEnd,
			ObjectTypes,
			IgnoreActors,
			DirectHits);
		OutDirectHitCount = DirectHits.Num();
		OutHits.Append(DirectHits);

		if (!TraceComponent)
		{
			return;
		}

		const TObjectKey<USceneComponent> ComponentKey(TraceComponent);
		const FVector* PreviousTraceStart = PreviousTraceStartByComponent.Find(ComponentKey);
		const FVector* PreviousTraceEnd = PreviousTraceEndByComponent.Find(ComponentKey);

		if (PreviousTraceStart && PreviousTraceEnd)
		{
			TArray<FHitResult> BridgeHits;
			if (!PreviousTraceStart->Equals(CurrentTraceStart))
			{
				AppendTraceHits(
					WorldContextObject,
					*PreviousTraceStart,
					CurrentTraceStart,
					ObjectTypes,
					IgnoreActors,
					BridgeHits);
				++OutBridgeTraceCount;
			}

			if (!PreviousTraceEnd->Equals(CurrentTraceEnd))
			{
				AppendTraceHits(
					WorldContextObject,
					*PreviousTraceEnd,
					CurrentTraceEnd,
					ObjectTypes,
					IgnoreActors,
					BridgeHits);
				++OutBridgeTraceCount;
			}

			OutBridgeHitCount = BridgeHits.Num();
			OutHits.Append(BridgeHits);
		}

		PreviousTraceStartByComponent.Add(ComponentKey, CurrentTraceStart);
		PreviousTraceEndByComponent.Add(ComponentKey, CurrentTraceEnd);
	}
}

UGA_LightAttack::UGA_LightAttack(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	InputType = EInputType::None;
	Montage = nullptr;
	PlayRate = 0.f;
	StartTime = 0.f;
	Montage = nullptr;
	RotationInterpSpeed = 360.0f;
}

bool UGA_LightAttack::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                         const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags,
                                         const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!AvatarActor)
	{
		return false;
	}

	return true;
}

void UGA_LightAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                      const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                      const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	/**
	 * 从TargetData获取参数（支持网络自动复制）
	 * FGameplayAbilityTargetData会自动复制到服务器，客户端和服务器都能获取到相同的数据
	 */
	if (TriggerEventData)
	{
		for (const TSharedPtr<FGameplayAbilityTargetData>& Data : TriggerEventData->TargetData.Data)
		{
			if (Data.IsValid() && Data->GetScriptStruct() == FLightAttackTargetData::StaticStruct())
			{
				FLightAttackTargetData* LightAttackData = static_cast<FLightAttackTargetData*>(Data.Get());
				
				/**
				 * 创建参数对象并填充数据
				 * 客户端和服务器都会执行这段代码，获取到相同的参数
				 */
				InputTag = LightAttackData->InputTag;
				InputType = LightAttackData->InputType;
				Montage = LightAttackData->Montage.Get();
				PlayRate = LightAttackData->PlayRate;
				StartSection = LightAttackData->StartSection;
				StartTime = LightAttackData->StartTime;
				WeaponInstance = LightAttackData->DataWeaponInstance;
				
				break;
			}
		}
	}
	if (!WaitCombatHit && WeaponInstance.IsValid())
	{
		WaitCombatHit = UAT_WaitCombatHit::WaitCombatHit(this,const_cast<UAOWeaponInstance*>(WeaponInstance.Get()));
		WaitCombatHit->ReadyForActivation();
	}

	if (!MovementInputTask)
	{
		MovementInputTask = UAT_WaitMovementInput::WaitMovementInput(this);
		if (MovementInputTask)
		{
			MovementInputTask->OnMovementInputDetected.AddDynamic(this, &UGA_LightAttack::OnMovementInputDetected);
			MovementInputTask->ReadyForActivation();
			UE_LOG(LogAegisOdysseyAbilitySystem, Log, TEXT("UGA_LightAttack::OnRecoveryTagChanged: Started movement input detection"));
		}
	}
	else
	{
		if (MovementInputTask)
		{
			MovementInputTask->EndTask();
			MovementInputTask = nullptr;
			UE_LOG(LogAegisOdysseyAbilitySystem, Log, TEXT("UGA_LightAttack::OnRecoveryTagChanged: Stopped movement input detection"));
		}
	}

	SetCharacterRotationToAttackDirection();

	PlayMontageAnimation();
}

void UGA_LightAttack::PlayMontageAnimation()
{
	const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo();
	UAnimInstance* AnimInstance = ActorInfo ? ActorInfo->GetAnimInstance() : nullptr;
	UE_LOG(LogAegisOdysseyAbilitySystem, Log, TEXT("UGA_LightAttack::PlayMontageAnimation: AnimInstance: %s"), 
		*GetNameSafe(AnimInstance));
    
	if (!AnimInstance)
	{
		UE_LOG(LogAegisOdysseyAbilitySystem, Error, TEXT("UGA_LightAttack::PlayMontageAnimation: AnimInstance is null!"));
		return;
	}
	

	MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		FName("PlayMontageAndWait"),
		Montage,
		PlayRate,
		StartSection,
		true,
		1.0f,
		StartTime,
		false
	);

	if (MontageTask)
	{
		MontageTask->OnBlendOut.AddDynamic(this, &UGA_LightAttack::OnMontageBlendedOut);
		MontageTask->OnCompleted.AddDynamic(this, &UGA_LightAttack::OnMontageCompleted);
		MontageTask->OnInterrupted.AddDynamic(this, &UGA_LightAttack::OnMontageInterrupted);
		MontageTask->OnCancelled.AddDynamic(this, &UGA_LightAttack::OnMontageCancelled);
		MontageTask->ReadyForActivation();
	}
}

void UGA_LightAttack::OnMovementInputDetected()
{
	if (!CancelAbilityTag.IsValid()) return;
	if (GetAbilitySystemComponentFromActorInfo()->HasMatchingGameplayTag(CancelAbilityTag))
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);  //强制和通知服务器结束能力
	}
}

void UGA_LightAttack::OnMontageCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_LightAttack::OnMontageBlendedOut()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_LightAttack::OnMontageInterrupted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UGA_LightAttack::OnMontageCancelled()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UGA_LightAttack::ClearCombatTags()
{
	AAOCharacter* AOCharacter = Cast<AAOCharacter>(GetAvatarActorFromActorInfo());
	if (!AOCharacter)
	{
		UE_LOG(LogAegisOdysseyAbilitySystem, Warning, TEXT("UGA_LightAttack::ClearCombatTags: AOCharacter is null"));
		return;
	}

	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(AOCharacter);
	if (!ASC)
	{
		UE_LOG(LogAegisOdysseyAbilitySystem, Warning, TEXT("UGA_LightAttack::ClearCombatTags: ASC is null"));
		return;
	}

	// 获取蒙太奇中所有的CombatWindowTag
	TArray<FGameplayTag> CombatTags;
	if (Montage)
	{
		GetCombatWindowTagsFromMontage(Montage, CombatTags);
	}

	// 立即移除所有连招窗口标签
	for (const FGameplayTag& Tag : CombatTags)
	{
		if (Tag.IsValid())
		{
			ASC->RemoveLooseGameplayTag(Tag);
			UE_LOG(LogAegisOdysseyAbilitySystem, Log, TEXT("UGA_LightAttack::ClearCombatTags: Removed tag: %s"), *Tag.ToString());
		}
	}
}

void UGA_LightAttack::GetCombatWindowTagsFromMontage(UAnimMontage* InMontage, TArray<FGameplayTag>& OutTags)
{
	if (!InMontage)
	{
		return;
	}

	// 遍历蒙太奇中的所有AnimNotify
	for (const FAnimNotifyEvent& NotifyEvent : InMontage->Notifies)
	{
		// 检查是否是UAOCombatWindow类型的AnimNotifyState
		if (NotifyEvent.NotifyStateClass && NotifyEvent.NotifyStateClass->IsA<UAOCombatWindow>())
		{
			// NotifyEvent.NotifyStateClass指向的是CDO（Class Default Object）
			// Tag的值是在编辑器中配置的，存储在CDO中
			UAOCombatWindow* CombatWindowCDO = Cast<UAOCombatWindow>(NotifyEvent.NotifyStateClass);
			if (!CombatWindowCDO)
			{
				continue;
			}
			
			// 直接使用Getter方法获取Tag值
			FGameplayTag CombatWindowTag = CombatWindowCDO->GetCombatWindowTag();
			if (CombatWindowTag.IsValid())
			{
				OutTags.AddUnique(CombatWindowTag);
				UE_LOG(LogAegisOdysseyAbilitySystem, Log, TEXT("UGA_LightAttack::GetCombatWindowTagsFromMontage: Found CombatWindowTag: %s"), *CombatWindowTag.ToString());
			}
			
			FGameplayTag CombatingTag = CombatWindowCDO->GetCombatingTag();
			if (CombatingTag.IsValid())
			{
				OutTags.AddUnique(CombatingTag);
				UE_LOG(LogAegisOdysseyAbilitySystem, Log, TEXT("UGA_LightAttack::GetCombatWindowTagsFromMontage: Found CombatingTag: %s"), *CombatingTag.ToString());
			}
		}
	}
}

void UGA_LightAttack::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	ClearCombatTags();
	if (MovementInputTask)
	{
		MovementInputTask->EndTask();
		MovementInputTask = nullptr;
	}
	
	if (RotationTask)
	{
		RotationTask->EndTask();
		RotationTask = nullptr;
	}
	
	if (WaitCombatHit)
	{
		WaitCombatHit->EndTask();
		WaitCombatHit = nullptr;
	}
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
	UE_LOG(LogAegisOdysseyAbilitySystem, Warning, TEXT("UGA_LightAttack::EndAbility: Called"));
}

void UGA_LightAttack::SetCharacterRotationToAttackDirection()
{
	if (!CurrentActorInfo)
	{
		return;
	}

	APawn* Pawn = Cast<APawn>(CurrentActorInfo->AvatarActor);
	if (!Pawn)
	{
		return;
	}

	AController* Controller = Pawn->GetController();
	if (!Controller)
	{
		return;
	}
	
	FRotator ControlRotation = Controller->GetControlRotation();
	FRotator TargetRotation = FRotator(0.0f, ControlRotation.Yaw, 0.0f);

	if (!RotationTask)
	{
		RotationTask = UAT_WaitRotateToDirection::WaitRotateToDirection(this, TargetRotation, RotationInterpSpeed);
		if (RotationTask)
		{
			RotationTask->ReadyForActivation();
			UE_LOG(LogAegisOdysseyAbilitySystem, Log, TEXT("UGA_LightAttack::SetCharacterRotationToAttackDirection: Started rotation to Yaw: %.2f with speed: %.2f"), TargetRotation.Yaw, RotationInterpSpeed);
		}
	}
}

UAT_WaitCombatHit::UAT_WaitCombatHit(const FObjectInitializer& ObjectInitializer)
{
	Weapon = nullptr;
	bTickingTask = true;
	
	// 预分配内存，减少动态分配
	HitResultPool.Reserve(100);
	UniqueTargetsPool.Reserve(20);
}

UAT_WaitCombatHit* UAT_WaitCombatHit::WaitCombatHit(UGA_LightAttack* OwningAbility,
	UAOWeaponInstance* WeaponInstance)
{
	UAT_WaitCombatHit* MyObj = NewAbilityTask<UAT_WaitCombatHit>(OwningAbility);
	MyObj->Weapon = WeaponInstance;
	MyObj->LightAttack = OwningAbility;
	return MyObj;
}
void UAT_WaitCombatHit::Activate()
{
	Super::Activate();
	if (!Weapon.IsValid()) EndTask();
	HitTargetsInActiveWindow.Reset();
	bWasHitWindowActiveLastTick = false;
	
	for (AActor* SpawnedActor : Weapon->GetSpawnedActors())
	{
		for (const UActorComponent* Comps : SpawnedActor->GetComponents())
		{
			if (const UStaticMeshComponent* Meshes = Cast<UStaticMeshComponent>(Comps))
			{
				CacheStaticMeshComponents.AddUnique(const_cast<UStaticMeshComponent*>(Meshes));
			}
			if (const USkeletalMeshComponent* SkeletalMesh = Cast<USkeletalMeshComponent>(Comps))
			{
				CacheSkeletalMeshComponents.AddUnique(const_cast<USkeletalMeshComponent*>(SkeletalMesh));
			}
		}
	}

	UE_LOG(
		LogAegisOdysseyCombatTrace,
		Warning,
		TEXT("[CombatTrace][Light] Activate. Ability=%s StaticMeshes=%d SkeletalMeshes=%d"),
		*GetNameSafe(LightAttack.Get()),
		CacheStaticMeshComponents.Num(),
		CacheSkeletalMeshComponents.Num());
}

void UAT_WaitCombatHit::TickTask(float DeltaTime)
{
	Super::TickTask(DeltaTime);
	
	if (!Weapon.IsValid() || !AbilitySystemComponent.IsValid() || !LightAttack.IsValid())
	{
		EndTask();
		return;
	}

	const bool bIsHitWindowActive = AbilitySystemComponent->GetOwnedGameplayTags().HasTagExact(LightAttack->ListenAttackActionTag);
	if (!bIsHitWindowActive)
	{
		if (bWasHitWindowActiveLastTick)
		{
			EndHitWindow();
		}

		bWasHitWindowActiveLastTick = false;
		return;
	}

	if (!bWasHitWindowActiveLastTick)
	{
		BeginHitWindow();
	}

	bWasHitWindowActiveLastTick = true;

	UE_LOG(
		LogAegisOdysseyCombatTrace,
		Warning,
		TEXT("[CombatTrace][Light] Tick active. Ability=%s StaticMeshes=%d SkeletalMeshes=%d"),
		*GetNameSafe(LightAttack.Get()),
		CacheStaticMeshComponents.Num(),
		CacheSkeletalMeshComponents.Num());

	HitResultPool.Empty();
	UniqueTargetsPool.Empty();
	FirstHitResultByTargetPool.Reset();

	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(EObjectTypeQuery::ObjectTypeQuery3);

	TArray<AActor*> IgnoreActors;
	IgnoreActors.AddUnique(Ability->GetAvatarActorFromActorInfo());

	for (const UStaticMeshComponent* MeshComponent : CacheStaticMeshComponents)
	{
		const FTransform StartLocation = MeshComponent->GetSocketTransform("LightAttackStart");
		const FTransform EndLocation = MeshComponent->GetSocketTransform("LightAttackEnd");
		const FVector CurrentTraceStart = StartLocation.GetLocation();
		const FVector CurrentTraceEnd = EndLocation.GetLocation();

		TArray<FHitResult> ComponentHits;
		int32 BridgeTraceCount = 0;
		int32 DirectHitCount = 0;
		int32 BridgeHitCount = 0;
		AOLightAttackTracePrivate::AppendContinuousTraceHits(
			Ability,
			MeshComponent,
			CurrentTraceStart,
			CurrentTraceEnd,
			ObjectTypes,
			IgnoreActors,
			PreviousTraceStartByComponent,
			PreviousTraceEndByComponent,
			ComponentHits,
			BridgeTraceCount,
			DirectHitCount,
			BridgeHitCount);

		UE_LOG(
			LogAegisOdysseyCombatTrace,
			Warning,
			TEXT("[CombatTrace][Light] StaticTrace Component=%s Start=%s End=%s DirectHits=%d BridgeTraces=%d BridgeHits=%d TotalHits=%d HitActors=%s"),
			*GetNameSafe(MeshComponent),
			*CurrentTraceStart.ToCompactString(),
			*CurrentTraceEnd.ToCompactString(),
			DirectHitCount,
			BridgeTraceCount,
			BridgeHitCount,
			ComponentHits.Num(),
			*AOLightAttackTracePrivate::BuildHitActorSummary(ComponentHits));

		HitResultPool.Append(ComponentHits);
	}

	for (const USkeletalMeshComponent* MeshComponent : CacheSkeletalMeshComponents)
	{
		const FTransform StartLocation = MeshComponent->GetSocketTransform("LightAttackStart");
		const FTransform EndLocation = MeshComponent->GetSocketTransform("LightAttackEnd");
		const FVector CurrentTraceStart = StartLocation.GetLocation();
		const FVector CurrentTraceEnd = EndLocation.GetLocation();

		TArray<FHitResult> ComponentHits;
		int32 BridgeTraceCount = 0;
		int32 DirectHitCount = 0;
		int32 BridgeHitCount = 0;
		AOLightAttackTracePrivate::AppendContinuousTraceHits(
			Ability,
			MeshComponent,
			CurrentTraceStart,
			CurrentTraceEnd,
			ObjectTypes,
			IgnoreActors,
			PreviousTraceStartByComponent,
			PreviousTraceEndByComponent,
			ComponentHits,
			BridgeTraceCount,
			DirectHitCount,
			BridgeHitCount);

		UE_LOG(
			LogAegisOdysseyCombatTrace,
			Warning,
			TEXT("[CombatTrace][Light] SkeletalTrace Component=%s Start=%s End=%s DirectHits=%d BridgeTraces=%d BridgeHits=%d TotalHits=%d HitActors=%s"),
			*GetNameSafe(MeshComponent),
			*CurrentTraceStart.ToCompactString(),
			*CurrentTraceEnd.ToCompactString(),
			DirectHitCount,
			BridgeTraceCount,
			BridgeHitCount,
			ComponentHits.Num(),
			*AOLightAttackTracePrivate::BuildHitActorSummary(ComponentHits));

		HitResultPool.Append(ComponentHits);
	}

	ProcessHitsBatch();
}

void UAT_WaitCombatHit::ProcessHitsBatch()
{
	ENetMode NetMode = NM_Standalone;
	if (Ability && Ability->GetWorld())
	{
		NetMode = Ability->GetWorld()->GetNetMode();
		if (NetMode == NM_Client) return;  //客户端没必要具体算法
	}
	
	if (HitResultPool.Num() == 0)
	{
		UE_LOG(
			LogAegisOdysseyCombatTrace,
			Warning,
			TEXT("[CombatTrace][Light] No hits collected. Ability=%s"),
			*GetNameSafe(LightAttack.Get()));
		return;
	}

	// 去重，避免同一帧里同一目标被多个碰撞点重复推进战斗入口。
	for (const FHitResult& Hit : HitResultPool)
	{
		AActor* Target = Hit.GetActor();
		if (Target && Target->Implements<UCombatInterface>())
		{
			const TObjectKey<AActor> TargetKey(Target);
			if (!FirstHitResultByTargetPool.Contains(TargetKey))
			{
				FirstHitResultByTargetPool.Add(TargetKey, Hit);
				UniqueTargetsPool.Add(Target);
			}
		}
	}

	UE_LOG(
		LogAegisOdysseyCombatTrace,
		Warning,
		TEXT("[CombatTrace][Light] RawHits=%d UniqueTargets=%d WindowTrackedTargets=%d Ability=%s"),
		HitResultPool.Num(),
		UniqueTargetsPool.Num(),
		HitTargetsInActiveWindow.Num(),
		*GetNameSafe(LightAttack.Get()));
	
	// 收集武器的Meta效果
	TArray<TSubclassOf<UGameplayEffect>> MetaEffects;
	if (Weapon.IsValid())
	{
		// 获取武器定义
		if (UAOWeaponDefinition* WeaponDef = Cast<UAOWeaponDefinition>(Weapon->GetItemCDO()))
		{
			// 获取武器的AbilitySets
			const TArray<TObjectPtr<UAOAbilitySet>>& AbilitySets = WeaponDef->GetAbilitySetsToGrant();
			for (const TObjectPtr<UAOAbilitySet>& AbilitySet : AbilitySets)
			{
				if (AbilitySet)
				{
					// 收集MetaGameplayEffects
					for (const FAOAbilitySet_MetaGameplayEffect& MetaEffect : AbilitySet->MetaGameplayEffects)
					{
						if (MetaEffect.GameplayEffect)
						{
							MetaEffects.Add(MetaEffect.GameplayEffect);
						}
					}
				}
			}
		}
	}
	
	// 批量应用伤害
	for (AActor* Target : UniqueTargetsPool)
	{
		if (!LightAttack.IsValid())
		{
			continue;
		}

		const TObjectKey<AActor> TargetKey(Target);
		if (HitTargetsInActiveWindow.Contains(TargetKey))
		{
			UE_LOG(
				LogAegisOdysseyCombatTrace,
				Warning,
				TEXT("[CombatTrace][Light] Skip duplicate target in same window. Ability=%s Target=%s"),
				*GetNameSafe(LightAttack.Get()),
				*GetNameSafe(Target));
			continue;
		}

		HitTargetsInActiveWindow.Add(TargetKey);
		
		if (ICombatInterface* CombatInterface = Cast<ICombatInterface>(Target))
		{
			// 构建伤害信息
			FAttackedInfo AttackedInfo;
			AttackedInfo.SourceASC = AbilitySystemComponent.Get();
			AttackedInfo.TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Target);
			AttackedInfo.SourceObject = Weapon.IsValid() ? StaticCast<UObject*>(Weapon.Get()) : StaticCast<UObject*>(LightAttack.Get());
			AttackedInfo.EffectCauser = GetAvatarActor();
			AttackedInfo.AttackTag = LightAttack.IsValid() ? LightAttack->AttackTag : FGameplayTag();
			AttackedInfo.MetaGameplayEffects = MetaEffects;  //传递Meta效果
			if (const FHitResult* FirstHitResult = FirstHitResultByTargetPool.Find(TObjectKey<AActor>(Target)))
			{
				AttackedInfo.HitResult = *FirstHitResult;
			}

			if (Weapon.IsValid())
			{
				if (const UAOWeaponDefinition* WeaponDef = Cast<UAOWeaponDefinition>(Weapon->GetItemCDO()))
				{
					AttackedInfo.WeaponTag = WeaponDef->GetWeaponTag();
					AttackedInfo.DamageTypeTags = WeaponDef->GetDamageTypeTags();
				}
			}
			
			// 应用伤害
			CombatInterface->ApplyDamageToTarget(AttackedInfo);
		}
	}
}

void UAT_WaitCombatHit::OnDestroy(bool bInOwnerFinished)
{
	EndHitWindow();
	Super::OnDestroy(bInOwnerFinished);
	CacheStaticMeshComponents.Reset();
	CacheSkeletalMeshComponents.Reset();
	PreviousTraceStartByComponent.Reset();
	PreviousTraceEndByComponent.Reset();
	FirstHitResultByTargetPool.Reset();
	HitTargetsInActiveWindow.Reset();
}

void UAT_WaitCombatHit::BeginHitWindow()
{
	HitTargetsInActiveWindow.Reset();
	PreviousTraceStartByComponent.Reset();
	PreviousTraceEndByComponent.Reset();
	UE_LOG(
		LogAegisOdysseyCombatTrace,
		Warning,
		TEXT("[CombatTrace][Light] BeginHitWindow. Ability=%s"),
		*GetNameSafe(LightAttack.Get()));
}

void UAT_WaitCombatHit::EndHitWindow()
{
	HitTargetsInActiveWindow.Reset();
	PreviousTraceStartByComponent.Reset();
	PreviousTraceEndByComponent.Reset();
	UE_LOG(
		LogAegisOdysseyCombatTrace,
		Warning,
		TEXT("[CombatTrace][Light] EndHitWindow. Ability=%s"),
		*GetNameSafe(LightAttack.Get()));
}

UAOWeaponInstance* UGA_LightAttack::GetAOWeaponInstance() const
{
	if (AAOCharacter* SourceCharacter = Cast<AAOCharacter>(GetAvatarActorFromActorInfo()))
	{
		UAOWeaponManagerComponent* WeaponManagerComponent = SourceCharacter->GetComponentByClass<UAOWeaponManagerComponent>();
		if (!WeaponManagerComponent) return nullptr;
		return Cast<UAOWeaponInstance>(WeaponManagerComponent->GetCurrentWeaponInstance());
	}
	return nullptr;
}

UAOWeaponDefinition* UGA_LightAttack::GetAOWeaponDefinition() const
{
	if (AAOCharacter* SourceCharacter = Cast<AAOCharacter>(GetAvatarActorFromActorInfo()))
	{
		UAOWeaponManagerComponent* WeaponManagerComponent = SourceCharacter->GetComponentByClass<UAOWeaponManagerComponent>();
		if (!WeaponManagerComponent) return nullptr;
		if (UAOWeaponInstance* Weapon = Cast<UAOWeaponInstance>(WeaponManagerComponent->GetCurrentWeaponInstance()))
		{
			return Cast<UAOWeaponDefinition>( Weapon->GetItemCDO());
		}
	}
	return nullptr;
}
