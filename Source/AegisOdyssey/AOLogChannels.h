#pragma once

class UObject;

AEGISODYSSEY_API DECLARE_LOG_CATEGORY_EXTERN(LogAegisOdyssey, Log, All);
AEGISODYSSEY_API DECLARE_LOG_CATEGORY_EXTERN(LogAegisOdysseyExperience, Log, All);
AEGISODYSSEY_API DECLARE_LOG_CATEGORY_EXTERN(LogAegisOdysseyAbilitySystem, Log, All);
AEGISODYSSEY_API DECLARE_LOG_CATEGORY_EXTERN(LogAegisOdysseyCombatTrace, Log, All);
AEGISODYSSEY_API DECLARE_LOG_CATEGORY_EXTERN(LogAegisOdysseyPlayer, Log, All);
AEGISODYSSEY_API DECLARE_LOG_CATEGORY_EXTERN(LogAegisOdysseyInventory, Log, All);
AEGISODYSSEY_API DECLARE_LOG_CATEGORY_EXTERN(LogAegisOdysseyAttributeSet, Log, All);


AEGISODYSSEY_API FString GetClientServerContextString(UObject* ContextObject = nullptr);


