// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "LylatDragoon.h"
#include "LylatDragoonPawn.h"

#include "LylatDragoonLevelCourse.h"
#include "LylatDragoonPlayerController.h"
#include "LylatDragoonProjectile.h"

#include "Matinee/MatineeActor.h"

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
	Camera->AttachTo(SpringArm, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false; // Don't rotate camera with controller
}

void ALylatDragoonPawn::Tick(float DeltaSeconds)
{
	ALylatDragoonPlayerController* LylatController = Cast<ALylatDragoonPlayerController>(Controller);
	if (LevelCourse && LylatController)
	{
		FRotator DesireRotation = GetActorRotation();
		DesireRotation.Yaw += RightInput * 10.0f;
		DesireRotation.Roll += RightInput * 10.0f;
		DesireRotation.Pitch += UpInput * 10.0f;
		FRotator FinalRotation = FMath::RInterpTo(GetActorRotation(), DesireRotation, DeltaSeconds, 10.0f);
		FinalRotation = FMath::RInterpTo(FinalRotation, LevelCourse->GetActorRotation(), DeltaSeconds, 2.5f);

		FVector FinalForwardDirection = FinalRotation.Vector();
		FVector FinalLocation = FMath::LinePlaneIntersection(GetActorLocation(), GetActorLocation() + FinalForwardDirection * 100.0f, LevelCourse->GetActorLocation(), LevelCourse->GetActorRotation().Vector());

		FVector PositionOffset = LevelCourse->GetActorLocation() - FinalLocation;
		if (FMath::Abs(PositionOffset.X) > 1000.0f || FMath::Abs(PositionOffset.Z) > 500.0f)
		{
			FinalLocation = FMath::LinePlaneIntersection(GetActorLocation(), GetActorLocation() + LevelCourse->GetMovementDirection() * 100.0f, LevelCourse->GetActorLocation(), LevelCourse->GetActorRotation().Vector());
		}

		SetActorLocation(FinalLocation);
		SetActorRotation(FinalRotation);

		FVector FinalSocketOffset = FVector::ZeroVector;
		FinalSocketOffset.Z = -PositionOffset.Z * 0.75f;
		FinalSocketOffset.Y = PositionOffset.X * 0.75f;

		SpringArm->SocketOffset = FMath::VInterpTo(SpringArm->SocketOffset, FinalSocketOffset, DeltaSeconds, 10.0f);
		FRotator FinalCameraRotation = Camera->RelativeRotation;
		FinalCameraRotation.Roll += RightInput * 0.5f;
		FinalCameraRotation = FMath::RInterpTo(FinalCameraRotation, FRotator::ZeroRotator, DeltaSeconds, 10.0f);
		Camera->SetRelativeRotation(FMath::RInterpTo(Camera->RelativeRotation, FinalCameraRotation, DeltaSeconds, 10.0f));

		AimPointLocation = GetActorLocation() + (GetActorRotation().Vector() * 5000.0f);

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

void ALylatDragoonPawn::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	check(InputComponent);

	// Bind our control axis to callback functions
	InputComponent->BindAxis("Thrust", this, &ALylatDragoonPawn::ThrustInput);

	InputComponent->BindAxis("MoveUp", this, &ALylatDragoonPawn::MoveUpInput);
	InputComponent->BindAxis("MoveRight", this, &ALylatDragoonPawn::MoveRightInput);

	InputComponent->BindAction("BarrelRoll", EInputEvent::IE_DoubleClick, this, &ALylatDragoonPawn::BarrelRollInput);

	InputComponent->BindAction("Fire", EInputEvent::IE_Pressed, this, &ALylatDragoonPawn::FireInput);
}

void ALylatDragoonPawn::ThrustInput(float Val)
{
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

void ALylatDragoonPawn::BarrelRollInput()
{
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
	LevelCourse->MatineeController->SetPosition(0.0f, true);
	CurrentHealth = MaxHealth;
}

void ALylatDragoonPawn::InitializePawnPosition()
{
	if (LevelCourse)
	{
		SetActorLocation(LevelCourse->GetActorLocation());
		SetActorRotation(LevelCourse->GetActorRotation());
		SpringArm->AttachTo(LevelCourse->GetRootComponent());
	}
}