// Fill out your copyright notice in the Description page of Project Settings.


#include "AOPublicFunctionLibrary.h"

#include "Character/AOCharacter.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOPublicFunctionLibrary)

int32 UAOPublicFunctionLibrary::GetPlayerLevel(UObject* ContextObject)
{
	if (AAOCharacter* TargetCharacter = Cast<AAOCharacter>(ContextObject))
	{
		return TargetCharacter->GetCharacterLevel();
	}
	return 0;
}
