// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"

/**
 * 
 */
namespace AOGameplayTags
{
	/*AO Initialize State Tags*/
	AEGISODYSSEY_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InitState_Spawned);
	AEGISODYSSEY_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InitState_DataAvailable);
	AEGISODYSSEY_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InitState_DataInitialized);
	AEGISODYSSEY_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InitState_GameplayReady);
	/*Initialize State End*/

	AEGISODYSSEY_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Move);
	AEGISODYSSEY_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_LookUp);
}
