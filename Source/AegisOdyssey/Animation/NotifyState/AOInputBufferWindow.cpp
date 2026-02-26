// Fill out your copyright notice in the Description page of Project Settings.


#include "AOInputBufferWindow.h"

#include "AegisOdyssey/Character/AOCharacter.h"
#include "AegisOdyssey/Character/AOInputBufferComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOInputBufferWindow)

UAOInputBufferWindow::UAOInputBufferWindow()
{
	
}

void UAOInputBufferWindow::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	
	AActor* OwnerActor = MeshComp->GetOwner();
	if (!OwnerActor)
	{
		return;
	}

	AAOCharacter* AOCharacter = Cast<AAOCharacter>(OwnerActor);
	if (!AOCharacter)
	{
		return;
	}

	UAOInputBufferComponent* BufferComponent = AOCharacter->GetComponentByClass<UAOInputBufferComponent>();  //获取预输入窗口组件
	if (!BufferComponent) return;
	BufferComponent->TriggerBufferedInput();  //直接调用预输入储存的输入
}

void UAOInputBufferWindow::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
}
