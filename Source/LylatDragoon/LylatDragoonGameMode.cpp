// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "LylatDragoon.h"
#include "LylatDragoonGameMode.h"
#include "LylatDragoonPawn.h"

ALylatDragoonGameMode::ALylatDragoonGameMode(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// set default pawn class to our flying pawn
	DefaultPawnClass = ALylatDragoonPawn::StaticClass();
}
