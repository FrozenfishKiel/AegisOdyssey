// Copyright Epic Games, Inc. All Rights Reserved.


#include "AOAssetManagerStartupJob.h"
#include "AegisOdyssey/AOLogChannels.h"

TSharedPtr<FStreamableHandle> FAOAssetManagerStartupJob::DoJob() const
{
	const double JobStartTime = FPlatformTime::Seconds();

	TSharedPtr<FStreamableHandle> Handle;
	UE_LOG(LogAegisOdyssey, Display, TEXT("Startup job \"%s\" starting"), *JobName);  //打印当前资源的工作状态
	JobFunc(*this,Handle);  //构造当前的函数，并初始化指针和参数

	if (Handle.IsValid())
	{
		Handle->BindUpdateDelegate(FStreamableUpdateDelegate::CreateRaw(this,&FAOAssetManagerStartupJob::UpdateSubstepProgressFromStreamable));
		Handle->WaitUntilComplete(0.0f,false);
		Handle->BindUpdateDelegate(FStreamableUpdateDelegate());
	}
	
	UE_LOG(LogAegisOdyssey, Display, TEXT("Startup job \"%s\" took %.2f seconds to complete"), *JobName, FPlatformTime::Seconds() - JobStartTime);

	return Handle;
}
