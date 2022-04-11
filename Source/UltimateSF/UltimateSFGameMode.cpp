// Copyright Epic Games, Inc. All Rights Reserved.

#include "UltimateSFGameMode.h"
#include "UltimateSFCharacter.h"
#include "UObject/ConstructorHelpers.h"

AUltimateSFGameMode::AUltimateSFGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
