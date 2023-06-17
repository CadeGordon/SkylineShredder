// Copyright Epic Games, Inc. All Rights Reserved.

#include "SkylineShredderCharacter.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"
#include <Kismet/KismetSystemLibrary.h>
#include "Kismet/GameplayStatics.h"
#include <Math/Vector.h>

//////////////////////////////////////////////////////////////////////////
// ATestComplexSystemCharacter

ASkylineShredderCharacter::ASkylineShredderCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	BaseSpeed = GetCharacterMovement()->MaxWalkSpeed;

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)
}

/// <summary>
/// Update for the character
/// </summary>
/// <param name="deltaTime"></param>
void ASkylineShredderCharacter::Tick(float deltaTime)
{
	Super::Tick(deltaTime);

	if (DoubleJumped)
		DoubleJumped = false;

	//Gets the forward velocity of the player
	float ForwardVelocity = FVector::DotProduct(GetVelocity(), GetActorForwardVector());

	// If Player is not moving at all, decrease momentum drastically.
	if (!InAction && GetCharacterMovement()->Velocity.Size() <= 0 && _momentum > 0)
		_momentum -= 5.0f;
	// If the Player is running on the ground, increase momentum to a point.
	else if (!InAction && GetCharacterMovement()->IsMovingOnGround() && _momentum <= 600)
		_momentum += 1.0f;
	// If the Player is wall running, increase momentum to a point.
	else if (IsWallRunning)
		_momentum += 1.0f;

	// If the Player is running, but not in action, decrease momentum to a point.
	if (!InAction && GetCharacterMovement()->IsMovingOnGround() && _momentum > 600)
		_momentum -= 3.0f;

	// If momentum begins to go above this point, set it back to this point.
	if (_momentum > 1500)
		_momentum = 1500;

	GetCharacterMovement()->MaxWalkSpeed = BaseSpeed + _momentum;

	//Sets the current height of the player for wall running
	_currentFrameHeight = GetActorLocation().Z;

	if (_jumping)
		CustomJump();

	//If the character is falling, check for wallrunning
	if (GetCharacterMovement()->IsFalling())
	{
		CheckForWallRunning();
		if (!IsWallRunning && !InAction)
		{
			_gravity += .05;
			if (_gravity >= 3.0f)
				_gravity = 3.0f;
			GetCharacterMovement()->GravityScale = _gravity;
		}
			
	}
	//Else...
	else
	{
		//...set wallrunning, inaction, rightside and leftside to be false
		//to turn off wall running
		IsWallRunning = false;
		RightSide = false;
		LeftSide = false;
		//Set the gravity scale and plane constraint back to normal
		_gravity = 0;
		GetCharacterMovement()->GravityScale = 1.0f;
		GetCharacterMovement()->SetPlaneConstraintNormal(FVector(0.0f, 0.0f, 0.0f));
	}

	//If the forward velocity is less than 100 and the player is still wallrunning...
	/*if (ForwardVelocity <= 100.0f && _isWallRunning)
	{
		//...turn off wallrunning by setting wallrunning, inaction, rightside and leftside to be false
		_isWallRunning = false;
		inAction = false;
		_rightSide = false;
		_leftSide = false;
		//Set the gravity scale and plane constraint back to normal
		GetCharacterMovement()->GravityScale = 50.0f;
		GetCharacterMovement()->SetPlaneConstraintNormal(FVector(0.0f, 0.0f, 0.0f));

		//GetWorldTimerManager().SetTimer(timerHandle, this, &ATestComplexSystemCharacter::TurnOffJumpOffWall, 1.5f, false);
	}
	*/
	//Handles grapple and constantly applies Grapple logic
	if (GrappleHookAttached) {
		DoGrapple();
	}
	//The cooldown timer for boosting currently 2 seconds
	if (HasAppliedBoost && (GetWorld()->TimeSeconds - LastBoostTime > BoostCooldownTime)) {
		HasAppliedBoost = false;
	}

	/*if (IsDashing)
	{
		DashTimeRemaining -= deltaTime;

		if (DashTimeRemaining <= 0.0f)
		{
			IsDashing = false;

			UGameplayStatics::GetPlayerController(GetWorld(), 0)->SetIgnoreLookInput(false);
		}
	}*/

	//Set the last frame height to be the current frame height
	_lastFrameHeight = _currentFrameHeight;
}

//////////////////////////////////////////////////////////////////////////
// Input

void ASkylineShredderCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ASkylineShredderCharacter::CheckJump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ASkylineShredderCharacter::CheckJump);

	//PlayerInputComponent->BindAction("Grapple", EInputEvent::IE_Pressed, this, &ASkylineShredderCharacter::CheckForGrapple);
	//PlayerInputComponent->BindAction("Grapple", EInputEvent::IE_Released, this, &ASkylineShredderCharacter::EndGrapple);
	//PlayerInputComponent->BindAction("GrappleBoost", IE_Pressed, this, &ASkylineShredderCharacter::GrappleBoost);


	PlayerInputComponent->BindAxis("MoveForward", this, &ASkylineShredderCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ASkylineShredderCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &ASkylineShredderCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &ASkylineShredderCharacter::LookUpAtRate);

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &ASkylineShredderCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &ASkylineShredderCharacter::TouchStopped);

	// VR headset functionality
	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &ASkylineShredderCharacter::OnResetVR);
}

/// <summary>
/// Sets the players move speed
/// </summary>
/// <param name="speed">the speed to set the player to</param>
void ASkylineShredderCharacter::SetMoveSpeed(float speed)
{
	GetCharacterMovement()->MaxWalkSpeed = speed;
}

/// <summary>
/// Checks if the player can climb the object it is facing
/// </summary>
/// <returns>true if the player can climb</returns>
bool ASkylineShredderCharacter::CheckForClimbing()
{
	//Hit result and collision params for use in line tracing
	FHitResult out;
	FCollisionQueryParams TraceParams;
	//Ignores the player for line tracing
	TraceParams.AddIgnoredActor(this);

	//Get the actor location and forward
	FVector actorLocation = GetActorLocation();
	FVector actorForward = GetActorForwardVector();

	//Sets the actor location and forward to what it needs to be to climb
	actorLocation.Z -= 44.0f;
	actorForward = actorForward * 70.0f;

	//Sets the start location and end location for line tracing
	FVector startLocation = actorLocation;
	FVector endLocation = actorLocation + actorForward;

	//Line traces to the object to climb
	bool hasHit = GetWorld()->LineTraceSingleByChannel(out, startLocation, endLocation, ECC_Visibility, TraceParams);

	//If the line trace hits nothing, return
	if (!hasHit)
		return false;

	//Gets the objects location and facing
	_wallLocation = out.Location;
	_wallNormal = out.Normal;

	//Creates a rotator from the wall normal and gets the forward vector from the wall
	FRotator rotator = UKismetMathLibrary::MakeRotFromX(_wallNormal);
	FVector wallForward = UKismetMathLibrary::GetForwardVector(rotator);

	//Sets the start and end location for line tracing using the walls forward and location.
	//line traces to get the height of the wall to see if the player can vault or climb that high
	wallForward *= -10.0f;
	startLocation = (wallForward + _wallLocation);
	startLocation.Z += 200.0f;
	endLocation = startLocation;
	endLocation.Z -= 200.0f;

	//Line trace the wall
	hasHit = GetWorld()->LineTraceSingleByChannel(out, startLocation, endLocation, ECC_Visibility, TraceParams);

	//If the line trace hits nothing, return
	if (!hasHit)
		return false;

	//Wall height is the out location of the line trace
	_wallHeight = out.Location;
	//Sets if the player should climb based off the height of the wall
	ShouldPlayerClimb = _wallHeight.Z - _wallLocation.Z > 50.0f;

	//Creates a rotator from the wall normal and gets the forward vector from the wall
	rotator = UKismetMathLibrary::MakeRotFromX(_wallNormal);
	wallForward = UKismetMathLibrary::GetForwardVector(rotator);

	//Sets the start and end location for line tracing using the walls forward and location.
	//Line traces to get the thickness of the wall
	wallForward *= -50.0f;
	startLocation = (wallForward + _wallLocation);
	startLocation.Z += 250.0f;
	endLocation = startLocation;
	endLocation.Z -= 300.0f;

	//Line trace the wall to check the thickness 
	hasHit = GetWorld()->LineTraceSingleByChannel(out, startLocation, endLocation, ECC_Visibility, TraceParams);

	//If the line trace hits nothing, the wall is not thick
	if (!hasHit)
		_isWallThick = false;

	//The height of the other wall is the line traces hit location
	_otherWallHeight = out.Location;

	float wallThickness = _wallHeight.Z - _otherWallHeight.Z;
	FString debugMessage = FString::SanitizeFloat(_otherWallHeight.Z);
	//GEngine->AddOnScreenDebugMessage(1, 2.5f, FColor::Blue, debugMessage);

	//Sets if the wall is too thick based off the width of the walls hit
	_isWallThick = !(_wallHeight.Z - _otherWallHeight.Z > 10.0f);
	if (wallThickness == 0.0f)
		_isWallThick = false;

	return true;
}

