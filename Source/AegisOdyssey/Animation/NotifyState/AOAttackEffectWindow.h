#pragma once

#include "CoreMinimal.h"
#include "AOGiveOwnerTagWindow.h"
#include "AOAttackEffectWindow.generated.h"

/**
 * 武器判定窗口。
 * 这层继续只承担 GiveOwnerTagWindow 的标签语义，不再负责刀光显示时长。
 */
UCLASS(DisplayName = "AttackEffectWindow")
class AEGISODYSSEY_API UAOAttackEffectWindow : public UAOGiveOwnerTagWindow
{
	GENERATED_BODY()
};
