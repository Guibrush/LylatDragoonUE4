// Fill out your copyright notice in the Description page of Project Settings.

#include "LylatDragoon.h"
#include "LylatDragoonEnemySpawner.h"


// Sets default values
ALylatDragoonEnemySpawner::ALylatDragoonEnemySpawner()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ALylatDragoonEnemySpawner::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ALylatDragoonEnemySpawner::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

}

