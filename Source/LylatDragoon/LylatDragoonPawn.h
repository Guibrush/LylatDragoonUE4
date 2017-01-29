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

	/** The current value of the health */
	UPROPERTY(Category = Health, BlueprintReadOnly)
	float CurrentHealth;

	/** The current value of the energy */
	UPROPERTY(Category = Energy, BlueprintReadOnly)
	float CurrentEnergy;

	// Begin AActor overrides
	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void NotifyHit(class UPrimitiveComponent* MyComp, class AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit) override;
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
	virtual void NotifyActorEndOverlap(AActor* OtherActor) override;
	virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
	// End AActor overrides

	/** Return the aim point */
	FVector GetAimPointLocation();

protected:

	// Begin APawn overrides
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override; // Allows binding actions/axes to functions
	// End APawn overrides

	/** Bound to the thrust axis */
	void ThrustInput(float Val);
	
	/** Bound to the vertical axis */
	void MoveUpInput(float Val);

	/** Bound to the horizontal axis */
	void MoveRightInput(float Val);

	/** Bound to the left shoulder button pressed */
	void LeftTiltInputPressed();
	/** Bound to the left shoulder button released */
	void LeftTiltInputReleased();
	/** Bound to the left shoulder button double pressed */
	void LeftTiltDoubleInput();

	/** Bound to the right shoulder button pressed */
	void RightTiltInputPressed();
	/** Bound to the right shoulder button released */
	void RightTiltInputReleased();
	/** Bound to the right shoulder button double pressed */
	void RightTiltDoubleInput();

	/** Bound to the fire button */
	void FireInput();

private:

	/** Execute the die procedure */
	void Die();

	/** Callback when the cooldown of automatic energy refill ends */
	void FinishEnergyCooldown();

	/** Callback when we finish a barrel roll */
	void FinishBarrelRoll();

	/** Teleport the player to the level course position */
	void InitializePawnPosition();

	/** Blueprint of the projectile to shoot */
	UPROPERTY(Category = Combat, EditAnywhere)
	TSubclassOf<class ALylatDragoonProjectile> Projectile;

	/** Max level of energy */
	UPROPERTY(Category = Energy, EditAnywhere)
	float MaxEnergy;

	/** Max level of health */
	UPROPERTY(Category = Health, EditAnywhere)
	float MaxHealth;

	/** Indicates what was the last value of the right input */
	float RightInput;
	/** Indicates what was the last value of the up intput */
	float UpInput;

	/** Indicates what was the last value of the thrust input */
	float CurrentThrustInput;

	/** Indicates if the left tilt button was pressed */
	bool LeftTiltPressed;
	/** Indicates if the right tilt button was pressed */
	bool RightTiltPressed;

	/** Indicates if I am doing a barrel roll or not */
	bool DoingBarrelRoll;
	/** Indicates if I am doing a barrel roll to the left */
	bool DoingLeftBarrelRoll;
	/** Indicates if I am doing a barrel roll to the right */
	bool DoingRightBarrelRoll;

	/** Indicates if the automatic energy refill is in cooldown */
	bool EnergyInCooldown;

	/** Point where the ship shoot at, in world space */
	FVector AimPointLocation;

	/** Location of the player in the last frame */
	FVector PreviousLocation;

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
