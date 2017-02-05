// Fill out your copyright notice in the Description page of Project Settings.

#include "LylatDragoon.h"
#include "LylatDragoonEnemyCourse.h"


// Sets default values
ALylatDragoonEnemyCourse::ALylatDragoonEnemyCourse()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ALylatDragoonEnemyCourse::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ALylatDragoonEnemyCourse::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

}

