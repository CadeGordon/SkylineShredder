// Copyright Epic Games, Inc. All Rights Reserved.

#include "SkylineShredderGameMode.h"
#include "SkylineShredderCharacter.h"
#include "UObject/ConstructorHelpers.h"

ASkylineShredderGameMode::ASkylineShredderGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
