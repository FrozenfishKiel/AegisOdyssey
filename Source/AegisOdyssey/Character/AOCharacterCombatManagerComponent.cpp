#include "AOCharacterCombatManagerComponent.h"
#include "CoreMinimal.h"
#include "AegisOdyssey/AOAbilityTypes.h"
#include "AegisOdyssey/AOCombatEventTags.h"
#include "AegisOdyssey/AOCombatCueTags.h"
#include "AegisOdyssey/AOCombatMessageSubsystem.h"
#include "AegisOdyssey/AOCombatResultMessage.h"
#include "AegisOdyssey/AOStateTags.h"
#include "AegisOdyssey/AbilitySystem/AOAbilitySystem.h"
#include "AegisOdyssey/AbilitySystem/Abilities/Combat/GA_HitReact.h"
#include "AegisOdyssey/AbilitySystem/Attributes/Combat/AOCombatAttributeSet.h"
#include "AegisOdyssey/AbilitySystem/Attributes/Core/AOHealthAttributeSet.h"
#include "CombatInterface.h"
#include "AbilitySystemComponent.h"
#include "AOCharacter.h"
#include "AegisOdyssey/Character/Enemies/AOEnemy.h"
#include "AOPersistentStateTagComponent.h"
#include "GameplayEffect.h"
#include "AegisOdyssey/Equipment/AOWeaponManagerComponent.h"
#include "AegisOdyssey/AOLogChannels.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AegisOdyssey/Combat/Effects/AOAttackEffectProfile.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "MotionWarpingComponent.h"
#include "TimerManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOCharacterCombatManagerComponent)

namespace AOCharacterCombatManagerComponentPrivate
{
	constexpr float CombatMagnetDebugDuration = 1.0f;
	// 杩欎袱涓?SourceId 鏄寔涔呯姸鎬佹爣绛剧粍浠堕噷鐨勬潵婧愭爣璇嗐€?
	// 杩欐牱鍋氱殑鎰忎箟鏄細鐮撮煣鍜岃寮瑰弽閮借兘鍚勮嚜鐙珛鍔犵姸鎬併€佺嫭绔嬫竻鐘舵€侊紝閬垮厤浜掔浉璇激銆?
	const FName ParriedSourceId(TEXT("Combat.Parried"));
	const FName BrokenSourceId(TEXT("Combat.Broken"));
	const FName HitReactSourceId(TEXT("Combat.HitReact"));

	// 缁熶竴鐢熸垚鈥滄帶鍒堕攣鈥濇爣绛鹃泦鍚堛€?
	// 杩欓噷闄や簡鍏蜂綋鐘舵€佹爣绛惧锛岃繕浼氶澶栧姞涓婅緭鍏ュ皝閿佹爣绛撅紝淇濊瘉瑙掕壊鍦ㄦ帶鍒舵€佷笅鏃笉鑳界Щ鍔ㄤ篃涓嶈兘缁х画鎺ユ敹鑳藉姏杈撳叆銆?
	FGameplayTagContainer MakeControlLockTags(const FGameplayTag& StateTag)
	{
		FGameplayTagContainer Tags;
		if (StateTag.IsValid())
		{
			Tags.AddTag(StateTag);
		}

		Tags.AddTag(FGameplayTag::RequestGameplayTag(TEXT("Gameplay.AbilityInputBlocked")));
		return Tags;
	}

	FGameplayTagContainer MakeHitReactTags(const FGameplayTag& StateTag)
	{
		FGameplayTagContainer Tags;
		if (StateTag.IsValid())
		{
			Tags.AddTag(StateTag);
		}
		return Tags;
	}
}

UAOCharacterCombatManagerComponent::UAOCharacterCombatManagerComponent(const FObjectInitializer& OI)
	:Super(OI)
{
}

void UAOCharacterCombatManagerComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UAOCharacterCombatManagerComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                                      FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UAOCharacterCombatManagerComponent::ApplyDamageToTarget(const FAttackedInfo& AttackedInfo)
{
	// 绗竴闃舵寮€濮嬶紝鎵€鏈夋敾鍑诲懡涓兘搴斿湪杩欓噷姝ｅ紡鏀跺彛杩涚粺涓€鎴樻枟鍏ュ彛銆?
	// 鍛戒腑閲囬泦灞傚彧璐熻矗鍛婅瘔鎴戜滑鈥滆皝鎵撳埌浜嗚皝鈥濓紱鐪熸鐨勬潵婧愯В閲婂拰涓婁笅鏂囨瀯寤轰粠杩欓噷寮€濮嬨€?
	// 鐮撮煣鍙唬琛ㄢ€滃綋鍓嶅浜庡け琛″彈鎺ф€佲€濓紝涓嶄唬琛ㄥ悗缁懡涓笉鍐嶆垚绔嬨€?
	// 鍥犳杩欓噷涓嶈兘鎶?bIsBroken 褰撴垚鎴樻枟缁撶畻鎬诲紑鍏筹紝鍚﹀垯浼氭妸鐮撮煣鍚庣殑杩藉嚮浼ゅ鏁翠綋鍚炴帀銆?
	if (GetOwner() == nullptr || !GetOwner()->HasAuthority())
	{
		UE_LOG(
			LogAegisOdysseyCombatTrace,
			Warning,
			TEXT("[CombatTrace][Manager] Early return: invalid owner/authority. Owner=%s HasAuthority=%s"),
			*GetNameSafe(GetOwner()),
			(GetOwner() != nullptr && GetOwner()->HasAuthority()) ? TEXT("true") : TEXT("false"));
		return;
	}

	if (!AttackedInfo.SourceASC.IsValid() || !AttackedInfo.TargetASC.IsValid())
	{
		UE_LOG(
			LogAegisOdysseyCombatTrace,
			Warning,
			TEXT("[CombatTrace][Manager] Early return: invalid ASC weak ptr. SourceASC=%s TargetASC=%s"),
			AttackedInfo.SourceASC.IsValid() ? TEXT("valid") : TEXT("invalid"),
			AttackedInfo.TargetASC.IsValid() ? TEXT("valid") : TEXT("invalid"));
		return;
	}

	UAbilitySystemComponent* SourceASC = AttackedInfo.SourceASC.Get();
	UAbilitySystemComponent* TargetASC = AttackedInfo.TargetASC.Get();
	if (!SourceASC || !TargetASC)
	{
		UE_LOG(
			LogAegisOdysseyCombatTrace,
			Warning,
			TEXT("[CombatTrace][Manager] Early return: null ASC. SourceASC=%s TargetASC=%s"),
			*GetNameSafe(SourceASC),
			*GetNameSafe(TargetASC));
		return;
	}

	AActor* SourceAvatarActor = SourceASC->GetAvatarActor();
	AActor* TargetAvatarActor = TargetASC->GetAvatarActor();
	if (!SourceAvatarActor || !TargetAvatarActor)
	{
		UE_LOG(
			LogAegisOdysseyCombatTrace,
			Warning,
			TEXT("[CombatTrace][Manager] Early return: null avatars. SourceAvatar=%s TargetAvatar=%s"),
			*GetNameSafe(SourceAvatarActor),
			*GetNameSafe(TargetAvatarActor));
		return;
	}

	UE_LOG(
		LogAegisOdysseyCombatTrace,
		Warning,
		TEXT("[CombatTrace][Manager] Resolve start. Source=%s Target=%s AttackTag=%s SkillTag=%s WeaponTag=%s"),
		*GetNameSafe(SourceAvatarActor),
		*GetNameSafe(TargetAvatarActor),
		*AttackedInfo.AttackTag.ToString(),
		*AttackedInfo.SkillTag.ToString(),
		*AttackedInfo.WeaponTag.ToString());

	// 绗竴闃舵鍏堟妸鏁屾垜鍏崇郴鏀跺彛鐐圭珛鍦ㄧ粺涓€鎴樻枟鍏ュ彛銆?
	// 褰撳墠鍗曚汉 PVE 榛樿鍙厑璁糕€滅帺瀹?<-> 鏁屼汉鈥濅簰鐩告垚绔嬶紝鍚庣画鍙嬩激/闃佃惀绯荤粺缁х画浠庤繖閲屾墿灞曘€?
	//if (!CanResolveCombatBetweenActors(SourceAvatarActor, TargetAvatarActor))
	//{
		//return;
	//}

	// 鍏堢粺涓€璁＄畻鏉ヨ鏂瑰悜涓庨槻寰℃墖鍖哄叧绯汇€?
	// 鍚庣画鏍兼尅鍜屽脊鍙嶉兘渚濊禆杩欓噷鐨勭粨鏋滐紝閬垮厤姣忕闃插尽璇箟鍚勮嚜鍗曠嫭绠椾竴閬嶈搴︺€?
	const FVector TargetLocation = TargetAvatarActor->GetActorLocation();
	const FVector TargetForward = TargetAvatarActor->GetActorForwardVector().GetSafeNormal2D();
	const FVector SourceLocation =
		AttackedInfo.EffectCauser.IsValid()
			? AttackedInfo.EffectCauser->GetActorLocation()
			: SourceAvatarActor->GetActorLocation();
	const FVector IncomingDirection = (SourceLocation - TargetLocation).GetSafeNormal2D();
	const bool bHasValidIncomingDirection = !IncomingDirection.IsNearlyZero();
	const bool bWithinBlockAngle =
		bHasValidIncomingDirection && IsWithinDefenseAngle(TargetForward, IncomingDirection, DefenseConfig.BlockHalfAngleDegrees);
	const bool bWithinParryAngle =
		bHasValidIncomingDirection && IsWithinDefenseAngle(TargetForward, IncomingDirection, DefenseConfig.ParryHalfAngleDegrees);

	const FGameplayTag DamageImmunityTag = TAG_Gameplay_DamageImmunity;
	const FGameplayTag BlockingWindowTag = FGameplayTag::RequestGameplayTag(TEXT("Ability.Block.Blocking"));
	const FGameplayTag ParryWindowTag = FGameplayTag::RequestGameplayTag(TEXT("Ability.Block.Parry"));

	const bool bHitInvulnerability = TargetASC->HasMatchingGameplayTag(DamageImmunityTag);
	const bool bHasParryWindow = TargetASC->HasMatchingGameplayTag(ParryWindowTag);
	const bool bHasBlockWindow = TargetASC->HasMatchingGameplayTag(BlockingWindowTag);
	const bool bHasBlockState = TargetASC->HasMatchingGameplayTag(AOStateTags::State_Combat_Block);

	// 鍏堢粰鏈鍛戒腑鍑嗗鈥滈粯璁ゆ寜姝ｅ父鍙楀嚮瑙ｉ噴鈥濈殑鍩虹缁撴灉锛?
	// 鐒跺悗鍐嶅湪鏃犳晫 / 寮瑰弽 / 鏍兼尅鍒嗘敮閲岄€愭鏀瑰啓銆?
	bool bWasBlocked = false;
	bool bWasParried = false;
	const UObject* SourceObject = AttackedInfo.SourceObject.IsValid() ? AttackedInfo.SourceObject.Get() : static_cast<UObject*>(SourceASC);
	AActor* EffectCauserActor = AttackedInfo.EffectCauser.IsValid() ? AttackedInfo.EffectCauser.Get() : SourceAvatarActor;
	const bool bIsFullBlock = bWithinBlockAngle && bHasBlockWindow;
	const bool bIsPartialBlock = bWithinBlockAngle && bHasBlockState && !bHasBlockWindow;

	const auto SendCombatEvent = [SourceObject](
		AActor* Receiver,
		const FGameplayTag& EventTag,
		AActor* InstigatorActor,
		AActor* TargetActor,
		AActor* EffectCauserActorParam,
		float EventMagnitude)
	{
		if (Receiver == nullptr || !EventTag.IsValid())
		{
			return;
		}

		FGameplayEventData Payload;
		Payload.EventTag = EventTag;
		Payload.Instigator = InstigatorActor;
		Payload.Target = TargetActor;
		Payload.OptionalObject = const_cast<UObject*>(SourceObject);
		Payload.EventMagnitude = EventMagnitude;
		Payload.ContextHandle.AddInstigator(InstigatorActor, EffectCauserActorParam);
		if (SourceObject != nullptr)
		{
			Payload.ContextHandle.AddSourceObject(const_cast<UObject*>(SourceObject));
		}

		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Receiver, EventTag, Payload);
	};

	if (bHitInvulnerability)
	{
		UE_LOG(LogAegisOdysseyCombatTrace, Warning, TEXT("[CombatTrace][Manager] Result=Invulnerable Target=%s"), *GetNameSafe(TargetAvatarActor));
		// 鏃犳晫鍛戒腑鍦ㄥ綋鍓嶆柟妗堥噷鐩存帴瑙嗕负鈥滄病鏈夎繘鍏ユ寮忓彈鍑婚摼鈥濄€?
		// 杩欓噷浠嶇劧骞挎挱缁熶竴缁撴灉锛屼繚璇?UI / 璋冭瘯灞傜煡閬撹繖娆″懡涓濡備綍瑙ｉ噴銆?
		FAOCombatResultMessage ResultMessage;
		ResultMessage.ResultType = EAOCombatResultType::Invulnerable;
		ResultMessage.FloatingTextType = EAOCombatFloatingTextType::None;
		ResultMessage.bShouldDisplayFloatingText = false;
		ResultMessage.bHitInvulnerability = true;
		ResultMessage.Instigator = SourceAvatarActor;
		ResultMessage.Target = TargetAvatarActor;
		ResultMessage.EffectCauser = AttackedInfo.EffectCauser.IsValid() ? AttackedInfo.EffectCauser.Get() : SourceAvatarActor;
		ResultMessage.AttackTag = AttackedInfo.AttackTag;
		ResultMessage.SkillTag = AttackedInfo.SkillTag;
		ResultMessage.WeaponTag = AttackedInfo.WeaponTag;
		ResultMessage.DamageTypeTags = AttackedInfo.DamageTypeTags;
		ResultMessage.HitResult = AttackedInfo.HitResult;
		BroadcastCombatResult(ResultMessage);
		return;
	}

	if (bHasParryWindow && bWithinParryAngle)
	{
		UE_LOG(LogAegisOdysseyCombatTrace, Warning, TEXT("[CombatTrace][Manager] Result=Parry Target=%s"), *GetNameSafe(TargetAvatarActor));
		// 寮瑰弽浼樺厛绾ч珮浜庢櫘閫氭牸鎸″拰姝ｅ父鍙楀嚮銆?
		// 涓€鏃﹀脊鍙嶆垚绔嬶紝鏈鍛戒腑涓嶅啀杩涘叆鏅€氫激瀹冲熬閾撅紝鑰屾槸鐩存帴鏀瑰啓鎴愬脊鍙嶇粨鏋溿€?
		bWasParried = true;
		SendCombatEvent(
			TargetAvatarActor,
			AOCombatEventTags::GameplayEvent_Combat_Block_ParrySuccess,
			SourceAvatarActor,
			TargetAvatarActor,
			EffectCauserActor,
			DefenseConfig.ParryVigorCost);

		bool bAttackerBrokenByParry = false;
		if (UAOCharacterCombatManagerComponent* SourceCombatManager = SourceAvatarActor->FindComponentByClass<UAOCharacterCombatManagerComponent>())
		{
			// 寮瑰弽鍛戒腑鐨勪氦浜掓牳蹇冧笉鍙槸鈥滃彇娑堣繖娆′激瀹斥€濓紝杩樿瀵硅繘鏀绘柟杩涜姝ｅ紡璧勬簮涓庡姩浣滄墦鏂鐞嗐€?
			SourceCombatManager->InterruptAllAbilities(SourceASC);
			SourceCombatManager->HandleParriedReaction();
			bAttackerBrokenByParry = SourceASC->HasMatchingGameplayTag(AOStateTags::State_Combat_Broken);
		}
		else
		{
			InterruptAllAbilities(SourceASC);
		}

		FAOCombatResultMessage ResultMessage;
		ResultMessage.ResultType = EAOCombatResultType::Parry;
		ResultMessage.FloatingTextType = EAOCombatFloatingTextType::Parry;
		ResultMessage.bShouldDisplayFloatingText = true;
		ResultMessage.bWasParried = true;
		ResultMessage.bTargetBroken = bAttackerBrokenByParry;
		ResultMessage.Instigator = TargetAvatarActor;
		ResultMessage.Target = SourceAvatarActor;
		ResultMessage.EffectCauser = AttackedInfo.EffectCauser.IsValid() ? AttackedInfo.EffectCauser.Get() : SourceAvatarActor;
		ResultMessage.CueTag = AOCombatCueTags::GameplayCue_Combat_Parry;
		ResultMessage.AttackTag = AttackedInfo.AttackTag;
		ResultMessage.SkillTag = AttackedInfo.SkillTag;
		ResultMessage.WeaponTag = AttackedInfo.WeaponTag;
		ResultMessage.DamageTypeTags = AttackedInfo.DamageTypeTags;
		ResultMessage.StaminaDamage = DefenseConfig.ParryAttackerStaminaDamage;
		ResultMessage.VigorCost = DefenseConfig.ParryVigorCost;
		ResultMessage.HitResult = AttackedInfo.HitResult;
		BroadcastCombatResult(ResultMessage);
		const FGameplayEffectContextHandle CueEffectContext = BuildCombatCueEffectContext(
			TargetAvatarActor,
			ResultMessage.EffectCauser,
			const_cast<UObject*>(SourceObject),
			&AttackedInfo.HitResult,
			ResultMessage.AttackTag,
			ResultMessage.SkillTag,
			ResultMessage.WeaponTag,
			ResultMessage.DamageTypeTags,
			false,
			false,
			true,
			false,
			bAttackerBrokenByParry);
		ExecuteCombatCue(
			SourceASC,
			ResultMessage.CueTag,
			&CueEffectContext,
			&AttackedInfo.HitResult,
			TargetAvatarActor,
			ResultMessage.EffectCauser,
			const_cast<UObject*>(SourceObject),
			ResultMessage.StaminaDamage);

		return;
	}

	if (bIsFullBlock)
	{
		bWasBlocked = true;
		SendCombatEvent(
			TargetAvatarActor,
			AOCombatEventTags::GameplayEvent_Combat_Block_FullBlockSuccess,
			SourceAvatarActor,
			TargetAvatarActor,
			EffectCauserActor,
			DefenseConfig.FullBlockVigorCost);

		const bool bTargetBroken = TargetASC->HasMatchingGameplayTag(AOStateTags::State_Combat_Broken);
		UE_LOG(LogAegisOdysseyCombatTrace, Warning, TEXT("[CombatTrace][Manager] Result=Blocked Target=%s"), *GetNameSafe(TargetAvatarActor));
		FAOCombatResultMessage ResultMessage;
		ResultMessage.ResultType = EAOCombatResultType::Blocked;
		ResultMessage.FloatingTextType = EAOCombatFloatingTextType::None;
		ResultMessage.bShouldDisplayFloatingText = false;
		ResultMessage.bWasBlocked = true;
		ResultMessage.bTargetBroken = bTargetBroken;
		ResultMessage.Instigator = SourceAvatarActor;
		ResultMessage.Target = TargetAvatarActor;
		ResultMessage.EffectCauser = EffectCauserActor;
		ResultMessage.CueTag = AOCombatCueTags::GameplayCue_Combat_Block;
		ResultMessage.AttackTag = AttackedInfo.AttackTag;
		ResultMessage.SkillTag = AttackedInfo.SkillTag;
		ResultMessage.WeaponTag = AttackedInfo.WeaponTag;
		ResultMessage.DamageTypeTags = AttackedInfo.DamageTypeTags;
		ResultMessage.StaminaDamage = DefenseConfig.BlockStaminaDamage;
		ResultMessage.VigorCost = DefenseConfig.FullBlockVigorCost;
		ResultMessage.HitResult = AttackedInfo.HitResult;
		BroadcastCombatResult(ResultMessage);

		const FGameplayEffectContextHandle CueEffectContext = BuildCombatCueEffectContext(
			SourceAvatarActor,
			ResultMessage.EffectCauser,
			const_cast<UObject*>(SourceObject),
			&AttackedInfo.HitResult,
			ResultMessage.AttackTag,
			ResultMessage.SkillTag,
			ResultMessage.WeaponTag,
			ResultMessage.DamageTypeTags,
			false,
			true,
			false,
			false,
			bTargetBroken);
		ExecuteCombatCue(
			TargetASC,
			ResultMessage.CueTag,
			&CueEffectContext,
			&AttackedInfo.HitResult,
			SourceAvatarActor,
			ResultMessage.EffectCauser,
			const_cast<UObject*>(SourceObject),
			0.0f);

		return;
	}
	else if (bIsPartialBlock)
	{
		bWasBlocked = true;
		SendCombatEvent(
			TargetAvatarActor,
			AOCombatEventTags::GameplayEvent_Combat_Block_PartialBlockSuccess,
			SourceAvatarActor,
			TargetAvatarActor,
			EffectCauserActor,
			DefenseConfig.PartialBlockVigorCost);
	}

	FGameplayEffectContextHandle ContextHandle = SourceASC->MakeEffectContext();
	ContextHandle.AddInstigator(
		SourceAvatarActor,
		AttackedInfo.EffectCauser.IsValid() ? AttackedInfo.EffectCauser.Get() : SourceAvatarActor);

	// SourceObject / HitResult 閮藉湪杩欓噷鍥炲～杩涚粺涓€涓婁笅鏂囷紝
	// 淇濊瘉鍚庣画 AttributeSet銆丆ue銆乁I 閮借兘鍩轰簬鍚屼竴浠界粨绠椾笂涓嬫枃宸ヤ綔銆?
	if (AttackedInfo.SourceObject.IsValid())
	{
		ContextHandle.AddSourceObject(AttackedInfo.SourceObject.Get());
	}
	else
	{
		ContextHandle.AddSourceObject(SourceASC);
	}

	if (AttackedInfo.HitResult.bBlockingHit || AttackedInfo.HitResult.GetActor() != nullptr)
	{
		ContextHandle.AddHitResult(AttackedInfo.HitResult, true);
	}

	if (FAOGameplayEffectContext* AOContext = static_cast<FAOGameplayEffectContext*>(ContextHandle.Get()))
	{
		// 杩欓噷寮€濮嬶紝鎶婂凡缁忚В閲婂畬鎴愮殑闃插尽鐪熺浉鍐欏洖缁熶竴涓婁笅鏂囥€?
		// 鍚庣画 ExecCal銆丄ttributeSet銆乁I銆丆ue 閮藉彧璇昏繖浜涘瓧娈碉紝涓嶅啀鑷繁閲嶅垽銆?
		UAOAttackEffectProfile* AttackEffectProfile = nullptr;
		if (const UAOAttackEffectProfile* ResolvedAttackEffectProfile =
			FAOAttackEffectProfileRuntime::ResolveProfileFromSourceObject(AttackedInfo.SourceObject.Get()))
		{
			AttackEffectProfile = const_cast<UAOAttackEffectProfile*>(ResolvedAttackEffectProfile);
		}
		else if (const UAOAttackEffectProfile* FallbackAttackEffectProfile =
			FAOAttackEffectProfileRuntime::ResolveProfileFromActor(SourceAvatarActor))
		{
			AttackEffectProfile = const_cast<UAOAttackEffectProfile*>(FallbackAttackEffectProfile);
		}

		AOContext->SetAttackTag(AttackedInfo.AttackTag);
		AOContext->SetSkillTag(AttackedInfo.SkillTag);
		AOContext->SetWeaponTag(AttackedInfo.WeaponTag);
		AOContext->SetDamageTypeTags(AttackedInfo.DamageTypeTags);
		AOContext->SetAttackEffectProfile(AttackEffectProfile);
		AOContext->SetWasBlocked(bWasBlocked);
		AOContext->SetWasParried(bWasParried);
		AOContext->SetHitInvulnerability(bHitInvulnerability);
	}

	for (const TSubclassOf<UGameplayEffect>& EffectClass : AttackedInfo.MetaGameplayEffects)
	{
		// CombatManager 涓嶇洿鎺ュ湪杩欓噷鏀圭敓鍛藉€硷紝
		// 鑰屾槸鎶婂凡缁忚В閲婂ソ鐨勬垬鏂楃湡鐩稿啓鍏?GE 涓婁笅鏂囷紝鍐嶈鍚庣画浼ゅ閾炬甯歌惤鍒?AttributeSet銆?
		FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(EffectClass, 1.0f, ContextHandle);
		if (SpecHandle.IsValid())
		{
			UE_LOG(
				LogAegisOdysseyCombatTrace,
				Warning,
				TEXT("[CombatTrace][Manager] Apply meta effect. Source=%s Target=%s Effect=%s bWasBlocked=%s bWasParried=%s"),
				*GetNameSafe(SourceAvatarActor),
				*GetNameSafe(TargetAvatarActor),
				*GetNameSafe(EffectClass.Get()),
				bWasBlocked ? TEXT("true") : TEXT("false"),
				bWasParried ? TEXT("true") : TEXT("false"));
			SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
		}
	}
}