/// <summary>
/// Starts vaulting functionality
/// </summary>
void ASkylineShredderCharacter::StartVaultOrGetUp()
{
	//If already in action, return then set in action  and is climbing to be true
	if (InAction || IsClimbing || IsVaulting)
		return;
	InAction = true;

	//Set the player collision to be off and movement mode to be none
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Flying);

	//Make a new vector for use in setting the actors location
	FVector actorNewLocation;

	//If the wall is too thick to vault over, then climb on top of the object
	if (_isWallThick)
	{
		//Set climing to be true
		IsClimbing = true;
		//Create a rotator from the walls normal and get the wall forward based off of that
		FRotator rotator = UKismetMathLibrary::MakeRotFromX(_wallNormal);
		FVector wallForward = UKismetMathLibrary::GetForwardVector(rotator);
		//Set the wall forward back by 50
		wallForward *= 50.0f;
		//Set the new location to be where the player is plus the wall forward
		//This is so the animation can play smoothly
		actorNewLocation = wallForward + GetActorLocation();
		SetActorLocation(actorNewLocation);
	}

	//If the wall is not too thick then the player can vault
	else
	{
		//Set is vaulting to be true
		IsVaulting = true;
		//Set the new location to be the players location
		//And the height is equal to the wall height minus 20
		//This si so the animation can play smoothly
		actorNewLocation = GetActorLocation();
		actorNewLocation.Z = _wallHeight.Z - 20.0f;
		SetActorLocation(actorNewLocation);

	}

	_momentum += 200.0f;

	//Set a timer so after the animation plays the collision, movement, and consequent booleans are set off
	GetWorldTimerManager().SetTimer(timerHandle, this, &ASkylineShredderCharacter::StopVaultOrGetUp, .15f, false);
}

/// <summary>
/// Stops vaulting functionality
/// </summary>
void ASkylineShredderCharacter::StopVaultOrGetUp()
{
	//Set the movement and collision back to normal
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
	TurnOffJumpOffWall();

	//Set the booleans to be false since the action is done
	InAction = false;
	IsClimbing = false;
	IsVaulting = false;
}

