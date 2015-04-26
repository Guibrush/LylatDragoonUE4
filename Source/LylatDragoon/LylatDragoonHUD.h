// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/HUD.h"
#include "LylatDragoonHUD.generated.h"

/**
 * 
 */
UCLASS()
class ALylatDragoonHUD : public AHUD
{
	GENERATED_BODY()

public:

	ALylatDragoonHUD(const FObjectInitializer& ObjectInitializer);

	// Begin AHUD overrides
	virtual void DrawHUD() override;
	// End AHUD overrides

	UPROPERTY(Category = LDHUD, EditAnywhere)
	class UTexture* CrosshairTexture;

	UPROPERTY(Category = LDHUD, EditAnywhere)
	float CrosshairSize;

};
