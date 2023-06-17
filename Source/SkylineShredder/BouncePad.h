// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BouncePad.generated.h"

UCLASS()
class SKYLINESHREDDER_API ABouncePad : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABouncePad();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void NotifyHit(class UPrimitiveComponent* MyComp, AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit);

	void ToggleCanInteract() { CanInteract = !CanInteract; }
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY(EditAnywhere)
	FVector LaunchVelocity;

	bool CanInteract = true;

	float TimeUntilCanInteract = 0.5f;
	float CurrentTimeUntilCanInteract = 0.0f;

};