/// <summary>
/// Checks for wall running and does the wall running functionality. Is 
/// called in update and only called when the player is in the air and 
/// falling downwards
/// </summary>
void ASkylineShredderCharacter::CheckForWallRunning()
{
	if (GrappleHookAttached)
		return;
	if (GetInputAxisValue("MoveForward") == 0.0f && GetInputAxisValue("MoveRight") == 0.0f && IsWallRunning)
	{ 
		//Set the gravity scale and plane constraints back to normal
		GetCharacterMovement()->GravityScale = 1.0f;
		GetCharacterMovement()->SetPlaneConstraintNormal(FVector(0.0f, 0.0f, 0.0f));

		//Get the right vector and select if the player launches to the right or the left
		//based off if the player is on the right side of a wall or not
		FVector actorRightVector = GetActorRightVector();
		FVector launchVelocity = UKismetMathLibrary::SelectVector(actorRightVector * -250.0f, actorRightVector * 250.0f, _onRightSide);

		//Set the launch velocity z to be higher
		launchVelocity.Z = 0.0f;

		//Launch the character using the launch velocity
		LaunchCharacter(launchVelocity, false, false);

		NumberOfJumps = 1;

		IsWallRunning = false;
		InAction = false;
		LeftSide = false;
		RightSide = false;

		return;
	}
		
	//If the player is not on the left side of the wall
	if (!LeftSide)
	{

		//Hit result and collision params for use in line tracing
		FHitResult out;
		//FCollisionQueryParams TraceParams;

		TArray<AActor*> Actors;
		//Ignores the player for line tracing
		//TraceParams.AddIgnoredActor(this);

		//Create a start location and end location for use in line tracing
		//The start location is the actors location and the end location is to the right of the player
		FVector startLocation = GetActorLocation();
		FVector endLocation = (GetActorRightVector() * 50.0f) + startLocation;

		//Line trace to the right
		//bool hasHit = GetWorld()->LineTraceSingleByChannel(out, startLocation, endLocation, ECC_Visibility, TraceParams);
		bool hasHit = UKismetSystemLibrary::SphereTraceSingle(GetWorld(), startLocation, endLocation, 30.0f, ObjectType, false, Actors, EDrawDebugTrace::None, out, true);

		//If the line trace has hit a wall, and the player is falling downwards, and the player is on the ground
		if (hasHit && _currentFrameHeight - _lastFrameHeight <= 0.0f && !GetCharacterMovement()->IsMovingOnGround())
		{
			if (!out.GetActor())
			{
				//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, out.GetComponent()->GetFName().ToString());
				for (int i = 0; i < out.GetComponent()->ComponentTags.Num(); i++)
				{
					
					//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString("hi"));
					//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString(out.GetComponent()->ComponentTags[i].ToString()));
				}
				
				
				return;
			}
				
			//If the wall is tagged not to wall run on, return
			if (out.GetActor()->ActorHasTag("NoWallrun"))
				return;

			//Set right side and on right side to be true
			RightSide = true;
			_onRightSide = true;

			//If the player is not jumping off of the wall
			if (!_isJumpingOffWall)
			{
				//Set in action to be true
				InAction = true;

				//Create a new rotator from the walls normal
				FRotator newRotation = UKismetMathLibrary::MakeRotFromX(out.Normal);
				//Set the rotation to be exactly 90 degrees
				newRotation.Yaw += 90.0f;
				newRotation.Roll = 0.0f;
				newRotation.Pitch = 0.0f;
				//Set the players rotation
				SetActorRotation(newRotation);

				//Get the actors forward
				FVector actorForward = GetActorForwardVector();
				//Set it to be straight ahead with no up or down movement
				actorForward.X *= FMath::Clamp(GetVelocity().Size(), 0.0f, 1200.0f + GetMomentum());
				
				actorForward.Y *= FMath::Clamp(GetVelocity().Size(), 0.0f, 1200.0f + GetMomentum());
				actorForward.Z = 0.0f;

				//Set the gravity scale to be higher than normal to slowly fall off the wall
				//Set the velocity to be the actors forward
				//Set the plane constraint to be 1 on the z to lock the player
				GetCharacterMovement()->GravityScale = 15.0f;
				GetCharacterMovement()->Velocity = actorForward;
				GetCharacterMovement()->SetPlaneConstraintNormal(FVector(0.0f, 0.0f, 1.0f));

				//Set is wall running to be true
				IsWallRunning = true;
				_gravity = 0;
			}
		}

		//If none of the if statements are true
		else
		{
			//Set the booleans to be false
			IsWallRunning = false;
			InAction = false;
			RightSide = false;
			//Set the gravity scale and plane constraints back to normal
			GetCharacterMovement()->GravityScale = 1.0f;
			GetCharacterMovement()->SetPlaneConstraintNormal(FVector(0.0f, 0.0f, 0.0f));
		}
	}

	//If the player is not on the right side
	if (!RightSide)
	{
		//Hit result and collision params for use in line tracing
		FHitResult out;
		FCollisionQueryParams TraceParams;
		//Ignores the player for line tracing
		TraceParams.AddIgnoredActor(this);
		TArray<AActor*> Actors;

		//Create a start location and end location for use in line tracing
		//The start location is the actors location and the end location is to the left of the player
		FVector startLocation = GetActorLocation();
		FVector endLocation = (GetActorRightVector() * -50.0f) + startLocation;

		//Line trace to the left
		//bool hasHit = GetWorld()->LineTraceSingleByChannel(out, startLocation, endLocation, ECC_Visibility, TraceParams);
		bool hasHit = UKismetSystemLibrary::SphereTraceSingle(GetWorld(), startLocation, endLocation, 30.0f, ObjectType, false, Actors, EDrawDebugTrace::None, out, true);

		//If the line trace has hit a wall, and the player is falling downwards, and the player is on the ground
		if (hasHit && _currentFrameHeight - _lastFrameHeight <= 0.0f && !GetCharacterMovement()->IsMovingOnGround())
		{
			if (!out.GetActor())
				return;
			//If the wall is tagged not to wall run on, return
			if (out.GetActor()->ActorHasTag("NoWallrun"))
				return;

			//Set left side to be true and on right side to be false
			LeftSide = true;
			_onRightSide = false;

			//If the player is not jumping off the wall
			if (!_isJumpingOffWall)
			{
				//Set in action to be true
				InAction = true;

				//Create a new rotator from the walls normal
				FRotator newRotation = UKismetMathLibrary::MakeRotFromX(out.Normal);
				//Set the rotation to be exactly negative 90 degrees
				newRotation.Yaw -= 90.0f;
				newRotation.Roll = 0.0f;
				newRotation.Pitch = 0.0f;
				//Set the players rotation
				SetActorRotation(newRotation);

				//Get the actors forward
				FVector actorForward = GetActorForwardVector();
				//Set it to be straight ahead with no up or down movement
				actorForward.X *= FMath::Clamp(GetVelocity().Size(), 0.0f, 1200.0f + GetMomentum());
				actorForward.Y *= FMath::Clamp(GetVelocity().Size(), 0.0f, 1200.0f + GetMomentum());
				actorForward.Z = 0.0f;

				//Set the gravity scale to be higher than normal to slowly fall off the wall
				//Set the velocity to be the actors forward
				//Set the plane constraint to be 1 on the z to lock the player
				GetCharacterMovement()->GravityScale = 15.0f;
				GetCharacterMovement()->Velocity = actorForward;
				GetCharacterMovement()->SetPlaneConstraintNormal(FVector(0.0f, 0.0f, 1.0f));

				//Set is wallrunning to be true
				IsWallRunning = true;
				_gravity = 0;
			}
		}

		//If none of the if statements are true
		else
		{
			//Set the booleans to be false
			IsWallRunning = false;
			InAction = false;
			LeftSide = false;
			//Set the gravity scale and plane constraints back to normal
			GetCharacterMovement()->GravityScale = 1.0f;
			GetCharacterMovement()->SetPlaneConstraintNormal(FVector(0.0f, 0.0f, 0.0f));
		}
	}
}

