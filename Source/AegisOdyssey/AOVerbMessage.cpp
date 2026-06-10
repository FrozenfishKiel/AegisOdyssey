// Fill out your copyright notice in the Description page of Project Settings.


#include "AOVerbMessage.h"

FString FAOVerbMessage::ToString() const
{
	return FString::Printf(
		TEXT("%s Instigator=%s Target=%s Magnitude=%.2f"),
		*Verb.ToString(),
		*GetNameSafe(Instigator),
		*GetNameSafe(Target),
		Magnitude);
}
