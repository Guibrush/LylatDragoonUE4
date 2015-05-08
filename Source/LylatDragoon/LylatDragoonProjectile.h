// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "LylatDragoonProjectile.generated.h"

UCLASS()
class ALylatDragoonProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ALylatDragoonProjectile();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

	/** Projectile speed when fired */
	UPROPERTY(Category = ProjectileMovement, EditAnywhere)
	float ProjectileSpeed;

	
	
};
