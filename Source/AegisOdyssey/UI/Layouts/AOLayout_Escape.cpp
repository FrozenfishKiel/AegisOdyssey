// Fill out your copyright notice in the Description page of Project Settings.


#include "AOLayout_Escape.h"
#include "Input/CommonUIInputTypes.h"
#include "AegisOdyssey/AOGameplayTags.h"
#include "AegisOdyssey/System/SubSystem/AOGameInstanceSubsystem.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(AOLayout_Escape)

void UAOLayout_Escape::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	RegisterUIActionBinding(FBindUIActionArgs(FUIActionTag::ConvertChecked(AOGameplayTags::UI_ACTION_ESCAPE), false, FSimpleDelegate::CreateUObject(this, &ThisClass::HandleEscapeAction)));

}

void UAOLayout_Escape::HandleEscapeAction()
{
	DeactivateWidget();
}

UMVVM_Escape* UAOLayout_Escape::GetEscapeViewModel() const
{
	UWorld* World = GetWorld();

	if (UGameInstance* GI = World->GetGameInstance())
	{
		if (UAOGameInstanceSubsystem* SubSystem = GI->GetSubsystem<UAOGameInstanceSubsystem>())
		{
			return SubSystem->GetEscapeViewModel();
		}
	}
	return nullptr;
}
