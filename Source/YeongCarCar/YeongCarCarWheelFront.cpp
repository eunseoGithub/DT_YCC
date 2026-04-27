// Copyright Epic Games, Inc. All Rights Reserved.

#include "YeongCarCarWheelFront.h"
#include "UObject/ConstructorHelpers.h"

UYeongCarCarWheelFront::UYeongCarCarWheelFront()
{
	AxleType = EAxleType::Front;
	bAffectedBySteering = true;
	MaxSteerAngle = 40.f;
}