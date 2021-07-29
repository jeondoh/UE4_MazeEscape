// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class MazeEscape : ModuleRules
{
	public MazeEscape(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay", "UMG" });
	}
}
