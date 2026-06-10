// Fill out your copyright notice in the Description page of Project Settings.


#include "AOEnemy.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(AOEnemy)


// Sets default values
AAOEnemy::AAOEnemy()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AAOEnemy::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AAOEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void AAOEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

