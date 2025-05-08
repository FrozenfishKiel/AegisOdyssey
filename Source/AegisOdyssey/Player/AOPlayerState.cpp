// Fill out your copyright notice in the Description page of Project Settings.


#include "AOPlayerState.h"

AAOPlayerController* AAOPlayerState::GetAOPlayerController() const
{
	return Cast<AAOPlayerController>(GetOwner());
}