/// <summary>
/// When the character lands on the ground
/// </summary>
/// <param name="Hit"></param>
void ASkylineShredderCharacter::Landed(const FHitResult& Hit)
{
	//Calls the original function
	Super::Landed(Hit);

	//Sets number of jumps to 0 amd double jumped to false to reset the double jump
	NumberOfJumps = 0;
	DoubleJumped = false;
	TurnOffJumpOffWall();
}

/// <summary>
/// Checks the jump to see if the player is jumping normally or jumping off a wall
/// </summary>
void ASkylineShredderCharacter::CheckJump()
{
	//if currently jumping, set it to false
	if (_jumping)
	{
		_jumping = false;
	}
	//If the player presses jump and is not currently jumping
	else
	{
		//Set jumping to true and add one to the number of jumps variable
		_jumping = true;
		NumberOfJumps++;
		//If the player is able to double jump and is not grappling
		if (NumberOfJumps == 2 && GetCharacterMovement()->IsFalling() && !GrappleHookAttached)
		{	
			//Double jump
			DoubleJumped = true;

			//Get the players current velocity and then set it to 0
			FVector currentVelocity = GetCharacterMovement()->Velocity;
			currentVelocity.Z = 0.0f;
			GetCharacterMovement()->Velocity = FVector{ 0.0f, 0.0f, 0.0f };

			//If the player is not moving then the double jump is straight up
			if (GetInputAxisValue("MoveForward") == 0.0f && GetInputAxisValue("MoveRight") == 0.0f)
				GetCharacterMovement()->AddImpulse(FVector{ 0.0f, 0.0f, 150000.0f });
			//If the player is moving
			else
			{
				float newXForward = GetVelocity().GetSafeNormal().X;
				float newYForward = GetVelocity().GetSafeNormal().Y;
				//Make the player double jump in the direction they are facing, with respect to momentum and the original velocity
				FVector doubleJumpForce = FVector{ (GetActorForwardVector().X + newXForward) * (50000.0f + GetMomentum() * 50.0f), (GetActorForwardVector().Y + newYForward) * (50000.0f + GetMomentum() * 50.0f), 150000.0f };
				GetCharacterMovement()->AddImpulse(doubleJumpForce + (currentVelocity * 50.0f));
			}
				
		}
	}


	//If the player is wall running
	if (IsWallRunning)
	{
		//Set is wall running to be false and is jumping off wall to be true
		IsWallRunning = false;
		_isJumpingOffWall = true;

		//Get the right vector and select if the player launches to the right or the left
		//based off if the player is on the right side of a wall or not
		FVector actorRightVector = GetActorRightVector();
		FVector launchVelocity = UKismetMathLibrary::SelectVector(actorRightVector * (- 450.0f - (GetMomentum() / 10.0f)), actorRightVector * (450.0f + (GetMomentum() / 10.0f)), _onRightSide);

		//Set the launch velocity z to be higher
		launchVelocity.Z = 850.0f + (GetMomentum() / 10.0f);

		//Launch the character using the launch velocity
		LaunchCharacter(launchVelocity, false, false); 

		NumberOfJumps = 1;

		//Set a timer to call the turn off wall run function
		GetWorldTimerManager().SetTimer(timerHandle, this, &ASkylineShredderCharacter::TurnOffJumpOffWall, .5f, false);
	}
}

