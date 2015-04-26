// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/PlayerController.h"
#include "LylatDragoonPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class ALylatDragoonPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:

	ALylatDragoonPlayerController(const FObjectInitializer& ObjectInitializer);
	
	bool SnapToViewFrustum(const FVector& worldPosition, FVector* outSnapped);

	bool IsOutOfFrustumXLeft(const FVector& worldPosition, float threshold = 0.1f);
	bool IsOutOfFrustumXRight(const FVector& worldPosition, float threshold = 0.1f);
	bool IsOutOfFrustumYUp(const FVector& worldPosition, float threshold = 0.1f);
	bool IsOutOfFrustumYDown(const FVector& worldPosition, float threshold = 0.1f);

protected:

	FVector ProjectPointInCameraFrustum(const FVector& worldPosition);
};
