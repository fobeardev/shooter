// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class SKGProceduralAnim : ModuleRules
{
	public SKGProceduralAnim(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		IWYUSupport = IWYUSupport.Full;
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"GameplayTags"
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"NetCore"
			}
			);
		
		//SetupIrisSupport(Target);
	}
}
