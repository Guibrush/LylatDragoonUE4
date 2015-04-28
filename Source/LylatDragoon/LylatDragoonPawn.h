// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "GameFramework/Pawn.h"
#include "LylatDragoonPawn.generated.h"

UCLASS(config=Game)
class ALylatDragoonPawn : public APawn
{
	GENERATED_BODY()

	/** StaticMesh component that will be the visuals for our flying pawn */
	UPROPERTY(Category = Mesh, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* PlaneMesh;

	/** Spring arm that will offset the camera */
	UPROPERTY(Category = Camera, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* SpringArm;

	/** Camera component that will be our viewpoint */
	UPROPERTY(Category = Camera, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* Camera;

public:
	ALylatDragoonPawn(const FObjectInitializer& ObjectInitializer);

	// Begin AActor overrides
	virtual void PostInitializeComponents() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void ReceiveHit(class UPrimitiveComponent* MyComp, class AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit) override;
	// End AActor overrides

	FVector GetAimPointLocation();

protected:

	// Begin APawn overrides
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override; // Allows binding actions/axes to functions
	// End APawn overrides

	/** Bound to the thrust axis */
	void ThrustInput(float Val);
	
	/** Bound to the vertical axis */
	void MoveUpInput(float Val);

	/** Bound to the horizontal axis */
	void MoveRightInput(float Val);

private:

	/** How quickly forward speed changes */
	UPROPERTY(Category = Plane, EditAnywhere)
	float Acceleration;

	/** How quickly pawn can move */
	UPROPERTY(Category = Movement, EditAnywhere)
	float PlayerMovementSpeed;

	/** How quickly camera can move */
	UPROPERTY(Category = Movement, EditAnywhere)
	float CameraMovementSpeed;

	/** How quickly camera can rotate when player moves */
	UPROPERTY(Category = Movement, EditAnywhere)
	float CameraRotationSpeed;

	/** How quickly camera recover its neutral orientation */
	UPROPERTY(Category = Movement, EditAnywhere)
	float CameraRecoverySpeed;

	/** How quickly aim point can move */
	UPROPERTY(Category = Movement, EditAnywhere)
	float AimPointMovementSpeed;

	/** How quickly aim point recover for itself */
	UPROPERTY(Category = Movement, EditAnywhere)
	float AimPointRecoverySpeed;

	/** How far from the centre of the screen aim point has to recover */
	UPROPERTY(Category = Movement, EditAnywhere)
	float AimPointRecoveryTolerance;

	/** Max forward speed */
	UPROPERTY(Category = Pitch, EditAnywhere)
	float MaxSpeed;

	/** Min forward speed */
	UPROPERTY(Category = Yaw, EditAnywhere)
	float MinSpeed;

	/** Distance from level course to the aim point */
	UPROPERTY(Category = Movement, EditAnywhere)
	float AimPointDistance;

	/** Current forward speed */
	float CurrentForwardSpeed;

	/** Current yaw speed */
	float CurrentYawSpeed;

	/** Current pitch speed */
	float CurrentPitchSpeed;

	/** Current roll speed */
	float CurrentRollSpeed;

	/** Level course location in the previous frame */
	FVector PreviousLevelCourseLocation;

	/** Player input. Its an offset from level course object */
	FVector PlayerInputLocation;

	/** Camera input. Its an offset from camera's target*/
	FVector CameraInputLocation;

	/** Aim point input. Its an offset from fixed aim point */
	FVector AimPointInputLocation;

	/** Movement direction from level course */
	FVector MovementDirection;

	/** Location of the aim point */
	FVector AimPointLocation;

	/** Location of the player */
	FVector PlayerLocation;

	bool MovementInputPressed;

	bool RightInputPressed;
	bool LeftInputPressed;
	bool UpInputPressed;
	bool DownInputPressed;

	/** Object to follow level course */
	class ALylatDragoonLevelCourse* LevelCourse;

public:
	/** Returns PlaneMesh subobject **/
	FORCEINLINE class UStaticMeshComponent* GetPlaneMesh() const { return PlaneMesh; }
	/** Returns SpringArm subobject **/
	FORCEINLINE class USpringArmComponent* GetSpringArm() const { return SpringArm; }
	/** Returns Camera subobject **/
	FORCEINLINE class UCameraComponent* GetCamera() const { return Camera; }
};