void UAOCharacterCombatManagerComponent::HandleConfirmedHitReact(const FAOGameplayEffectContext& EffectContext, const float FinalDamage)
{
	if (GetOwner() == nullptr || !GetOwner()->HasAuthority() || bIsParriedReacting)
	{
		return;
	}

	AAOCharacter* OwnerCharacter = Cast<AAOCharacter>(GetOwner());
	if (OwnerCharacter == nullptr)
	{
		return;
	}

	UAbilitySystemComponent* TargetASC = OwnerCharacter->GetAbilitySystemComponent();
	if (TargetASC == nullptr)
	{
		return;
	}

	const EAOHitReactLevel NewLevel = ResolveHitReactLevel(TargetASC, EffectContext.GetResolvedHitReactStrength());
	if (NewLevel == EAOHitReactLevel::Ignore)
	{
		return;
	}

	if (ShouldRejectHitReactLevel(TargetASC, NewLevel))
	{
		return;
	}

	const FGameplayTag StateTag = ResolveHitReactStateTag(NewLevel);
	if (!StateTag.IsValid())
	{
		return;
	}

	EndHitReactState();
	InterruptAllAbilities(TargetASC);
	ApplyHitReactState(StateTag, true);
	SetOwnerControlLocked(true);

	FGameplayEventData Payload;
	Payload.EventTag = AOCombatEventTags::GameplayEvent_Combat_HitReact_Activate;
	Payload.Instigator = EffectContext.GetOriginalInstigator();
	Payload.Target = OwnerCharacter;
	Payload.OptionalObject = EffectContext.GetSourceObject();
	Payload.EventMagnitude = FinalDamage;

	FHitReactTargetData* HitReactData = new FHitReactTargetData();
	HitReactData->StateTag = StateTag;
	HitReactData->SourceDirection = ResolveHitReactSourceDirection(EffectContext);

	FGameplayAbilityTargetDataHandle TargetDataHandle;
	TargetDataHandle.Data.Add(TSharedPtr<FGameplayAbilityTargetData>(HitReactData));
	Payload.TargetData = TargetDataHandle;

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(OwnerCharacter, Payload.EventTag, Payload);
}

