// Fill out your copyright notice in the Description page of Project Settings.


#include "AOWorldSettings.h"

#include "Engine/AssetManager.h"

AAOWorldSettings::AAOWorldSettings(const FObjectInitializer& ObjectInitializer)
{
}

FPrimaryAssetId AAOWorldSettings::GetDefaultGameplayExperience() const
{
	FPrimaryAssetId Result;

	if (!DefaultGameplayDefinition.IsNull())
	{
		Result = UAssetManager::Get().GetPrimaryAssetIdForPath(DefaultGameplayDefinition.ToSoftObjectPath());

		if (!Result.IsValid())
		{
			
		}
	}
	return Result;
}

TSubclassOf<UAOExperienceDefinition> AAOWorldSettings::GetFeatureDefinitionClass() const
{
	TSubclassOf<UAOExperienceDefinition> Result;
	if (!DefaultGameplayDefinition.IsNull())
	{
		Result = DefaultGameplayDefinition.LoadSynchronous();
		if (!Result) return nullptr;
	}
	return Result;
}
