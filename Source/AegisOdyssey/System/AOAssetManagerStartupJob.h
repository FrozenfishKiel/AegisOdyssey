#pragma once

#include "Engine/StreamableManager.h"

DECLARE_DELEGATE_OneParam(FAOAssetManagerStartUpJobSubstepProgress,float /*NewProgress*/)

//这个主要是处理资源在加载的时候的通知时间和进度数据
struct FAOAssetManagerStartupJob
{
	FAOAssetManagerStartUpJobSubstepProgress SubStepProgressDelegate;
	TFunction<void(const FAOAssetManagerStartupJob&,TSharedPtr<FStreamableHandle>&)> JobFunc;  //函数模板句柄
	FString JobName;
	float JobWeight; //权重数据，根据权重来预先为资源分配加载时间
	mutable double LasUpdate = 0;  //mutable表示允许const函数修改它

	FAOAssetManagerStartupJob(const FString& InJobName , const TFunction<void(const FAOAssetManagerStartupJob&, TSharedPtr<FStreamableHandle>&)>& InJobFunc,float InWeight)
	: JobFunc(InJobFunc)
	, JobName(InJobName)
	, JobWeight(InWeight)
	{}

	TSharedPtr<FStreamableHandle> DoJob() const;  //执行资源加载，之后会返回一个流式加载的句柄

	void UpdateSubStepProgress(float NewProgress) const
	{
		SubStepProgressDelegate.ExecuteIfBound(NewProgress);
	}

	void UpdateSubstepProgressFromStreamable(TSharedRef<FStreamableHandle> InStreamable) const
	{
		if (SubStepProgressDelegate.IsBound())
		{
			double Now = FPlatformTime::Seconds();
			if (LasUpdate - Now > 1.0/60.0)
			{
				SubStepProgressDelegate.Execute(InStreamable->GetProgress());
				LasUpdate = Now;
			}
		}
	}
};