void UAOCharacterCombatManagerComponent::HandleParriedReaction()
{
	if (GetOwner() == nullptr || !GetOwner()->HasAuthority() || bIsBroken)
	{
		return;
	}

	// 鍏堟竻鐞嗘棫瀹氭椂鍣紝閬垮厤杩炵画瑙﹀彂鏃跺嚭鐜扮姸鎬佹椂闀垮彔鍔犳垨鎻愬墠鎭㈠銆?
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ParriedReactionTimerHandle);
	}

	bIsParriedReacting = true;
	ApplyPersistentControlState(
		AOCharacterCombatManagerComponentPrivate::ParriedSourceId,
		AOCharacterCombatManagerComponentPrivate::MakeControlLockTags(AOStateTags::State_Combat_Parried),
		true);
	SetOwnerControlLocked(true);

	// 琚脊鍙嶅弽搴旂殑鐩爣鏄€滅珛鍗虫墦鏂綘褰撳墠鍔ㄤ綔骞惰浣犵煭鏆傚兊鐩粹€濓紝
	// 鎵€浠ヨ繖閲屼細鍚屾椂娓呰兘鍔涖€侀攣杈撳叆銆佸仠绉诲姩銆?
	if (UAbilitySystemComponent* AbilitySystemComponent = Cast<AAOCharacter>(GetOwner()) != nullptr
		? Cast<AAOCharacter>(GetOwner())->GetAbilitySystemComponent()
		: nullptr)
	{
		InterruptAllAbilities(AbilitySystemComponent);
	}

	if (AAOCharacter* OwnerCharacter = Cast<AAOCharacter>(GetOwner()))
	{
		FGameplayEventData Payload;
		Payload.EventTag = AOCombatEventTags::GameplayEvent_Combat_ParriedReact_Activate;
		Payload.Instigator = OwnerCharacter;
		Payload.Target = OwnerCharacter;
		Payload.EventMagnitude = DefenseConfig.ParryAttackerStaminaDamage;
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(OwnerCharacter, Payload.EventTag, Payload);
	}
}

