// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class LylatDragoon : ModuleRules
{
	public LylatDragoon(ReadOnlyTargetRules ROTargetRules) : base (ROTargetRules)
	{
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "LevelSequence", "MovieScene" });
	}
}
