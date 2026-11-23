// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;
using System.Collections.Generic;

public class AegisOdysseyTarget : TargetRules
{
	// 注册独立游戏目标和主模块，以便打包时使用同一版引擎规则。
	public AegisOdysseyTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		// 固定 UE 5.8 的构建设置和头文件顺序，便于和已安装引擎的编译产物配套。
		DefaultBuildSettings = BuildSettingsVersion.V7;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_8;

		ExtraModuleNames.AddRange( new string[] { "AegisOdyssey" } );
	}
}