void UAOCharacterCombatManagerComponent::HandleBrokenState(
	AActor* InstigatorActor,
	AActor* EffectCauserActor,
	const UObject* SourceObject)
{
	if (GetOwner() == nullptr || !GetOwner()->HasAuthority())
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		// 鐮撮煣浼樺厛绾ч珮浜庤寮瑰弽纭洿銆?
		// 涓€鏃︽寮忚繘鍏ョ牬闊э紝鏃х殑琚脊鍙嶅畾鏃跺櫒鍜岀牬闊у畾鏃跺櫒閮借閲嶆柊鏀舵潫銆?
		World->GetTimerManager().ClearTimer(ParriedReactionTimerHandle);
		World->GetTimerManager().ClearTimer(BrokenStateTimerHandle);
	}

	EndParriedReaction();
	bIsBroken = true;

	// 鐮撮煣鏄竴涓嫭绔嬫寮忕粨鏋溿€?
	// 鍗充娇鍚庣画鍔ㄧ敾琛ㄧ幇杩樻病鎺ュ畬锛岃繖閲屼篃瑕佸厛鎶婃秷鎭笌 Cue 鐨勬敹鍙ｇ偣鍥哄畾涓嬫潵銆?
	FAOCombatResultMessage ResultMessage;
	ResultMessage.ResultType = EAOCombatResultType::Broken;
	ResultMessage.FloatingTextType = EAOCombatFloatingTextType::Broken;
	ResultMessage.bShouldDisplayFloatingText = true;
	ResultMessage.bTargetBroken = true;
	ResultMessage.Instigator = InstigatorActor;
	ResultMessage.Target = Cast<AActor>(GetOwner());
	ResultMessage.EffectCauser = EffectCauserActor != nullptr ? EffectCauserActor : InstigatorActor;
	ResultMessage.CueTag = AOCombatCueTags::GameplayCue_Combat_Broken;
	BroadcastCombatResult(ResultMessage);

	ApplyPersistentControlState(
		AOCharacterCombatManagerComponentPrivate::BrokenSourceId,
		AOCharacterCombatManagerComponentPrivate::MakeControlLockTags(AOStateTags::State_Combat_Broken),
		true);
	SetOwnerControlLocked(true);

	if (UAbilitySystemComponent* AbilitySystemComponent = Cast<AAOCharacter>(GetOwner()) != nullptr
		? Cast<AAOCharacter>(GetOwner())->GetAbilitySystemComponent()
		: nullptr)
	{
		InterruptAllAbilities(AbilitySystemComponent);
	}

	if (UWorld* World = GetWorld())
	{
		// 褰撳墠鏂规涓殑鐮撮煣鎭㈠璇箟鏄€滈攣瀹氫竴娈垫椂闂村悗鐩存帴鍥炴弧闊ф€р€濓紝
		// 鎵€浠ヨ繖閲屼娇鐢ㄤ竴娆℃€у畾鏃跺櫒锛屽湪缁撴潫鏃剁粺涓€鎭㈠銆?
		World->GetTimerManager().SetTimer(
			BrokenStateTimerHandle,
			this,
			&ThisClass::EndBrokenState,
			DefenseConfig.BrokenDurationSeconds,
			false);
	}

	if (AAOCharacter* OwnerCharacter = Cast<AAOCharacter>(GetOwner()))
	{
		if (UAbilitySystemComponent* AbilitySystemComponent = OwnerCharacter->GetAbilitySystemComponent())
		{
			const FGameplayEffectContextHandle CueEffectContext = BuildCombatCueEffectContext(
				InstigatorActor != nullptr ? InstigatorActor : Cast<AActor>(OwnerCharacter),
				EffectCauserActor != nullptr ? EffectCauserActor : Cast<AActor>(OwnerCharacter),
				const_cast<UObject*>(SourceObject != nullptr ? SourceObject : static_cast<const UObject*>(OwnerCharacter)),
				nullptr,
				ResultMessage.AttackTag,
				ResultMessage.SkillTag,
				ResultMessage.WeaponTag,
				ResultMessage.DamageTypeTags,
				false,
				false,
				false,
				false,
				true);
			ExecuteCombatCue(
				AbilitySystemComponent,
				AOCombatCueTags::GameplayCue_Combat_Broken,
				&CueEffectContext,
				nullptr,
				InstigatorActor != nullptr ? InstigatorActor : Cast<AActor>(OwnerCharacter),
				EffectCauserActor != nullptr ? EffectCauserActor : Cast<AActor>(OwnerCharacter),
				const_cast<UObject*>(SourceObject != nullptr ? SourceObject : static_cast<const UObject*>(OwnerCharacter)),
				0.0f);
		}
	}
}

void UAOCharacterCombatManagerComponent::EndParriedReaction()
{
	if (!bIsParriedReacting)
	{
		return;
	}

	// 鍙竻鐞嗗睘浜庘€滆寮瑰弽纭洿鈥濊繖涓€鏉ユ簮鐨勬寔涔呯姸鎬併€?
	// 濡傛灉瑙掕壊姝ゆ椂宸茬粡杩涘叆鐮撮煣锛屽氨涓嶈兘鍦ㄨ繖閲屾妸鎬绘帶鍒堕攣鎻愬墠瑙ｅ紑銆?
	bIsParriedReacting = false;
	ApplyPersistentControlState(
		AOCharacterCombatManagerComponentPrivate::ParriedSourceId,
		FGameplayTagContainer(),
		false);

	if (!bIsBroken)
	{
		SetOwnerControlLocked(false);
	}
}

