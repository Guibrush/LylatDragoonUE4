// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "LylatDragoon.h"
#include "LylatDragoonPawn.h"

#include "LylatDragoonLevelCourse.h"
#include "LylatDragoonPlayerController.h"
#include "LylatDragoonProjectile.h"

#include "Matinee/MatineeActor.h"
#include "LevelSequenceActor.h"

#include "EngineUtils.h"
#include "DrawDebugHelpers.h"
#include "EngineGlobals.h"
#include "Engine/Engine.h"

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
	//SpringArm->AttachTo(RootComponent);
	SpringArm->TargetArmLength = 160.0f; // The camera follows at this distance behind the character	
	SpringArm->SocketOffset = FVector(0.f,0.f,60.f);
	SpringArm->bEnableCameraLag = false;
	SpringArm->CameraLagSpeed = 15.f;

	// Create camera component 
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera0"));
	Camera->AttachToComponent(SpringArm, FAttachmentTransformRules::KeepRelativeTransform, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false; // Don't rotate camera with controller

	LeftTiltPressed = false;
	RightTiltPressed = false;

	DoingBarrelRoll = false;
	DoingLeftBarrelRoll = false;
	DoingRightBarrelRoll = false;

	EnergyInCooldown = false;
	EnergyConsuptionRate = 10.0f;
	EnergyCooldownTime = 3.0f;
	EnergyRecoveryRate = 10.0f;

	SpeedChangeRate = 2.5f;
	MinSpeed = 0.5f;
	MaxSpeed = 2.0f;
	SpeedRecoveryRate = 1.0f;

	MovementRotationDegrees = 10.0f;
	RotChangeBarrellRollRate = 20.0f;
	RotChangeRate = 10.0f;
	RotationRecoveryRate = 2.5f;

	MovRefPointDistance = 100.0f;
	RightMovementLimit = 1000.0f;
	LeftMovementLimit = -1000.0f;
	UpMovementLimit = 500.0f;
	DownMovementLimit = -500.0f;

	VerticalCameraDisplacement = 0.75f;
	HorizontalCameraDisplacement = 0.75f;
	CamMovementRate = 10.0f;
	CamRotationDegrees = 0.5f;
	CamRotationRecoveryRate = 10.0f;
	CamRotationRate = 10.0f;

	AimPointDistance = 5000.0f;
}

