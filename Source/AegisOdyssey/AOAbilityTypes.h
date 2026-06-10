#pragma once

#include "GameplayEffectTypes.h"
#include "GameplayTagContainer.h"
#include "AOAbilityTypes.generated.h"

class UAOAttackEffectProfile;

USTRUCT(BlueprintType)
struct FAOGameplayEffectContext : public FGameplayEffectContext
{
	GENERATED_BODY()

public:
	bool GetIsCritical() const { return bIsCritical; }
	void SetIsCritical(bool NewIsCritical) { bIsCritical = NewIsCritical; }

	bool GetWasBlocked() const { return bWasBlocked; }
	void SetWasBlocked(bool bNewWasBlocked) { bWasBlocked = bNewWasBlocked; }

	bool GetWasParried() const { return bWasParried; }
	void SetWasParried(bool bNewWasParried) { bWasParried = bNewWasParried; }

	bool GetHitInvulnerability() const { return bHitInvulnerability; }
	void SetHitInvulnerability(bool bNewHitInvulnerability) { bHitInvulnerability = bNewHitInvulnerability; }

	bool GetTargetBroken() const { return bTargetBroken; }
	void SetTargetBroken(bool bNewTargetBroken) { bTargetBroken = bNewTargetBroken; }

	float GetResolvedHitStaminaDamage() const { return ResolvedHitStaminaDamage; }
	void SetResolvedHitStaminaDamage(float NewResolvedHitStaminaDamage) { ResolvedHitStaminaDamage = NewResolvedHitStaminaDamage; }

	float GetResolvedHitReactStrength() const { return ResolvedHitReactStrength; }
	void SetResolvedHitReactStrength(float NewResolvedHitReactStrength) { ResolvedHitReactStrength = NewResolvedHitReactStrength; }

	const UAOAttackEffectProfile* GetAttackEffectProfile() const { return AttackEffectProfile; }
	void SetAttackEffectProfile(UAOAttackEffectProfile* NewAttackEffectProfile) { AttackEffectProfile = NewAttackEffectProfile; }

	const FGameplayTag& GetAttackTag() const { return AttackTag; }
	void SetAttackTag(const FGameplayTag& NewAttackTag) { AttackTag = NewAttackTag; }

	const FGameplayTag& GetSkillTag() const { return SkillTag; }
	void SetSkillTag(const FGameplayTag& NewSkillTag) { SkillTag = NewSkillTag; }

	const FGameplayTag& GetWeaponTag() const { return WeaponTag; }
	void SetWeaponTag(const FGameplayTag& NewWeaponTag) { WeaponTag = NewWeaponTag; }

	const FGameplayTagContainer& GetDamageTypeTags() const { return DamageTypeTags; }
	void SetDamageTypeTags(const FGameplayTagContainer& NewDamageTypeTags) { DamageTypeTags = NewDamageTypeTags; }

public:
	virtual UScriptStruct* GetScriptStruct() const override { return StaticStruct(); }
	virtual bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess) override;

	virtual FAOGameplayEffectContext* Duplicate() const override
	{
		FAOGameplayEffectContext* NewContext = new FAOGameplayEffectContext();
		*NewContext = *this;
		if (GetHitResult())
		{
			NewContext->AddHitResult(*GetHitResult(), true);
		}
		return NewContext;
	}

protected:
	bool bIsCritical = false;
	bool bWasBlocked = false;
	bool bWasParried = false;
	bool bHitInvulnerability = false;
	bool bTargetBroken = false;
	float ResolvedHitStaminaDamage = 0.0f;
	float ResolvedHitReactStrength = 0.0f;
	TObjectPtr<UAOAttackEffectProfile> AttackEffectProfile = nullptr;

	FGameplayTag AttackTag;
	FGameplayTag SkillTag;
	FGameplayTag WeaponTag;
	FGameplayTagContainer DamageTypeTags;
};

template<>
struct TStructOpsTypeTraits<FAOGameplayEffectContext> : public TStructOpsTypeTraitsBase2<FAOGameplayEffectContext>
{
	enum
	{
		WithNetSerializer = true,
		WithCopy = true
	};
};
