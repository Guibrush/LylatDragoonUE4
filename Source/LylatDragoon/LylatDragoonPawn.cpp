// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "LylatDragoon.h"
#include "LylatDragoonPawn.h"

#include "LylatDragoonLevelCourse.h"
#include "LylatDragoonPlayerController.h"

#include "EngineUtils.h"
#include "DrawDebugHelpers.h"

ALylatDragoonPawn::ALylatDragoonPawn(const FObjectInitializer& ObjectInitializer) 
	: Super(ObjectInitializer)
{
	// Structure to hold one-time initialization
	struct FConstructorStatics
	{
		ConstructorHelpers::FObjectFinderOptional<UStaticMesh> PlaneMesh;
		FConstructorStatics()
			: PlaneMesh(TEXT("/Game/Flying/Meshes/UFO.UFO"))
		{
		}
	};
	static FConstructorStatics ConstructorStatics;

	// Create static mesh component
	PlaneMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlaneMesh0"));
	PlaneMesh->SetStaticMesh(ConstructorStatics.PlaneMesh.Get());
	RootComponent = PlaneMesh;

	// Create a spring arm component
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm0"));
	SpringArm->AttachTo(RootComponent);
	SpringArm->TargetArmLength = 160.0f; // The camera follows at this distance behind the character	
	SpringArm->SocketOffset = FVector(0.f,0.f,60.f);
	SpringArm->bEnableCameraLag = false;
	SpringArm->CameraLagSpeed = 15.f;

	// Create camera component 
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera0"));
	Camera->AttachTo(SpringArm, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false; // Don't rotate camera with controller

	// Set handling parameters
	Acceleration = 500.f;
	PlayerMovementSpeed = 50.f;
	MaxSpeed = 4000.f;
	MinSpeed = 500.f;
	CurrentForwardSpeed = 500.f;

	PlayerInputLocation = FVector(0.0f, 0.0f, 0.0f);
	PreviousLevelCourseLocation = FVector(0.0f, 0.0f, 0.0f);
}

void ALylatDragoonPawn::Tick(float DeltaSeconds)
{
	ALylatDragoonPlayerController* LylatController = Cast<ALylatDragoonPlayerController>(Controller);
	if (LevelCourse && LylatController)
	{
		// Calculate the movement direction and rotation quaternion
		MovementDirection = LevelCourse->GetActorLocation() - PreviousLevelCourseLocation;
		FQuat RelativeMovementQuat = MovementDirection.Rotation().Quaternion();

		// Calculate the recovery movement of the aim point
		FVector PawnToAimPoint = (AimPointLocation - GetActorLocation());
		if (!MovementDirection.GetSafeNormal().Equals(PawnToAimPoint.GetSafeNormal(), AimPointRecoveryTolerance) && !MovementInputPressed)
		{
			AimPointInputLocation = FMath::VInterpTo(AimPointInputLocation, FVector::ZeroVector, DeltaSeconds, AimPointRecoverySpeed);
		}

		// Calculate the aim point and its position according to player input
		FVector FixedAimPointLocation = LevelCourse->GetActorLocation() + (MovementDirection.GetSafeNormal() * AimPointDistance);
		FVector LocalAimOffset = RelativeMovementQuat.RotateVector(AimPointInputLocation);
		AimPointLocation = FixedAimPointLocation + LocalAimOffset;
		LylatController->SnapToViewFrustum(AimPointLocation, &AimPointLocation);

		// Calculate the ship position according to player input
		PlayerLocation = LevelCourse->GetActorLocation();
		FVector LocalActorOffset = RelativeMovementQuat.RotateVector(PlayerInputLocation);
		PlayerLocation += LocalActorOffset;
		LylatController->SnapToViewFrustum(PlayerLocation, &PlayerLocation);

		// Calculate the ship rotation
		FRotator FinalRotation = PawnToAimPoint.Rotation();
		FinalRotation.Roll = 0.0f;
		
		// Set ship rotation and location
		SetActorLocation(PlayerLocation);
		SetActorRotation(FinalRotation);

		// Set level course rotation
		LevelCourse->SetActorRotation(MovementDirection.Rotation());

		// Set current level course position to use it in next frame
		PreviousLevelCourseLocation = LevelCourse->GetActorLocation();

		// Move camera
		SpringArm->SocketOffset = CameraInputLocation;

		// Add camera rotation according to player movement
		if (MovementInputPressed)
		{
			if (RightInputPressed)
			{
				Camera->SetRelativeRotation(FMath::RInterpTo(Camera->RelativeRotation, FRotator(0.0f, 0.0f, 20.0f), DeltaSeconds, CameraRotationSpeed));
			}
			else if (LeftInputPressed)
			{
				Camera->SetRelativeRotation(FMath::RInterpTo(Camera->RelativeRotation, FRotator(0.0f, 0.0f, -20.0f), DeltaSeconds, CameraRotationSpeed));
			}
		}
		
		// Camera orientation recovery
		Camera->SetRelativeRotation(FMath::RInterpTo(Camera->RelativeRotation, FRotator::ZeroRotator, DeltaSeconds, CameraRecoverySpeed));
	}

	MovementInputPressed = false;

	// Call any parent class Tick implementation
	Super::Tick(DeltaSeconds);
}

void ALylatDragoonPawn::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	for (TActorIterator<ALylatDragoonLevelCourse> LCItr(GetWorld()); LCItr; ++LCItr)
	{
		LevelCourse = *LCItr;
		break;
	}

	if (LevelCourse)
	{
		SetActorLocation(LevelCourse->GetActorLocation());
		SetActorRotation(LevelCourse->GetActorRotation());

		SpringArm->AttachTo(LevelCourse->GetRootComponent());
	}
}

