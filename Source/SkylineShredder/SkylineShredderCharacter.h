// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SkylineShredderCharacter.generated.h"

UCLASS(config=Game)
class ASkylineShredderCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class UCameraComponent* FollowCamera;


private:
	//Variables for setting up timers
	FTimerHandle TimerHandle;
	float _delayTimer;
	ETraceTypeQuery ObjectType{};

	//Axis variables
	float axisForward;
	float axisRight;
public:
	ASkylineShredderCharacter();

	/// <summary>
	/// Update which handles all logic for the player
	/// </summary>
	/// <param name="deltaTime"></param>
	virtual void Tick(float deltaTime) override;

	/// <summary>
	/// When the player lands on the ground
	/// </summary>
	/// <param name="Hit"></param>
	virtual void Landed(const FHitResult& Hit) override;

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
		float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
		float BaseLookUpRate;

	//Booleans used for animations in the blueprint and to check if the player 
	//is in a certain action or not
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Parkour)
		bool IsSprinting = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Parkour)
		bool IsSliding = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Parkour)
		bool IsClimbing = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Parkour)
		bool IsCrouching = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Parkour)
		bool IsVaulting = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadwrite, Category = Parkour)
		bool InAction = false;

public:
	//Variable used for checking for climbing
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Parkour)
		bool ShouldPlayerClimb;

	//Variables used for wall running
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Parkour)
		bool IsWallRunning;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Parkour)
		bool LeftSide;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Parkour)
		bool RightSide;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Parkour)
		bool DoubleJumped;

	//Variables for Grappling
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Grapple)
	bool GrappleHookAttached = false;
	float GrappleHookLength = 3000.0f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Grapple)
	FVector HookLocation;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Grapple)
	bool HasAppliedBoost;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Grapple)
	AActor* HookHitActor;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Grapple)
	float LastBoostTime = 0.0f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Grapple)
	float BoostCooldownTime = 2.0f;
	float ForceMultiplier = 100.0f;
	float GrappleImpulseMultiplier = 100.0f;

	////Unused code for dashing
	//float DashDistance = 1000.0f;
	//float DashDuration = 0.2f;
	//float DashTimeRemaining;
	//bool IsDashing;
	
	//The base speed of the player
	float BaseSpeed;

	//The number of jumps the player is currently at
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Parkour)
	int NumberOfJumps;
	

private:
	//Variables used for climbing and or vaulting
	bool _isWallThick;
	bool _canClimb;
	FVector _wallLocation;
	FVector _wallNormal;
	FVector _wallHeight;
	FVector _otherWallHeight;

	//Momentum variable which affects the speed of the player
	float _momentum;

	//Variables used for checking the height of the player frame by frame
	float _lastFrameHeight;
	float _currentFrameHeight;

	//Variables used for wall running
	bool _onRightSide;
	bool _isJumpingOffWall;
	bool _isJumping;

	//If the player is jumping
	bool _jumping;
	
	//The amount of gravity on the player
	float _gravity;

	/// <summary>
	/// Turns off the variables necessary for jumping off a wall
	/// </summary>
	UFUNCTION()
	void TurnOffJumpOffWall();

	//The timer handle
	FTimerHandle timerHandle;

	/// <summary>
	/// Turns off the variables necessary for double jumping
	/// </summary>
	void DoubleJumpOff();

protected:

	/** Resets HMD orientation in VR. */
	void OnResetVR();

	/** Called for forwards/backward input */
	void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);

	/**
	 * Called via input to turn at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	/** Handler for when a touch input begins. */
	void TouchStarted(ETouchIndex::Type FingerIndex, FVector Location);

	/** Handler for when a touch input stops. */
	void TouchStopped(ETouchIndex::Type FingerIndex, FVector Location);

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End of APawn interface

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	//These functions and variables can be called in blueprint in case the user would like to change when they are used
	UFUNCTION(BlueprintCallable, Category = "Parkour")
		void SetMoveSpeed(float speed);

	UFUNCTION(BlueprintCallable, Category = "Parkour")
		bool CheckForClimbing();

	UFUNCTION(BlueprintCallable, Category = "Parkour")
		void StartVaultOrGetUp();

	UFUNCTION(BlueprintCallable, Category = "Parkour")
		void StopVaultOrGetUp();

	UFUNCTION(BlueprintCallable, Category = "Parkour")
		void CheckForWallRunning();

	UFUNCTION(BlueprintCallable, Category = "Parkour")
		void CheckJump();

	UFUNCTION(BlueprintCallable, Category = "Parkour")
		void CustomJump();

	UFUNCTION(BlueprintCallable, Category = "Grapple")
		void CheckForGrapple();

	UFUNCTION(BlueprintCallable, Category = "Grapple")
		void DoGrapple();

	UFUNCTION(BlueprintCallable, Category = "Grapple")
		void EndGrapple();

	UFUNCTION(BlueprintCallable, Category = "Grapple")
		void GrappleBoost();

	UFUNCTION(BlueprintCallable, Category = "Parkour")
	float GetMomentum() { return _momentum; }

	UFUNCTION(BlueprintCallable, Category = "Parkour")
	void SetMomentum(float momentum) { _momentum = momentum; }

	/*UFUNCTION(BlueprintCallable, Category = "Dash")
	void StartDash();*/
	
};

