// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "LylatDragoonEnemyCourse.generated.h"

UCLASS()
class LYLATDRAGOON_API ALylatDragoonEnemyCourse : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ALylatDragoonEnemyCourse();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

	
	
};
