#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "AOAttackEffectProfile.generated.h"

class AActor;
class UAbilitySystemComponent;
struct FAOGameplayEffectContext;
struct FGameplayCueParameters;

UENUM(BlueprintType)
enum class EAOAttackEffectTrigger : uint8
{
	CombatWindowBegin UMETA(DisplayName = "Combat Window Begin"),
	CombatWindowEnd UMETA(DisplayName = "Combat Window End"),
	HitConfirmed UMETA(DisplayName = "Hit Confirmed")
};

USTRUCT(BlueprintType)
struct FAOAttackEffectEntry
{
	GENERATED_BODY()

	// 触发语义只描述“什么时候播”，不关心具体表现资产细节。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AttackEffect")
	EAOAttackEffectTrigger Trigger = EAOAttackEffectTrigger::HitConfirmed;

	// 一条触发语义允许挂多个 GameplayCue，具体怎么播由 Cue 自己组织。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AttackEffect", meta = (Categories = "GameplayCue"))
	TArray<FGameplayTag> CueTags;
};

UCLASS(BlueprintType, Const)
class AEGISODYSSEY_API UAOAttackEffectProfile : public UDataAsset
{
	GENERATED_BODY()

public:
	const TArray<FAOAttackEffectEntry>& GetEntries() const { return Entries; }

private:
	// 第一版这里只承载“触发什么 GameplayCue”的编排，不下沉到底层特效/音效资源细节。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AttackEffect", meta = (AllowPrivateAccess = true))
	TArray<FAOAttackEffectEntry> Entries;
};

// 运行时只负责把既有战斗语义桥接到攻击表现 Profile，不在这里扩展新的战斗判定逻辑。
class AEGISODYSSEY_API FAOAttackEffectProfileRuntime
{
public:
	static const UAOAttackEffectProfile* ResolveProfileFromSourceObject(const UObject* SourceObject);
	static const UAOAttackEffectProfile* ResolveProfileFromActor(const AActor* Actor);
	static const UAOAttackEffectProfile* ResolveProfileFromEffectContext(const FAOGameplayEffectContext& EffectContext);
	static void DispatchTrigger(
		const UAOAttackEffectProfile* Profile,
		EAOAttackEffectTrigger Trigger,
		UAbilitySystemComponent* AbilitySystemComponent,
		const FGameplayCueParameters& CueParameters);
};
