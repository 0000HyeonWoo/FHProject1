// Copyright Epic Games, Inc. All Rights Reserved.

#include "FHProjectGameMode.h"
#include "FHProjectCharacter.h"
#include "UObject/ConstructorHelpers.h"

AFHProjectGameMode::AFHProjectGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