/// <summary>
/// The custom jump function for the player
/// </summary>
void ASkylineShredderCharacter::CustomJump()
{
	//If the player can jump
	if (_jumping && GetCharacterMovement()->IsMovingOnGround()) {
		//Add impulse to the player in the direction they are facing with respect to momentum
		float newXForward = GetVelocity().GetSafeNormal().X;
		float newYForward = GetVelocity().GetSafeNormal().Y;
		GetCharacterMovement()->AddImpulse(FVector{ (GetActorForwardVector().X + newXForward) * (10000.0f + GetMomentum() * 10.0f), (GetActorForwardVector().Y + newYForward) * (10000.0f + GetMomentum() * 10.0f), 125000.0f });
	}
}

//This Function checks to see if the player is able to do a grapple by using a sphere trace to find if a grapple point is in range and is called when left click has been pressed
void ASkylineShredderCharacter::CheckForGrapple()
{
	//Initialize variables
	bool bHit = false;
	FHitResult HitResult;

	//If already in action return
	if (InAction)
		return;

	//Checks to see if the player not currently on the ground and that a grapple is not already attached to a point
	if (!GetCharacterMovement()->GetCharacterOwner()->GetMovementComponent()->IsMovingOnGround() && !GrappleHookAttached) {

		//Sets the start location of the trace to be the actors location
		FVector Start = GetActorLocation();

		//The end point is the actors location and the camera direction being multiplied by a value
		FVector End = Start + (UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0)->GetCameraRotation().Vector() * 6000.0f);

		//The raidus of the sphere
		float radius = 2500.0f;

		//Make an array of actors to ignore
		TArray<AActor*> ActorsToIgnore;

		//ingore the character during the check
		ActorsToIgnore.Add(this);

		//Establish an array of what object type to search for
		TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypesArray;

		//Set the object to looked for in the array in this case it is Game trace channel 2 because that it where the collision to check for is happening
		ObjectTypesArray.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_GameTraceChannel2));

		//Perform the sphere trace in this case it is searching for the specific object 
		bHit = UKismetSystemLibrary::SphereTraceSingleForObjects(GetWorld(), Start, End, radius, ObjectTypesArray,
			false, ActorsToIgnore, EDrawDebugTrace::None, HitResult, true);
		//if the sphere trace has hit...
		if (bHit) {
			//...Set the grapple hook to be attached
			GrappleHookAttached = true;
			//Set the location of the where the hand model will be placed
			HookLocation = HitResult.ImpactPoint;
		}
	}
	else
	{
		//else set grapple to be false
		GrappleHookAttached = false;
	}
	
}

//this function called when the player is using the grapple hook and calculates the logic of the grapple and swinging perameters
void ASkylineShredderCharacter::DoGrapple()
{
	//Calculate character and hook direction and the direction between them
	FVector CharacterLocation = GetActorLocation();
	FVector HookDirection = (HookLocation - CharacterLocation).GetSafeNormal();

	//Calculate the direction of the swing perpendicular to the hook direction
	FVector SwingDirection = FVector::CrossProduct(HookDirection, FVector::UpVector).GetSafeNormal();

	//Calculate the speed of the swing
	float SwingSpeed = FVector::DotProduct(GetVelocity(), SwingDirection);

	//Calculate the swing velocity vector
	FVector SwingVelocity = SwingSpeed * SwingDirection;

	//Calculate the direction of radial velocity perpendicular to both the hook and swinging directions
	FVector RadialDirection = FVector::CrossProduct(SwingDirection, HookDirection).GetSafeNormal();

	//Calculate the speed of radial velocity
	float RadialSpeed = FVector::DotProduct(GetVelocity(), RadialDirection);

	//Calculate the radial velocity vector
	FVector RadialVelocity = RadialSpeed * RadialDirection;

	//Calculate the total velocity vector by combining the swing and radial velocity vectors
	FVector TotalVelocity = SwingVelocity + RadialVelocity;

	//Set the characters velocity to the total velocity vector
	GetCharacterMovement()->Velocity = TotalVelocity;

	//Check if the player is touching the ground
	if (GetCharacterMovement()->IsMovingOnGround())
	{
		//If the player is touching the ground, end the grapple
		EndGrapple();
	}
	
}

