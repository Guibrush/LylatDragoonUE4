// Fill out your copyright notice in the Description page of Project Settings.

#include "LylatDragoon.h"
#include "LylatDragoonHUD.h"
#include "LylatDragoonPawn.h"

ALylatDragoonHUD::ALylatDragoonHUD(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	CrosshairSize = 1.0f;
}

void ALylatDragoonHUD::DrawHUD()
{
	Super::DrawHUD();

	ALylatDragoonPawn* LDPawn = Cast<ALylatDragoonPawn>(PlayerOwner->GetPawn());
	if (LDPawn)
	{
		FVector CrosshairLocation = Project(LDPawn->GetAimPointLocation());

		DrawTextureSimple(CrosshairTexture, CrosshairLocation.X - ((CrosshairTexture->GetSurfaceWidth() * CrosshairSize) / 2), CrosshairLocation.Y - ((CrosshairTexture->GetSurfaceHeight() * CrosshairSize) / 2), CrosshairSize);
	}
}