FVector ALylatDragoonPawn::GetAimPointLocation()
{
	return AimPointLocation;
}

void ALylatDragoonPawn::ReceiveHit(class UPrimitiveComponent* MyComp, class AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit)
{
	Super::ReceiveHit(MyComp, Other, OtherComp, bSelfMoved, HitLocation, HitNormal, NormalImpulse, Hit);

	// Set velocity to zero upon collision
	CurrentForwardSpeed = 0.f;
}


void ALylatDragoonPawn::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	check(InputComponent);

	// Bind our control axis' to callback functions
	//InputComponent->BindAxis("Thrust", this, &ALylatDragoonPawn::ThrustInput);
	InputComponent->BindAxis("MoveUp", this, &ALylatDragoonPawn::MoveUpInput);
	InputComponent->BindAxis("MoveRight", this, &ALylatDragoonPawn::MoveRightInput);
}

void ALylatDragoonPawn::ThrustInput(float Val)
{
	// Is there no input?
	bool bHasInput = !FMath::IsNearlyEqual(Val, 0.f);
	// If input is not held down, reduce speed
	float CurrentAcc = bHasInput ? (Val * Acceleration) : (-0.5f * Acceleration);
	// Calculate new speed
	float NewForwardSpeed = CurrentForwardSpeed + (GetWorld()->GetDeltaSeconds() * CurrentAcc);
	// Clamp between MinSpeed and MaxSpeed
	CurrentForwardSpeed = FMath::Clamp(NewForwardSpeed, MinSpeed, MaxSpeed);
}

void ALylatDragoonPawn::MoveUpInput(float Val)
{
	ALylatDragoonPlayerController* LylatController = Cast<ALylatDragoonPlayerController>(Controller);
	if (LylatController)
	{
		if (Val > 0.0f)
		{
			if (!LylatController->IsOutOfFrustumYDown(PlayerLocation))
				PlayerInputLocation.Z += Val*PlayerMovementSpeed;

			if (!LylatController->IsOutOfFrustumYDown(AimPointLocation))
				AimPointInputLocation.Z += Val*AimPointMovementSpeed;

			DownInputPressed = true;
			UpInputPressed = false;
		}
		else
		{
			if (!LylatController->IsOutOfFrustumYUp(PlayerLocation))
				PlayerInputLocation.Z += Val*PlayerMovementSpeed;

			if (!LylatController->IsOutOfFrustumYUp(AimPointLocation))
				AimPointInputLocation.Z += Val*AimPointMovementSpeed;

			DownInputPressed = false;
			UpInputPressed = true;
		}
	}

	if (Val != 0.0f)
		MovementInputPressed = true;
	
	CameraInputLocation.Z += Val*CameraMovementSpeed;
}

void ALylatDragoonPawn::MoveRightInput(float Val)
{
	ALylatDragoonPlayerController* LylatController = Cast<ALylatDragoonPlayerController>(Controller);
	if (LylatController)
	{
		if (Val > 0.0f)
		{
			if (!LylatController->IsOutOfFrustumXRight(PlayerLocation))
				PlayerInputLocation.Y += Val*PlayerMovementSpeed;

			if (!LylatController->IsOutOfFrustumXRight(AimPointLocation))
				AimPointInputLocation.Y += Val*AimPointMovementSpeed;

			RightInputPressed = true;
			LeftInputPressed = false;
		}
		else
		{
			if (!LylatController->IsOutOfFrustumXLeft(PlayerLocation))
				PlayerInputLocation.Y += Val*PlayerMovementSpeed;

			if (!LylatController->IsOutOfFrustumXLeft(AimPointLocation))
				AimPointInputLocation.Y += Val*AimPointMovementSpeed;
			
			RightInputPressed = false;
			LeftInputPressed = true;
		}
	}

	if (Val != 0.0f)
		MovementInputPressed = true;

	CameraInputLocation.Y += Val*CameraMovementSpeed;
}