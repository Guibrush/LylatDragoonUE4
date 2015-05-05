// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "LylatDragoon.h"
#include "LylatDragoonPawn.h"

#include "LylatDragoonLevelCourse.h"
#include "LylatDragoonPlayerController.h"

#include "Matinee/MatineeActor.h"

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
	PlayerMovementSpeed = 50.f;

	PlayerInputLocation = FVector(0.0f, 0.0f, 0.0f);
	PreviousLevelCourseLocation = FVector(0.0f, 0.0f, 0.0f);

	RightTiltPressed = false;
	LeftTiltPressed = false;

	CurrentThrustFuel = ThrustFuel;
	bThrustFuelRecoveryIsInCooldown = false;

	CurrentBrakeResistance = BrakeResistance;
	bBrakeResistanceRecoveryIsInCooldown = false;
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
		//FRotator FinalRotation = PawnToAimPoint.Rotation();
		FRotator FinalRotation = FMath::RInterpTo(GetActorRotation(), PawnToAimPoint.Rotation(), DeltaSeconds, PlayerRotationRecoverySpeed);

		if (RightTiltPressed)
		{
			FinalRotation.Roll = FMath::FInterpTo(FinalRotation.Roll, 90.0f, DeltaSeconds, PlayerTiltRotationSpeed);
		}

		if (LeftTiltPressed)
		{
			FinalRotation.Roll = FMath::FInterpTo(FinalRotation.Roll, -90.0f, DeltaSeconds, PlayerTiltRotationSpeed);
		}
		
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
			FRotator RotationAdded = FRotator::ZeroRotator;

			if (RightInputPressed)
			{
				RotationAdded += FRotator(0.0f, 20.0f, 5.0f);
			}
			else if (LeftInputPressed)
			{
				RotationAdded += FRotator(0.0f, -20.0f, -5.0f);
			}

			if (UpInputPressed)
			{
				RotationAdded += FRotator(-20.0f, 0.0f, 0.0f);
			}
			else if (DownInputPressed)
			{
				RotationAdded += FRotator(20.0f, 0.0f, 0.0f);
			}

			Camera->SetRelativeRotation(FMath::RInterpTo(Camera->RelativeRotation, RotationAdded, DeltaSeconds, CameraRotationSpeed));
		}
		
		// Camera orientation recovery
		Camera->SetRelativeRotation(FMath::RInterpTo(Camera->RelativeRotation, FRotator::ZeroRotator, DeltaSeconds, CameraRecoverySpeed));
	}

	MovementInputPressed = false;

	if (!bThrustFuelRecoveryIsInCooldown)
	{
		CurrentThrustFuel = FMath::Clamp(CurrentThrustFuel + (ThrustFuelRecoveryRate * DeltaSeconds), 0.0f, ThrustFuel);
	}

	if (!bBrakeResistanceRecoveryIsInCooldown)
	{
		CurrentBrakeResistance = FMath::Clamp(CurrentBrakeResistance + (BrakeResistanceRecoveryRate * DeltaSeconds), 0.0f, BrakeResistance);
	}

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
}


void ALylatDragoonPawn::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	check(InputComponent);

	// Bind our control axis' to callback functions
	InputComponent->BindAxis("Thrust", this, &ALylatDragoonPawn::ThrustInput);

	InputComponent->BindAxis("MoveUp", this, &ALylatDragoonPawn::MoveUpInput);
	InputComponent->BindAxis("MoveRight", this, &ALylatDragoonPawn::MoveRightInput);

	InputComponent->BindAction("RightTilt", EInputEvent::IE_Pressed, this, &ALylatDragoonPawn::RightTiltPressedInput);
	InputComponent->BindAction("RightTilt", EInputEvent::IE_Released, this, &ALylatDragoonPawn::RightTiltReleaseInput);
	InputComponent->BindAction("RightTilt", EInputEvent::IE_DoubleClick, this, &ALylatDragoonPawn::RightBarrelRollInput);

	InputComponent->BindAction("LeftTilt", EInputEvent::IE_Pressed, this, &ALylatDragoonPawn::LeftTiltPressedInput);
	InputComponent->BindAction("LeftTilt", EInputEvent::IE_Released, this, &ALylatDragoonPawn::LeftTiltReleasedInput);
	InputComponent->BindAction("LeftTilt", EInputEvent::IE_DoubleClick, this, &ALylatDragoonPawn::LeftBarrelRollInput);
}