void UAOCharacterCombatManagerComponent::EndBrokenState()
{
	if (!bIsBroken)
	{
		return;
	}

	// 鐮撮煣缁撴潫鏃讹紝褰撳墠鏂规瑕佹眰闊ф€ф潯鐩存帴琛ユ弧锛岃€屼笉鏄儚鍙嫾閭ｆ牱缂撴參鍛煎惛鎭㈠銆?
	bIsBroken = false;
	ApplyPersistentControlState(
		AOCharacterCombatManagerComponentPrivate::BrokenSourceId,
		FGameplayTagContainer(),
		false);
	SetOwnerControlLocked(false);

	if (const AAOCharacter* OwnerCharacter = Cast<AAOCharacter>(GetOwner()))
	{
		if (UAbilitySystemComponent* AbilitySystemComponent = OwnerCharacter->GetAbilitySystemComponent())
		{
			const float CurrentStamina = AbilitySystemComponent->GetNumericAttribute(UAOCombatAttributeSet::GetStaminaAttribute());
			const float MaxStamina = AbilitySystemComponent->GetNumericAttribute(UAOCombatAttributeSet::GetMaxStaminaAttribute());
			ApplyAttributeDelta(AbilitySystemComponent, UAOCombatAttributeSet::GetStaminaAttribute(), MaxStamina - CurrentStamina);
		}
	}
}

void UAOCharacterCombatManagerComponent::EndHitReactState()
{
	ApplyHitReactState(FGameplayTag(), false);
	if (!bIsBroken && !bIsParriedReacting)
	{
		SetOwnerControlLocked(false);
	}
}

bool UAOCharacterCombatManagerComponent::BeginCombatMagnetWindow(const FAOCombatMagnetWindowConfig& WindowConfig)
{
	if (WindowConfig.WarpTargetName.IsNone())
	{
		return false;
	}

	const AAOCharacter* OwnerCharacter = Cast<AAOCharacter>(GetOwner());
	if (OwnerCharacter == nullptr)
	{
		return false;
	}

	UMotionWarpingComponent* MotionWarpingComponent = OwnerCharacter->GetMotionWarpingComponent();
	if (MotionWarpingComponent == nullptr)
	{
		return false;
	}

	const FVector SearchForward = ResolveCombatMagnetSearchForward(*OwnerCharacter);
	AActor* SelectedTarget = FindBestCombatMagnetTarget(*OwnerCharacter, WindowConfig, SearchForward);

	if (WindowConfig.bDrawDebug)
	{
		DrawCombatMagnetDebug(*OwnerCharacter, WindowConfig, SearchForward, SelectedTarget);
	}

	if (SelectedTarget == nullptr)
	{
		MotionWarpingComponent->RemoveWarpTarget(WindowConfig.WarpTargetName);
		return false;
	}

	MotionWarpingComponent->AddOrUpdateWarpTargetFromLocation(
		WindowConfig.WarpTargetName,
		SelectedTarget->GetActorLocation());
	return true;
}

void UAOCharacterCombatManagerComponent::EndCombatMagnetWindow(const FName WarpTargetName)
{
	if (WarpTargetName.IsNone())
	{
		return;
	}

	const AAOCharacter* OwnerCharacter = Cast<AAOCharacter>(GetOwner());
	if (OwnerCharacter == nullptr)
	{
		return;
	}

	if (UMotionWarpingComponent* MotionWarpingComponent = OwnerCharacter->GetMotionWarpingComponent())
	{
		MotionWarpingComponent->RemoveWarpTarget(WarpTargetName);
	}
}

void UAOCharacterCombatManagerComponent::BroadcastCombatResult(const FAOCombatResultMessage& Message) const
{
	// CombatManager 鍙礋璐ｅ湪缁熶竴缁撶畻鍑哄彛骞挎挱缁撴灉銆?
	// 璋佹潵鏄剧ず銆佽皝鏉ユ挱鍔ㄧ敾銆佽皝鏉ヨ烦瀛楋紝閮藉簲鍦ㄨ闃呬晶瑙ｅ喅銆?
	if (UAOCombatMessageSubsystem* CombatMessageSubsystem = UAOCombatMessageSubsystem::Get(this))
	{
		CombatMessageSubsystem->BroadcastCombatResult(Message);
	}
}

void UAOCharacterCombatManagerComponent::ExecuteCombatCue(
	UAbilitySystemComponent* AbilitySystemComponent,
	const FGameplayTag& CueTag,
	const FGameplayEffectContextHandle* EffectContextHandle,
	const FHitResult* HitResult,
	AActor* InstigatorActor,
	AActor* EffectCauserActor,
	UObject* SourceObject,
	float RawMagnitude) const
{
	if (AbilitySystemComponent == nullptr || !CueTag.IsValid())
	{
		return;
	}

	// GameplayCue 鍦ㄨ繖閲屾槸绾〃鐜板叆鍙ｃ€?
	// 瀹冩秷璐瑰凡缁忕‘瀹氱殑鎴樻枟缁撴灉锛屼笉鍙備笌浠讳綍鍛戒腑/鏍兼尅/寮瑰弽鐪熺浉鍒ゆ柇銆?
	FGameplayCueParameters CueParameters;
	if (EffectContextHandle != nullptr)
	{
		CueParameters.EffectContext = *EffectContextHandle;
	}

	CueParameters.RawMagnitude = RawMagnitude;
	CueParameters.Instigator = InstigatorActor;
	CueParameters.EffectCauser = EffectCauserActor;
	CueParameters.SourceObject = SourceObject;

	if (HitResult != nullptr && HitResult->bBlockingHit)
	{
		CueParameters.Location = HitResult->ImpactPoint;
		CueParameters.Normal = HitResult->ImpactNormal.GetSafeNormal();
		CueParameters.PhysicalMaterial = HitResult->PhysMaterial.Get();
	}
	else if (AActor* AvatarActor = AbilitySystemComponent->GetAvatarActor())
	{
		CueParameters.Location = AvatarActor->GetActorLocation();
	}

	AbilitySystemComponent->ExecuteGameplayCue(CueTag, CueParameters);
}

FGameplayEffectContextHandle UAOCharacterCombatManagerComponent::BuildCombatCueEffectContext(
	AActor* InstigatorActor,
	AActor* EffectCauserActor,
	UObject* SourceObject,
	const FHitResult* HitResult,
	const FGameplayTag& AttackTag,
	const FGameplayTag& SkillTag,
	const FGameplayTag& WeaponTag,
	const FGameplayTagContainer& DamageTypeTags,
	const bool bIsCritical,
	const bool bWasBlocked,
	const bool bWasParried,
	const bool bHitInvulnerability,
	const bool bTargetBroken) const
{
	FGameplayEffectContextHandle ContextHandle = FGameplayEffectContextHandle(new FAOGameplayEffectContext());
	ContextHandle.AddInstigator(InstigatorActor, EffectCauserActor);
	if (SourceObject != nullptr)
	{
		ContextHandle.AddSourceObject(SourceObject);
	}

	if (HitResult != nullptr && (HitResult->bBlockingHit || HitResult->GetActor() != nullptr))
	{
		ContextHandle.AddHitResult(*HitResult, true);
	}

	if (FAOGameplayEffectContext* CombatContext = static_cast<FAOGameplayEffectContext*>(ContextHandle.Get()))
	{
		CombatContext->SetAttackTag(AttackTag);
		CombatContext->SetSkillTag(SkillTag);
		CombatContext->SetWeaponTag(WeaponTag);
		CombatContext->SetDamageTypeTags(DamageTypeTags);
		CombatContext->SetIsCritical(bIsCritical);
		CombatContext->SetWasBlocked(bWasBlocked);
		CombatContext->SetWasParried(bWasParried);
		CombatContext->SetHitInvulnerability(bHitInvulnerability);
		CombatContext->SetTargetBroken(bTargetBroken);
	}

	return ContextHandle;
}

