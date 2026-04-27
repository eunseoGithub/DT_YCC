// Copyright Epic Games, Inc. All Rights Reserved.

#include "YeongCarCarGameMode.h"
#include "Base/Player/YeongCarCarPlayerController.h"

AYeongCarCarGameMode::AYeongCarCarGameMode()
{
	PlayerControllerClass = AYeongCarCarPlayerController::StaticClass();
}
