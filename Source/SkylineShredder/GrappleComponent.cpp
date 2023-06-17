// Fill out your copyright notice in the Description page of Project Settings.


#include "GrappleComponent.h"
#include "SkylineShredderCharacter.h"
#include "Camera/CameraComponent.h"
#include <Components/SphereComponent.h>
#include <GameFramework/SpringArmComponent.h>
#include <Kismet/KismetSystemLibrary.h>

// Sets default values for this component's properties
UGrappleComponent::UGrappleComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	HandPointCollider = CreateDefaultSubobject<USphereComponent>(TEXT("HandPointCollider"));
	UPROPERTY(EditAnywhere, BlueprintCallable, Category = "Grapple")
	HandPointCollider->SetSphereRadius(10.0f);
	UPROPERTY(EditAnywhere)
	HandPointCollider->SetRelativeLocation(FVector(0.0f, 40.0f, 0.0f));
	

	

	// ...
}


// Called when the game starts
void UGrappleComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

// Called every frame
void UGrappleComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UGrappleComponent::CheckForGrapple()
{
	
	
	
	
	


}