void ALylatDragoonPawn::Tick(float DeltaSeconds)
{
	ALylatDragoonPlayerController* LylatController = Cast<ALylatDragoonPlayerController>(Controller);
	if (LevelCourse && LylatController)
	{
		if (!EnergyInCooldown)
		{
			if (CurrentThrustInput != 0.0f)
			{
				CurrentEnergy -= DeltaSeconds * EnergyConsuptionRate;
			}

			if (CurrentEnergy < 0.0f)
			{
				CurrentEnergy = 0.0f;
				EnergyInCooldown = true;
				FTimerHandle TimerHandle;
				GetWorldTimerManager().SetTimer(TimerHandle, this, &ALylatDragoonPawn::FinishEnergyCooldown, EnergyCooldownTime, false);
				CurrentThrustInput = 0.0f;
			}
			else
			{
				CurrentEnergy = FMath::Clamp(CurrentEnergy + DeltaSeconds * EnergyRecoveryRate, 0.0f, MaxEnergy);
			}
		}
		else
		{
			CurrentThrustInput = 0.0f;
		}

		float DesirePlayRate = LevelCourse->SequenceController->SequencePlayer->GetPlayRate() + CurrentThrustInput;
		float FinalPlayRate = FMath::Clamp(FMath::FInterpTo(LevelCourse->SequenceController->SequencePlayer->GetPlayRate(), DesirePlayRate, DeltaSeconds, SpeedChangeRate), MinSpeed, MaxSpeed);
		FinalPlayRate = FMath::FInterpTo(FinalPlayRate, 1.0f, DeltaSeconds, SpeedRecoveryRate);
		LevelCourse->SequenceController->SequencePlayer->SetPlayRate(FinalPlayRate);

		// Calculate the rotation according to the input
		FRotator DesireRotation = GetActorRotation();
		DesireRotation.Yaw += RightInput * MovementRotationDegrees;
		if (DoingBarrelRoll)
		{
			DesireRotation.Roll = DoingLeftBarrelRoll ? DesireRotation.Roll - 45.0f : DoingRightBarrelRoll ? DesireRotation.Roll + 45.0f : DesireRotation.Roll;
		}
		else
		{
			DesireRotation.Roll = RightTiltPressed ? 90.0f : DesireRotation.Roll + RightInput * MovementRotationDegrees;
			DesireRotation.Roll = LeftTiltPressed ? -90.0f : DesireRotation.Roll + RightInput * MovementRotationDegrees;
		}
		DesireRotation.Pitch += UpInput * MovementRotationDegrees;
		float RotationSpeed = DoingBarrelRoll ? RotChangeBarrellRollRate : RotChangeRate;
		FRotator FinalRotation = FMath::RInterpTo(GetActorRotation(), DesireRotation, DeltaSeconds, RotationSpeed);
		FinalRotation = FMath::RInterpTo(FinalRotation, LevelCourse->GetActorRotation(), DeltaSeconds, RotationRecoveryRate);

		//Calculate the position according to the rotation
		FVector FinalForwardDirection = FinalRotation.Vector();
		FVector FinalLocation = FMath::LinePlaneIntersection(GetActorLocation(), GetActorLocation() + FinalForwardDirection * MovRefPointDistance, LevelCourse->GetActorLocation(), LevelCourse->GetActorRotation().Vector());

		FVector PositionOffset = LevelCourse->GetActorLocation() - FinalLocation;
		if (PositionOffset.X > RightMovementLimit)
		{
			PositionOffset.X = RightMovementLimit;
		}

		if (PositionOffset.X < LeftMovementLimit)
		{
			PositionOffset.X = LeftMovementLimit;
		}

		if (PositionOffset.Z > UpMovementLimit)
		{
			PositionOffset.Z = UpMovementLimit;
		}

		if (PositionOffset.Z < DownMovementLimit)
		{
			PositionOffset.Z = DownMovementLimit;
		}

		FinalLocation = LevelCourse->GetActorLocation() - PositionOffset;

		SetActorLocation(FinalLocation);
		SetActorRotation(FinalRotation);

		FVector FinalSocketOffset = FVector::ZeroVector;
		FinalSocketOffset.Z = -PositionOffset.Z * VerticalCameraDisplacement;
		FinalSocketOffset.Y = PositionOffset.X * HorizontalCameraDisplacement;

		SpringArm->SocketOffset = FMath::VInterpTo(SpringArm->SocketOffset, FinalSocketOffset, DeltaSeconds, CamMovementRate);
		FRotator FinalCameraRotation = Camera->RelativeRotation;
		FinalCameraRotation.Roll += RightInput * CamRotationDegrees;
		FinalCameraRotation = FMath::RInterpTo(FinalCameraRotation, FRotator::ZeroRotator, DeltaSeconds, CamRotationRecoveryRate);
		Camera->SetRelativeRotation(FMath::RInterpTo(Camera->RelativeRotation, FinalCameraRotation, DeltaSeconds, CamRotationRate));

		AimPointLocation = GetActorLocation() + (GetActorRotation().Vector() * AimPointDistance);

		PreviousLocation = GetActorLocation();
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

	CurrentHealth = MaxHealth;
	CurrentEnergy = MaxEnergy;
}

void ALylatDragoonPawn::BeginPlay()
{
	Super::BeginPlay();

	FTimerHandle TimerHandle;
	GetWorldTimerManager().SetTimer(TimerHandle, this, &ALylatDragoonPawn::InitializePawnPosition, 1.0f, false);
}

void ALylatDragoonPawn::NotifyHit(class UPrimitiveComponent* MyComp, class AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit)
{
	Super::NotifyHit(MyComp, Other, OtherComp, bSelfMoved, HitLocation, HitNormal, NormalImpulse, Hit);
}

void ALylatDragoonPawn::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	if (OtherActor->GetOwner() != this)
	{
		TakeDamage(10.0f, FDamageEvent(), nullptr, OtherActor);
	}
}

void ALylatDragoonPawn::NotifyActorEndOverlap(AActor* OtherActor)
{
	Super::NotifyActorEndOverlap(OtherActor);
}

float ALylatDragoonPawn::TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	CurrentHealth -= Damage;

	if (CurrentHealth <= 0.0f)
	{
		Die();
	}

	return Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);
}

FVector ALylatDragoonPawn::GetAimPointLocation()
{
	return AimPointLocation;
}

