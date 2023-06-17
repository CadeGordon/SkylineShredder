// Fill out your copyright notice in the Description page of Project Settings.


#include "BouncePad.h"
#include "SkylineShredderCharacter.h"
#include <GameFramework/CharacterMovementComponent.h>

// Sets default values
ABouncePad::ABouncePad()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ABouncePad::BeginPlay()
{
	Super::BeginPlay();
	
}

void ABouncePad::NotifyHit(UPrimitiveComponent* MyComp, AActor* Other, UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit)
{
	if (CanInteract)
	{
		ASkylineShredderCharacter* player = dynamic_cast<ASkylineShredderCharacter*>(Other);

		player->GetCharacterMovement()->AddImpulse(LaunchVelocity, true);

		player->NumberOfJumps = 1;

		CanInteract = false;
	}
}

// Called every frame
void ABouncePad::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!CanInteract)
	{
		CurrentTimeUntilCanInteract += GetWorld()->DeltaTimeSeconds;

		if (CurrentTimeUntilCanInteract >= TimeUntilCanInteract)
		{
			CanInteract = true;
			CurrentTimeUntilCanInteract = 0.0f;
		}
	}
}