float UAOCharacterCombatManagerComponent::ApplyAttributeDelta(
	UAbilitySystemComponent* AbilitySystemComponent,
	const FGameplayAttribute& Attribute,
	float DeltaValue) const
{
	if (AbilitySystemComponent == nullptr || !Attribute.IsValid() || FMath::IsNearlyZero(DeltaValue))
	{
		return AbilitySystemComponent != nullptr ? AbilitySystemComponent->GetNumericAttribute(Attribute) : 0.0f;
	}

	AbilitySystemComponent->ApplyModToAttribute(Attribute, EGameplayModOp::Additive, DeltaValue);
	return AbilitySystemComponent->GetNumericAttribute(Attribute);
}

bool UAOCharacterCombatManagerComponent::IsWithinDefenseAngle(
	const FVector& DefenderForward,
	const FVector& IncomingDirection,
	float HalfAngleDegrees) const
{
	const float CosThreshold = FMath::Cos(FMath::DegreesToRadians(FMath::Clamp(HalfAngleDegrees, 0.0f, 180.0f)));
	return FVector::DotProduct(DefenderForward.GetSafeNormal2D(), IncomingDirection.GetSafeNormal2D()) >= CosThreshold;
}

bool UAOCharacterCombatManagerComponent::CanResolveCombatBetweenActors(const AActor* SourceActor, const AActor* TargetActor) const
{
	if (SourceActor == nullptr || TargetActor == nullptr || SourceActor == TargetActor)
	{
		return false;
	}

	const bool bSourceIsEnemy = SourceActor->IsA<AAOEnemy>();
	const bool bTargetIsEnemy = TargetActor->IsA<AAOEnemy>();

	// 褰撳墠榛樿瑙勫垯鍙厑璁糕€滅帺瀹舵墦鏁屼汉 / 鏁屼汉鎵撶帺瀹垛€濄€?
	// 杩欓噷灏辨槸鏈潵鎵╂垚瀹屾暣鏁屾垜/闃佃惀/鍙嬩激绯荤粺鐨勭粺涓€鏈€缁堝垽瀹氱偣銆?
	return bSourceIsEnemy != bTargetIsEnemy;
}

void UAOCharacterCombatManagerComponent::InterruptAllAbilities(UAbilitySystemComponent* AbilitySystemComponent) const
{
	if (AbilitySystemComponent == nullptr)
	{
		return;
	}

	// 杩欓噷缁熶竴娓呯┖褰撳墠鑳藉姏鍜岃緭鍏ワ紝閬垮厤瑙掕壊鍦ㄧ牬闊?/ 琚脊鍙嶅悗浠嶇劧鎶婃棫鍔ㄤ綔缁х画鎾笅鍘汇€?
	AbilitySystemComponent->CancelAllAbilities();

	if (UAOAbilitySystem* AOAbilitySystem = Cast<UAOAbilitySystem>(AbilitySystemComponent))
	{
		AOAbilitySystem->ClearAbilityInput();
	}
}

void UAOCharacterCombatManagerComponent::SetOwnerControlLocked(bool bLocked) const
{
	AAOCharacter* OwnerCharacter = Cast<AAOCharacter>(GetOwner());
	if (OwnerCharacter == nullptr)
	{
		return;
	}

	if (AController* Controller = OwnerCharacter->GetController())
	{
		Controller->SetIgnoreMoveInput(bLocked);
	}

	if (UCharacterMovementComponent* MovementComponent = OwnerCharacter->GetCharacterMovement())
	{
		if (bLocked)
		{
			// 閿佹帶鍒舵椂绔嬪嵆鍋滀笅骞跺叧闂Щ鍔紝閬垮厤瑙掕壊闈犳畫浣欓€熷害婊戝嚭鍘汇€?
			MovementComponent->StopMovementImmediately();
			MovementComponent->DisableMovement();
		}
		else if (MovementComponent->MovementMode == MOVE_None)
		{
			// 鎭㈠鎺у埗鏃跺彧鍦ㄥ綋鍓嶄粛鏄畬鍏ㄧ鐢ㄧЩ鍔ㄧ殑鎯呭喌涓嬫仮澶?Walking锛?
			// 閬垮厤鎶婂叾浠栫郴缁熶富鍔ㄨ缃殑鐗规畩绉诲姩妯″紡寮鸿瑕嗙洊鎺夈€?
			MovementComponent->SetMovementMode(MOVE_Walking);
		}
	}
}

void UAOCharacterCombatManagerComponent::ApplyPersistentControlState(
	FName SourceId,
	const FGameplayTagContainer& Tags,
	bool bApply) const
{
	AAOCharacter* OwnerCharacter = Cast<AAOCharacter>(GetOwner());
	if (OwnerCharacter == nullptr)
	{
		return;
	}

	if (UAOPersistentStateTagComponent* PersistentStateTagComponent =
		OwnerCharacter->FindComponentByClass<UAOPersistentStateTagComponent>())
	{
		if (bApply)
		{
			PersistentStateTagComponent->EnsureTagsGranted(SourceId, Tags);
		}
		else
		{
			PersistentStateTagComponent->ClearTagsBySource(SourceId);
		}
	}
}

void UAOCharacterCombatManagerComponent::ApplyHitReactState(const FGameplayTag& StateTag, const bool bApply)
{
	AAOCharacter* OwnerCharacter = Cast<AAOCharacter>(GetOwner());
	if (OwnerCharacter == nullptr)
	{
		return;
	}

	if (UAOPersistentStateTagComponent* PersistentStateTagComponent = OwnerCharacter->FindComponentByClass<UAOPersistentStateTagComponent>())
	{
		if (bApply)
		{
			PersistentStateTagComponent->EnsureTagsGranted(
				AOCharacterCombatManagerComponentPrivate::HitReactSourceId,
				AOCharacterCombatManagerComponentPrivate::MakeHitReactTags(StateTag));
		}
		else
		{
			PersistentStateTagComponent->ClearTagsBySource(AOCharacterCombatManagerComponentPrivate::HitReactSourceId);
		}
	}
}

FGameplayTag UAOCharacterCombatManagerComponent::ResolveHitReactStateTag(const EAOHitReactLevel HitReactLevel) const
{
	switch (HitReactLevel)
	{
	case EAOHitReactLevel::Light:
		return AOStateTags::State_Combat_HitReact_Light;

	case EAOHitReactLevel::Heavy:
		return AOStateTags::State_Combat_HitReact_Heavy;

	default:
		return FGameplayTag();
	}
}

