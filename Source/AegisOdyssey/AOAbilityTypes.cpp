#include "AOAbilityTypes.h"

#include "AegisOdyssey/Combat/Effects/AOAttackEffectProfile.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOAbilityTypes)

bool FAOGameplayEffectContext::NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
{
	uint32 RepBits = 0;
	if (Ar.IsSaving())
	{
		if (bReplicateInstigator && Instigator.IsValid())
		{
			RepBits |= 1 << 0;
		}
		if (bReplicateEffectCauser && EffectCauser.IsValid())
		{
			RepBits |= 1 << 1;
		}
		if (AbilityCDO.IsValid())
		{
			RepBits |= 1 << 2;
		}
		if (bReplicateSourceObject && SourceObject.IsValid())
		{
			RepBits |= 1 << 3;
		}
		if (Actors.Num() > 0)
		{
			RepBits |= 1 << 4;
		}
		if (HitResult.IsValid())
		{
			RepBits |= 1 << 5;
		}
		if (bHasWorldOrigin)
		{
			RepBits |= 1 << 6;
		}
		if (bIsCritical)
		{
			RepBits |= 1 << 7;
		}
		if (bWasBlocked)
		{
			RepBits |= 1 << 8;
		}
		if (bWasParried)
		{
			RepBits |= 1 << 9;
		}
		if (bHitInvulnerability)
		{
			RepBits |= 1 << 10;
		}
		if (bTargetBroken)
		{
			RepBits |= 1 << 11;
		}
		if (AttackTag.IsValid())
		{
			RepBits |= 1 << 14;
		}
		if (SkillTag.IsValid())
		{
			RepBits |= 1 << 15;
		}
		if (WeaponTag.IsValid())
		{
			RepBits |= 1 << 16;
		}
		if (!DamageTypeTags.IsEmpty())
		{
			RepBits |= 1 << 17;
		}
		if (!FMath::IsNearlyZero(ResolvedHitStaminaDamage))
		{
			RepBits |= 1 << 18;
		}
		if (!FMath::IsNearlyZero(ResolvedHitReactStrength))
		{
			RepBits |= 1 << 19;
		}
		if (AttackEffectProfile != nullptr)
		{
			RepBits |= 1 << 20;
		}
	}

	Ar.SerializeBits(&RepBits, 21);

	if (RepBits & (1 << 0))
	{
		Ar << Instigator;
	}
	if (RepBits & (1 << 1))
	{
		Ar << EffectCauser;
	}
	if (RepBits & (1 << 2))
	{
		Ar << AbilityCDO;
	}
	if (RepBits & (1 << 3))
	{
		Ar << SourceObject;
	}
	if (RepBits & (1 << 4))
	{
		SafeNetSerializeTArray_Default<31>(Ar, Actors);
	}
	if (RepBits & (1 << 5))
	{
		if (Ar.IsLoading() && !HitResult.IsValid())
		{
			HitResult = TSharedPtr<FHitResult>(new FHitResult());
		}

		HitResult->NetSerialize(Ar, Map, bOutSuccess);
	}
	if (RepBits & (1 << 6))
	{
		Ar << WorldOrigin;
		bHasWorldOrigin = true;
	}
	else
	{
		bHasWorldOrigin = false;
	}

	bIsCritical = (RepBits & (1 << 7)) != 0;
	bWasBlocked = (RepBits & (1 << 8)) != 0;
	bWasParried = (RepBits & (1 << 9)) != 0;
	bHitInvulnerability = (RepBits & (1 << 10)) != 0;
	bTargetBroken = (RepBits & (1 << 11)) != 0;

	if (RepBits & (1 << 14))
	{
		Ar << AttackTag;
	}
	else if (Ar.IsLoading())
	{
		AttackTag = FGameplayTag();
	}

	if (RepBits & (1 << 15))
	{
		Ar << SkillTag;
	}
	else if (Ar.IsLoading())
	{
		SkillTag = FGameplayTag();
	}

	if (RepBits & (1 << 16))
	{
		Ar << WeaponTag;
	}
	else if (Ar.IsLoading())
	{
		WeaponTag = FGameplayTag();
	}

	if (RepBits & (1 << 17))
	{
		DamageTypeTags.NetSerialize(Ar, Map, bOutSuccess);
	}
	else if (Ar.IsLoading())
	{
		DamageTypeTags.Reset();
	}

	if (RepBits & (1 << 18))
	{
		Ar << ResolvedHitStaminaDamage;
	}
	else if (Ar.IsLoading())
	{
		ResolvedHitStaminaDamage = 0.0f;
	}

	if (RepBits & (1 << 19))
	{
		Ar << ResolvedHitReactStrength;
	}
	else if (Ar.IsLoading())
	{
		ResolvedHitReactStrength = 0.0f;
	}

	if (RepBits & (1 << 20))
	{
		Ar << AttackEffectProfile;
	}
	else if (Ar.IsLoading())
	{
		AttackEffectProfile = nullptr;
	}

	bOutSuccess = true;
	return true;
}
