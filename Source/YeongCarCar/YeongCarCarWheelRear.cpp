// Copyright Epic Games, Inc. All Rights Reserved.

#include "YeongCarCarWheelRear.h"
#include "UObject/ConstructorHelpers.h"

UYeongCarCarWheelRear::UYeongCarCarWheelRear()
{
	AxleType = EAxleType::Rear;
	bAffectedByHandbrake = true;
	bAffectedByEngine = true;
}