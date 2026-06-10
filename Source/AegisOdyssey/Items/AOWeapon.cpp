// Fill out your copyright notice in the Description page of Project Settings.


#include "AOWeapon.h"

void AAOWeapon::InitializeActorSpawnConfig()
{
	// 装备到角色身上后，武器世界物只保留表现，不再参与拾取或展示碰撞。
	DisableEquippedPresentationCollision();
}
