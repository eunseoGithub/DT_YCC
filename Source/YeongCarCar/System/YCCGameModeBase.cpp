#include "YCCGameModeBase.h"
#include "YCCPlayerController.h"

AYCCGameModeBase::AYCCGameModeBase()
{
	PlayerControllerClass = AYCCPlayerController::StaticClass();
}
