// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "LylatDragoonLevelCourse.generated.h"

UCLASS()
class ALylatDragoonLevelCourse : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ALylatDragoonLevelCourse();

	// The matinee which control this actor
	UPROPERTY(Category = Movement, EditAnywhere)
	class AMatineeActor* MatineeController;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

	FORCEINLINE FVector GetMovementDirection() const { return MovementDirection; }

private:

	FVector MovementDirection;

	FVector PreviousLocation;
	
};
