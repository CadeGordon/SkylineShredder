// Fill out your copyright notice in the Description page of Project Settings.


#include "BoostPad.h"
#include "SkylineShredderCharacter.h"
#include <GameFramework/CharacterMovementComponent.h>

// Sets default values
ABoostPad::ABoostPad()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ABoostPad::BeginPlay()
{
	Super::BeginPlay();
	
}

void ABoostPad::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Player = dynamic_cast<ASkylineShredderCharacter*>(OtherActor);

	if (!CurrentlyBoosting && Player != nullptr)
	{
		Player->BaseSpeed += BoostAmount;

		Player->SetMomentum(Player->GetMomentum() + 200.0f);

		CurrentlyBoosting = true;
	}
}

// Called every frame
void ABoostPad::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (CurrentlyBoosting)
	{
		CurrentBoostDuration += GetWorld()->DeltaTimeSeconds;

		if (CurrentBoostDuration >= BoostDuration && Player)
		{
			CurrentlyBoosting = false;
			Player->BaseSpeed -= BoostAmount;
			Player = nullptr;
			CurrentBoostDuration = 0.0f;
		}
	}
}

