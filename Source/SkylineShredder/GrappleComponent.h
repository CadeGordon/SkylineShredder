// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GrappleComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent), Blueprintable)
class SKYLINESHREDDER_API UGrappleComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	

public:	
	// Sets default values for this component's properties
	UGrappleComponent();

	UPROPERTY()
	class USphereComponent* HandPointCollider;

	

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


	virtual void CheckForGrapple();

	

	
	
private:
	
		
};
