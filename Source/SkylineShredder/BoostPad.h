// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BoostPad.generated.h"

UCLASS()
class SKYLINESHREDDER_API ABoostPad : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABoostPad();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;

public:	 
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY(EditAnywhere)
	float BoostAmount = 2000.0f;

	bool CurrentlyBoosting = false;
	float BoostDuration = 2.5f;
	float CurrentBoostDuration = 0.0f;

	class ASkylineShredderCharacter* Player = nullptr;

};
