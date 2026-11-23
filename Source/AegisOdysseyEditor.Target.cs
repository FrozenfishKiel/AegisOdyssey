// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;
using System.Collections.Generic;

public class AegisOdysseyEditorTarget : TargetRules
{
	// 注册编辑器目标和主模块，并使用已安装引擎的构建规则。
	public AegisOdysseyEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		// 编辑器目标与 UnrealEditor 共享构建产物，构建设置和头文件顺序按 UE 5.8 固定。
		DefaultBuildSettings = BuildSettingsVersion.V7;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_8;

		ExtraModuleNames.AddRange( new string[] { "AegisOdyssey" } );
	}
}
