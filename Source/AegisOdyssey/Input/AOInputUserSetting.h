#pragma once
#include "PlayerMappableKeySettings.h"
#include "UserSettings/EnhancedInputUserSettings.h"
#include "AOInputUserSetting.generated.h"

UCLASS()
class AEGISODYSSEY_API UAOInputUserSetting : public UEnhancedInputUserSettings
{
	GENERATED_BODY()
public:
	virtual void ApplySettings() override;
};



UCLASS()
class AEGISODYSSEY_API UAOPlayerMappableKeySettings : public UPlayerMappableKeySettings
{
	GENERATED_BODY()

public:

	const FText& GetTooltipText() const;

protected:

	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category = "Settings",meta = (AllowPrivateAccess = true))
	FText Tooltip = FText::GetEmpty();
};