// Fill out your copyright notice in the Description page of Project Settings.

#include "LylatDragoon.h"
#include "LylatDragoonProjectile.h"


// Sets default values
ALylatDragoonProjectile::ALylatDragoonProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ALylatDragoonProjectile::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ALylatDragoonProjectile::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );
	
	FVector FinalLocation = GetActorLocation() + (GetActorRotation().Vector().SafeNormal() * (DeltaTime * ProjectileSpeed));

	SetActorLocation(FinalLocation);
}

