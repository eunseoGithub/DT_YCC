// Copyright Epic Games, Inc. All Rights Reserved.

#include "YeongCarCarGameMode.h"
#include "YeongCarCarPlayerController.h"

AYeongCarCarGameMode::AYeongCarCarGameMode()
{
	PlayerControllerClass = AYeongCarCarPlayerController::StaticClass();
}
