// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class AegisOdyssey : ModuleRules
{
	public AegisOdyssey(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core", "CoreUObject", "Engine", "InputCore","ModularGameplay" , "GameplayTags","GameplayAbilities","GameplayTasks" , "ModularGameplayActors" , "GameFeatures",
			"EnhancedInput"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { "ModularGameplay","UMG" , "GameFeatures" });

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
