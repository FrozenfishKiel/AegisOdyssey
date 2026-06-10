#pragma once

#include "CoreMinimal.h"
#include "AOBotInterface.generated.h"

class UAOPawnData;
UINTERFACE()
class UAOBotInterface : public UInterface
{
	GENERATED_BODY()
public:
	
};
class IAOBotInterface
{
	GENERATED_BODY()
public:
	virtual const UAOPawnData* GetPawnData() const = 0;
};