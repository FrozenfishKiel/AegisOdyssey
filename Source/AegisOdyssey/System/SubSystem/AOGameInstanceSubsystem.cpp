// Fill out your copyright notice in the Description page of Project Settings.


#include "AOGameInstanceSubsystem.h"

#include "AegisOdyssey/UI/ViewModel/MVVM_Escape.h"
#include "Types/MVVMViewModelContext.h"

void UAOGameInstanceSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UAOGameInstanceSubsystem::Deinitialize()
{
	Super::Deinitialize();

	if (EscapeViewModel)
	{
		EscapeViewModel->MarkAsGarbage();
		EscapeViewModel = nullptr;
	}
}

void UAOGameInstanceSubsystem::InitializeViewModel()
{
	EscapeViewModel = NewObject<UMVVM_Escape>(this);
}
