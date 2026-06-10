// Fill out your copyright notice in the Description page of Project Settings.


#include "AOEnemyBotController.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(AOEnemyBotController)


// Sets default values
AAOEnemyBotController::AAOEnemyBotController()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

void AAOEnemyBotController::SetSenseResultActor_Implementation(AActor* SenseResultActor)
{
	FocusActor = SenseResultActor;
	SetCurrentTarget(SenseResultActor);
}

// Called when the game starts or when spawned
void AAOEnemyBotController::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AAOEnemyBotController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
