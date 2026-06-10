// Fill out your copyright notice in the Description page of Project Settings.


#include "AOHarvestTool.h"

void AAOHarvestTool::InitializeActorSpawnConfig()
{
	// 装备到角色身上后，工具世界物只保留表现，不再参与拾取或展示碰撞。
	DisableEquippedPresentationCollision();
}