void ALylatDragoonPawn::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	check(PlayerInputComponent);

	// Bind our control axis to callback functions
	PlayerInputComponent->BindAxis("Thrust", this, &ALylatDragoonPawn::ThrustInput);

	PlayerInputComponent->BindAxis("MoveUp", this, &ALylatDragoonPawn::MoveUpInput);
	PlayerInputComponent->BindAxis("MoveRight", this, &ALylatDragoonPawn::MoveRightInput);

	PlayerInputComponent->BindAction("LeftTilt", EInputEvent::IE_Pressed, this, &ALylatDragoonPawn::LeftTiltInputPressed);
	PlayerInputComponent->BindAction("LeftTilt", EInputEvent::IE_Released, this, &ALylatDragoonPawn::LeftTiltInputReleased);
	PlayerInputComponent->BindAction("LeftTilt", EInputEvent::IE_DoubleClick, this, &ALylatDragoonPawn::LeftTiltDoubleInput);

	PlayerInputComponent->BindAction("RightTilt", EInputEvent::IE_Pressed, this, &ALylatDragoonPawn::RightTiltInputPressed);
	PlayerInputComponent->BindAction("RightTilt", EInputEvent::IE_Released, this, &ALylatDragoonPawn::RightTiltInputReleased);
	PlayerInputComponent->BindAction("RightTilt", EInputEvent::IE_DoubleClick, this, &ALylatDragoonPawn::RightTiltDoubleInput);

	PlayerInputComponent->BindAction("Fire", EInputEvent::IE_Pressed, this, &ALylatDragoonPawn::FireInput);
}

void ALylatDragoonPawn::ThrustInput(float Val)
{
	CurrentThrustInput = Val;
}

void ALylatDragoonPawn::MoveUpInput(float Val)
{
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("MoveUpInput, Val: %f"), Val));

	if (Val < 0)
	{
		UpInput = Val + 1.0f;
	}
	else if (Val > 0)
	{
		UpInput = Val - 1.0f;
	}
	else
	{
		UpInput = Val;
	}
}

void ALylatDragoonPawn::MoveRightInput(float Val)
{
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("MoveRightInput, Val: %f"), Val));

	if (Val < 0)
	{
		RightInput = Val + 1.0f;
	}
	else if (Val > 0)
	{
		RightInput = Val - 1.0f;
	}
	else
	{
		RightInput = Val;
	}
}

void ALylatDragoonPawn::LeftTiltInputPressed()
{
	LeftTiltPressed = true;
}

void ALylatDragoonPawn::LeftTiltInputReleased()
{
	LeftTiltPressed = false;
}

void ALylatDragoonPawn::LeftTiltDoubleInput()
{
	if (!DoingBarrelRoll)
	{
		DoingBarrelRoll = true;
		DoingLeftBarrelRoll = true;
		FTimerHandle TimerHandle;
		GetWorldTimerManager().SetTimer(TimerHandle, this, &ALylatDragoonPawn::FinishBarrelRoll, 0.5f, false);
	}
}

void ALylatDragoonPawn::RightTiltInputPressed()
{
	RightTiltPressed = true;
}

void ALylatDragoonPawn::RightTiltInputReleased()
{
	RightTiltPressed = false;
}

void ALylatDragoonPawn::RightTiltDoubleInput()
{
	if (!DoingBarrelRoll)
	{
		DoingBarrelRoll = true;
		DoingRightBarrelRoll = true;
		FTimerHandle TimerHandle;
		GetWorldTimerManager().SetTimer(TimerHandle, this, &ALylatDragoonPawn::FinishBarrelRoll, 0.5f, false);
	}
}

void ALylatDragoonPawn::FireInput()
{
	if (Projectile)
	{
		FTransform SpawnTM(GetTransform());
		ALylatDragoonProjectile* ProjectileSpawned = Cast<ALylatDragoonProjectile>(UGameplayStatics::BeginDeferredActorSpawnFromClass(this, Projectile, SpawnTM));
		if (ProjectileSpawned)
		{
			ProjectileSpawned->Instigator = Instigator;
			ProjectileSpawned->SetOwner(this);

			UGameplayStatics::FinishSpawningActor(ProjectileSpawned, SpawnTM);
		}
	}
}

void ALylatDragoonPawn::Die()
{
	LevelCourse->SequenceController->SequencePlayer->SetPlaybackPosition(0.0f);
	CurrentHealth = MaxHealth;
}

void ALylatDragoonPawn::FinishEnergyCooldown()
{
	EnergyInCooldown = false;
}

void ALylatDragoonPawn::FinishBarrelRoll()
{
	DoingBarrelRoll = false;
	DoingLeftBarrelRoll = false;
	DoingRightBarrelRoll = false;
}

void ALylatDragoonPawn::InitializePawnPosition()
{
	if (LevelCourse)
	{
		SetActorLocation(LevelCourse->GetActorLocation());
		SetActorRotation(LevelCourse->GetActorRotation());
		SpringArm->AttachToComponent(LevelCourse->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
	}
}