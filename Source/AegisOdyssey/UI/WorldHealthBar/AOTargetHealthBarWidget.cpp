#include "AOTargetHealthBarWidget.h"

#include "AegisOdyssey/UI/ViewModel/MVVM_TargetHealthBar.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOTargetHealthBarWidget)

void UAOTargetHealthBarWidget::SetTargetHealthBarViewModel(UMVVMTargetHealthBar* InViewModel)
{
	if (TargetHealthBarViewModel == InViewModel)
	{
		return;
	}

	TargetHealthBarViewModel = InViewModel;
	OnTargetHealthBarViewModelSet(TargetHealthBarViewModel);
}