void ALylatDragoonPawn::ThrustInput(float Val)
{
	float FinalPlayRate = 1.0f;

	if (!FMath::IsNearlyEqual(Val, 0.f))
	{
		if ((Val > 0.0f) && (CurrentThrustFuel > 0.0f) && !bThrustFuelRecoveryIsInCooldown)
		{
			FinalPlayRate = FMath::Clamp(FinalPlayRate + (Val * Acceleration), MinMatineeSpeed, MaxMatineeSpeed);
			CurrentThrustFuel -= Val * ThrustFuelConsumptionRate;
			
			if (CurrentThrustFuel <= 0.0f)
			{
				bThrustFuelRecoveryIsInCooldown = true;
				GetWorldTimerManager().SetTimer(this, &ALylatDragoonPawn::ThrustFuelRecoveryCooldownFinish, ThrustFuelRecoveryCooldown, false);
			}
		}

		if ((Val < 0.0f) && (CurrentBrakeResistance > 0.0f) && !bBrakeResistanceRecoveryIsInCooldown)
		{
			FinalPlayRate = FMath::Clamp(FinalPlayRate + (Val * Acceleration), MinMatineeSpeed, MaxMatineeSpeed);
			CurrentBrakeResistance += Val * BrakeResistanceConsumptionRate;

			if (CurrentBrakeResistance <= 0.0f)
			{
				bBrakeResistanceRecoveryIsInCooldown = true;
				GetWorldTimerManager().SetTimer(this, &ALylatDragoonPawn::BreakResistanceRecoveryCooldownFinish, BrakeResistanceRecoveryCooldown, false);
			}
		}
	}

	CurrentThrustFuel = FMath::Clamp(CurrentThrustFuel, 0.0f, ThrustFuel);
	CurrentBrakeResistance = FMath::Clamp(CurrentBrakeResistance, 0.0f, BrakeResistance);

	LevelCourse->MatineeController->PlayRate = FMath::FInterpTo(LevelCourse->MatineeController->PlayRate, FinalPlayRate, GetWorld()->DeltaTimeSeconds, 1000.0f);
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
	
	if (PlayerInputLocation.Z < -PlayerMovementLimit.Z)
	{
		PlayerInputLocation.Z = -PlayerMovementLimit.Z;
	}
	else if (PlayerInputLocation.Z > PlayerMovementLimit.Z)
	{
		PlayerInputLocation.Z = PlayerMovementLimit.Z;
	}
	else
	{
		CameraInputLocation.Z += Val*CameraMovementSpeed;
	}
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

	if (PlayerInputLocation.Y < -PlayerMovementLimit.Y)
	{
		PlayerInputLocation.Y = -PlayerMovementLimit.Y;
	}
	else if (PlayerInputLocation.Y > PlayerMovementLimit.Y)
	{
		PlayerInputLocation.Y = PlayerMovementLimit.Y;
	}
	else
	{
		CameraInputLocation.Y += Val*CameraMovementSpeed;
	}
}

void ALylatDragoonPawn::RightTiltPressedInput()
{
	RightTiltPressed = true;
}

void ALylatDragoonPawn::RightTiltReleaseInput()
{
	RightTiltPressed = false;
}

void ALylatDragoonPawn::RightBarrelRollInput()
{

}

void ALylatDragoonPawn::LeftTiltPressedInput()
{
	LeftTiltPressed = true;
}

void ALylatDragoonPawn::LeftTiltReleasedInput()
{
	LeftTiltPressed = false;
}

void ALylatDragoonPawn::LeftBarrelRollInput()
{

}

void ALylatDragoonPawn::ThrustFuelRecoveryCooldownFinish()
{
	bThrustFuelRecoveryIsInCooldown = false;
}

void ALylatDragoonPawn::BreakResistanceRecoveryCooldownFinish()
{
	bBrakeResistanceRecoveryIsInCooldown = false;
}