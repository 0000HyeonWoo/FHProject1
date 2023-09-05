// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class FHProject : ModuleRules
{
	public FHProject(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] 
        { "Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay", "EnhancedInput",
           "UMG", "Weapon" 
        });

        PublicIncludePaths.AddRange(new string[] { "FHProject", "FHProject/Public" });

        PrivateIncludePaths.AddRange(new string[] { "FHProject/Private" });

        DynamicallyLoadedModuleNames.Add("OnlineSubsystemNull");

    }
}