bool UAOCharacterCombatManagerComponent::ShouldRejectHitReactLevel(
	UAbilitySystemComponent* TargetASC,
	const EAOHitReactLevel NewLevel) const
{
	if (TargetASC == nullptr)
	{
		return true;
	}

	if (NewLevel == EAOHitReactLevel::Light
		&& TargetASC->HasMatchingGameplayTag(AOStateTags::State_Combat_HitReact_Heavy))
	{
		return true;
	}

	return false;
}

FVector UAOCharacterCombatManagerComponent::ResolveHitReactSourceDirection(const FAOGameplayEffectContext& EffectContext) const
{
	const FHitResult* HitResult = EffectContext.GetHitResult();
	FVector AttackDirection = FVector::ZeroVector;

	if (EffectContext.GetEffectCauser() != nullptr)
	{
		AttackDirection = (EffectContext.GetEffectCauser()->GetActorForwardVector() * -1.0f).GetSafeNormal2D();
	}

	if (AttackDirection.IsNearlyZero() && HitResult != nullptr)
	{
		AttackDirection = HitResult->ImpactNormal.GetSafeNormal2D();
	}

	return AttackDirection;
}

EAOHitReactLevel UAOCharacterCombatManagerComponent::ResolveHitReactLevel(
	UAbilitySystemComponent* TargetASC,
	const float ResolvedHitReactStrength) const
{
	if (TargetASC == nullptr)
	{
		return EAOHitReactLevel::Ignore;
	}

	const UAOCombatAttributeSet* CombatAttributes = TargetASC->GetSet<UAOCombatAttributeSet>();
	const float HitReactThreshold = CombatAttributes->GetHitReactTotalThreshold();

	if (HitReactThreshold <= 0.0f)
	{
		return EAOHitReactLevel::Ignore;
	}

	if (ResolvedHitReactStrength >= HitReactThreshold * 2.0f)
	{
		return EAOHitReactLevel::Heavy;
	}
	if (ResolvedHitReactStrength >= HitReactThreshold)
	{
		return EAOHitReactLevel::Light;
	}
	return EAOHitReactLevel::Ignore;
}

FVector UAOCharacterCombatManagerComponent::ResolveCombatMagnetSearchForward(const AAOCharacter& OwnerCharacter) const
{
	if (OwnerCharacter.IsPlayerControlled())
	{
		if (const AController* Controller = OwnerCharacter.GetController())
		{
			FVector ControlForward = Controller->GetControlRotation().Vector();
			ControlForward.Z = 0.0f;
			if (!ControlForward.IsNearlyZero())
			{
				return ControlForward.GetSafeNormal();
			}
		}
	}

	FVector ActorForward = OwnerCharacter.GetActorForwardVector();
	ActorForward.Z = 0.0f;
	return ActorForward.GetSafeNormal();
}

AActor* UAOCharacterCombatManagerComponent::FindBestCombatMagnetTarget(
	const AAOCharacter& OwnerCharacter,
	const FAOCombatMagnetWindowConfig& WindowConfig,
	const FVector& SearchForward) const
{
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(EObjectTypeQuery::ObjectTypeQuery3);

	TArray<AActor*> IgnoreActors;
	IgnoreActors.Add(const_cast<AAOCharacter*>(&OwnerCharacter));

	TArray<AActor*> OverlappedActors;
	if (!UKismetSystemLibrary::SphereOverlapActors(
		OwnerCharacter.GetWorld(),
		OwnerCharacter.GetActorLocation(),
		WindowConfig.SearchRadius,
		ObjectTypes,
		AActor::StaticClass(),
		IgnoreActors,
		OverlappedActors))
	{
		return nullptr;
	}

	const float CosThreshold = FMath::Cos(
		FMath::DegreesToRadians(FMath::Clamp(WindowConfig.SearchHalfAngleDegrees, 0.0f, 180.0f)));

	AActor* BestTarget = nullptr;
	float BestDistSq = FMath::Square(WindowConfig.SearchRadius);

	for (AActor* CandidateActor : OverlappedActors)
	{
		if (CandidateActor == nullptr)
		{
			continue;
		}

		if (!CanResolveCombatBetweenActors(&OwnerCharacter, CandidateActor))
		{
			continue;
		}

		if (!IsCombatMagnetTargetAlive(*CandidateActor))
		{
			continue;
		}

		FVector ToCandidate = CandidateActor->GetActorLocation() - OwnerCharacter.GetActorLocation();
		ToCandidate.Z = 0.0f;
		if (ToCandidate.IsNearlyZero())
		{
			continue;
		}

		const float DistSq = ToCandidate.SizeSquared();
		const FVector ToCandidateDirection = ToCandidate.GetSafeNormal();
		if (FVector::DotProduct(SearchForward, ToCandidateDirection) < CosThreshold)
		{
			continue;
		}

		if (DistSq < BestDistSq)
		{
			BestDistSq = DistSq;
			BestTarget = CandidateActor;
		}
	}

	return BestTarget;
}

bool UAOCharacterCombatManagerComponent::IsCombatMagnetTargetAlive(const AActor& CandidateActor) const
{
	const AAOCharacter* CandidateCharacter = Cast<AAOCharacter>(&CandidateActor);
	if (CandidateCharacter == nullptr)
	{
		return true;
	}

	const UAbilitySystemComponent* AbilitySystemComponent = CandidateCharacter->GetAbilitySystemComponent();
	if (AbilitySystemComponent == nullptr)
	{
		return true;
	}

	return AbilitySystemComponent->GetNumericAttribute(UAOHealthAttributeSet::GetHealthAttribute()) > KINDA_SMALL_NUMBER;
}

void UAOCharacterCombatManagerComponent::DrawCombatMagnetDebug(
	const AAOCharacter& OwnerCharacter,
	const FAOCombatMagnetWindowConfig& WindowConfig,
	const FVector& SearchForward,
	const AActor* SelectedTarget) const
{
	UWorld* World = OwnerCharacter.GetWorld();
	if (World == nullptr)
	{
		return;
	}

	const FVector Origin = OwnerCharacter.GetActorLocation();
	DrawDebugSphere(
		World,
		Origin,
		WindowConfig.SearchRadius,
		24,
		FColor::Yellow,
		false,
		AOCharacterCombatManagerComponentPrivate::CombatMagnetDebugDuration);
	DrawDebugLine(
		World,
		Origin,
		Origin + SearchForward * WindowConfig.SearchRadius,
		FColor::Cyan,
		false,
		AOCharacterCombatManagerComponentPrivate::CombatMagnetDebugDuration,
		0,
		2.0f);

	if (SelectedTarget != nullptr)
	{
		DrawDebugLine(
			World,
			Origin,
			SelectedTarget->GetActorLocation(),
			FColor::Green,
			false,
			AOCharacterCombatManagerComponentPrivate::CombatMagnetDebugDuration,
			0,
			2.0f);
		DrawDebugSphere(
			World,
			SelectedTarget->GetActorLocation(),
			24.0f,
			12,
			FColor::Green,
			false,
			AOCharacterCombatManagerComponentPrivate::CombatMagnetDebugDuration);
	}
}


