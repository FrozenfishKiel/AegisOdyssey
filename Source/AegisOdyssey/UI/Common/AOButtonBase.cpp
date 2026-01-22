// Fill out your copyright notice in the Description page of Project Settings.


#include "AOButtonBase.h"
#include "CommonActionWidget.h"

void UAOButtonBase::NativePreConstruct()
{
	Super::NativePreConstruct();
	UpdateButtonStyle();
	RefreshButtonText();
}

//输入控件更新(InputAction)
void UAOButtonBase::UpdateInputActionWidget()
{
	Super::UpdateInputActionWidget();
	UpdateButtonStyle();
	RefreshButtonText();
}

//输入设备切换时相应
void UAOButtonBase::OnInputMethodChanged(ECommonInputType CurrentInputType)
{
	Super::OnInputMethodChanged(CurrentInputType);

	UpdateButtonStyle();

}

void UAOButtonBase::RefreshButtonText()
{
	if (bOverride_ButtonText || ButtonText.IsEmpty())
	{
		if (InputActionWidget)
		{
			const FText ActionDisplayText = InputActionWidget->GetDisplayText();	
			if (!ActionDisplayText.IsEmpty())
			{
				UpdateButtonText(ActionDisplayText);
				return;
			}
		}
	}
	
	UpdateButtonText(ButtonText);	
}
void UAOButtonBase::SetButtonText(const FText& InText)
{
	bOverride_ButtonText = InText.IsEmpty();
	ButtonText = InText;
	RefreshButtonText();
}