//This function is called to end the grappling proccess and is called when left click has been released
void ASkylineShredderCharacter::EndGrapple()
{
	//force variable created to apply a force once the grapple has been let go
	FVector Force = GetActorForwardVector() * 200000.0f;

	//If the grapple hook is not attached do nothing
	if (!GrappleHookAttached) {
		return;
	}

	//set the grapple hook to be not attached and reset the boost state
	GrappleHookAttached = false;
	HasAppliedBoost = false;
	
	_momentum += 300.0f;

	//Apply the calculated impulse to the character
	GetCharacterMovement()->AddImpulse(FVector(Force.X, Force.Y, 450.0f));
	
}

//this function is called to give the player the ability to apply an impulse once per grapple it is called when right click is pressed during the grapple
void ASkylineShredderCharacter::GrappleBoost()
{
	//check to see if the grapple hook is attached and the player is not on the ground
	if (GrappleHookAttached && !GetCharacterMovement()->IsMovingOnGround()) {
		//if the boost not been applied yet
		if (!HasAppliedBoost) {
			//calculate the boost force
			FVector BoostForce = GetActorForwardVector() * 150000;
			//apply the boost to the character
			GetCharacterMovement()->AddImpulse(FVector(BoostForce.X, BoostForce.Y, 0.0f));
			//set the boost state to applied
			HasAppliedBoost = true;
			LastBoostTime = GetWorld()->TimeSeconds;
			
			
		}
	}
}

////Unused dash mechanic
//void ASkylineShredderCharacter::StartDash()
//{
//	if (!IsDashing)
//	{
//		IsDashing = true;
//		DashTimeRemaining = DashDuration;
//
//		FVector DashDirection = GetActorForwardVector();
//		FVector DashDestination = GetActorLocation() + DashDirection * DashDistance;
//
//		UGameplayStatics::GetPlayerController(GetWorld(), 0)->SetIgnoreLookInput(true);
//
//		FHitResult HitResult;
//		SetActorLocation(DashDestination, true, &HitResult);
//
//		if (HitResult.bBlockingHit)
//		{
//			// If we hit something during the dash, stop dashing
//			IsDashing = false;
//			DashTimeRemaining = 0.0f;
//			UGameplayStatics::GetPlayerController(GetWorld(), 0)->SetIgnoreLookInput(false);
//		}
//	}
//}

/// <summary>
/// Ends the wall running functionality
/// </summary>
void ASkylineShredderCharacter::TurnOffJumpOffWall()
{
	//Set the booleans to be false
	_isJumpingOffWall = false;
	InAction = false;
	//Set the gravity scale and plane constraints back to normal
	GetCharacterMovement()->GravityScale = 1.0f;
	GetCharacterMovement()->SetPlaneConstraintNormal(FVector(0.0f, 0.0f, 0.0f));
}

/// <summary>
/// Turns double jumped to false
/// </summary>
void ASkylineShredderCharacter::DoubleJumpOff()
{
	DoubleJumped = false;
}

void ASkylineShredderCharacter::OnResetVR()
{
	// If TestComplexSystem is added to a project via 'Add Feature' in the Unreal Editor the dependency on HeadMountedDisplay in TestComplexSystem.Build.cs is not automatically propagated
	// and a linker error will result.
	// You will need to either:
	//		Add "HeadMountedDisplay" to [YourProject].Build.cs PublicDependencyModuleNames in order to build successfully (appropriate if supporting VR).
	// or:
	//		Comment or delete the call to ResetOrientationAndPosition below (appropriate if not supporting VR)
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void ASkylineShredderCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
	Jump();
}

void ASkylineShredderCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
	StopJumping();
}

void ASkylineShredderCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void ASkylineShredderCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void ASkylineShredderCharacter::MoveForward(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void ASkylineShredderCharacter::MoveRight(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}
