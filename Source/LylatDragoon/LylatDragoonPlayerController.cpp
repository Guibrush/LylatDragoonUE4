// Fill out your copyright notice in the Description page of Project Settings.

#include "LylatDragoon.h"
#include "LylatDragoonPlayerController.h"

#include "Engine/LocalPlayer.h"

ALylatDragoonPlayerController::ALylatDragoonPlayerController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

bool ALylatDragoonPlayerController::SnapToViewFrustum(const FVector& worldPosition, FVector* outSnapped)
{
	// SceneView initialization gently adapted from APlayerController::ProjectWorldLocationToScreen.
	ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(Player);
	if (LocalPlayer != NULL && LocalPlayer->ViewportClient != NULL && LocalPlayer->ViewportClient->Viewport != NULL)
	{
		// This sceneview initialization should probably be moved into another function.
		// Create a view family for the game viewport
		FSceneViewFamilyContext ViewFamily(FSceneViewFamily::ConstructionValues(
			LocalPlayer->ViewportClient->Viewport,
			GetWorld()->Scene,
			LocalPlayer->ViewportClient->EngineShowFlags)
			.SetRealtimeUpdate(true));
		FVector ViewLocation;
		FRotator ViewRotation;
		FSceneView* SceneView = LocalPlayer->CalcSceneView(&ViewFamily, /*out*/ ViewLocation, /*out*/ ViewRotation, LocalPlayer->ViewportClient->Viewport);

		if (SceneView)
		{
			// Transform our world position into projection coordinates.
			auto ProjPosAug = SceneView->ViewProjectionMatrix.TransformPosition(worldPosition);
			FVector ProjPos(ProjPosAug);
			// Divide by the augmented coord W value.
			ProjPos /= ProjPosAug.W;
			// Clamp position to -1,1 on x and y (corresponds to on-screen)
			ProjPos.X = FMath::Clamp<float>(ProjPos.X, -1, 1);
			ProjPos.Y = FMath::Clamp<float>(ProjPos.Y, -1, 1);
			// Invert the transform back to world space.
			auto AugWorldSnapped = SceneView->InvViewProjectionMatrix.TransformPosition(ProjPos);
			FVector SnapPos(AugWorldSnapped);
			SnapPos /= AugWorldSnapped.W;
			*outSnapped = SnapPos;
			return true;
		}
	}
	return false;
}

bool ALylatDragoonPlayerController::IsOutOfFrustumXLeft(const FVector& worldPosition, float threshold)
{
	FVector ProjPos(ProjectPointInCameraFrustum(worldPosition));
	return ProjPos.X < (-1 + threshold);
}

bool ALylatDragoonPlayerController::IsOutOfFrustumXRight(const FVector& worldPosition, float threshold)
{
	FVector ProjPos(ProjectPointInCameraFrustum(worldPosition));
	return ProjPos.X > (1 - threshold);
}

bool ALylatDragoonPlayerController::IsOutOfFrustumYUp(const FVector& worldPosition, float threshold)
{
	FVector ProjPos(ProjectPointInCameraFrustum(worldPosition));
	return ProjPos.Y < (-1 + threshold);
}

bool ALylatDragoonPlayerController::IsOutOfFrustumYDown(const FVector& worldPosition, float threshold)
{
	FVector ProjPos(ProjectPointInCameraFrustum(worldPosition));
	return ProjPos.Y > (1 - threshold);
}

FVector ALylatDragoonPlayerController::ProjectPointInCameraFrustum(const FVector& worldPosition)
{
	// SceneView initialization gently adapted from APlayerController::ProjectWorldLocationToScreen.
	ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(Player);
	if (LocalPlayer != NULL && LocalPlayer->ViewportClient != NULL && LocalPlayer->ViewportClient->Viewport != NULL)
	{
		// This sceneview initialization should probably be moved into another function.
		// Create a view family for the game viewport
		FSceneViewFamilyContext ViewFamily(FSceneViewFamily::ConstructionValues(
			LocalPlayer->ViewportClient->Viewport,
			GetWorld()->Scene,
			LocalPlayer->ViewportClient->EngineShowFlags)
			.SetRealtimeUpdate(true));
		FVector ViewLocation;
		FRotator ViewRotation;
		FSceneView* SceneView = LocalPlayer->CalcSceneView(&ViewFamily, /*out*/ ViewLocation, /*out*/ ViewRotation, LocalPlayer->ViewportClient->Viewport);

		if (SceneView)
		{
			// Transform our world position into projection coordinates.
			auto ProjPosAug = SceneView->ViewProjectionMatrix.TransformPosition(worldPosition);
			FVector ProjPos(ProjPosAug);
			// Divide by the augmented coord W value.
			ProjPos /= ProjPosAug.W;
			return ProjPos;
		}
	}

	return FVector::ZeroVector;
}
