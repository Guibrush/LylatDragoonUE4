// Fill out your copyright notice in the Description page of Project Settings.

#include "LylatDragoon.h"
#include "LylatDragoonLevelCourse.h"


// Sets default values
ALylatDragoonLevelCourse::ALylatDragoonLevelCourse()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ALylatDragoonLevelCourse::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ALylatDragoonLevelCourse::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

	MovementDirection = GetActorLocation() - PreviousLocation;

	SetActorRotation(MovementDirection.Rotation());

	PreviousLocation = GetActorLocation();
}

