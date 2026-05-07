// Copyright Epic Games, Inc. All Rights Reserved.

#include "YeongCarCarPawn.h"
#include "Base/Car/YeongCarCarWheelFront.h"
#include "Base/Car/YeongCarCarWheelRear.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "TimerManager.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/Texture2D.h"
#include "Base/Component/SplineFollowerComponent.h"
#include "Camera/CameraSensorComponent.h"
#include "Sensor/LidarSensorComponent.h"
#include "DataLogger/YCCAgentDataLoggerComponent.h"
#include "System/YCCPlayerController.h"

#define LOCTEXT_NAMESPACE "VehiclePawn"

DEFINE_LOG_CATEGORY(LogTemplateVehicle);

AYeongCarCarPawn::AYeongCarCarPawn()
{
	FrontSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Front Spring Arm"));
	FrontSpringArm->SetupAttachment(GetMesh());
	FrontSpringArm->TargetArmLength = 0.0f;
	FrontSpringArm->bDoCollisionTest = false;
	FrontSpringArm->bEnableCameraRotationLag = true;
	FrontSpringArm->CameraRotationLagSpeed = 15.0f;
	FrontSpringArm->SetRelativeLocation(FVector(30.0f, 0.0f, 120.0f));

	FrontCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Front Camera"));
	FrontCamera->SetupAttachment(FrontSpringArm);
	FrontCamera->bAutoActivate = false;

	BackSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Back Spring Arm"));
	BackSpringArm->SetupAttachment(GetMesh());
	BackSpringArm->TargetArmLength = 650.0f;
	BackSpringArm->SocketOffset.Z = 150.0f;
	BackSpringArm->bDoCollisionTest = false;
	BackSpringArm->bInheritPitch = false;
	BackSpringArm->bInheritRoll = false;
	BackSpringArm->bEnableCameraRotationLag = true;
	BackSpringArm->CameraRotationLagSpeed = 2.0f;
	BackSpringArm->CameraLagMaxDistance = 50.0f;

	BackCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Back Camera"));
	BackCamera->SetupAttachment(BackSpringArm);

	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetCollisionProfileName(FName("Vehicle"));

	ChaosVehicleMovement = CastChecked<UChaosWheeledVehicleMovementComponent>(GetVehicleMovement());

	CameraSensor = CreateDefaultSubobject<UCameraSensorComponent>(TEXT("CameraSensor"));
	CameraSensor->SetupAttachment(GetMesh());

	LidarSensor = CreateDefaultSubobject<ULidarSensorComponent>(TEXT("LidarSensor"));
	LidarSensor->SetupAttachment(GetMesh());
	LidarSensor->SetRelativeLocation(FVector(0.0f, 0.0f, 180.0f));

	SplineFollower = CreateDefaultSubobject<USplineFollowerComponent>(TEXT("SplineFollower"));

	DataLogger = CreateDefaultSubobject<UYCCAgentDataLoggerComponent>(TEXT("DataLogger"));
}

void AYeongCarCarPawn::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(SteeringAction, ETriggerEvent::Triggered, this, &AYeongCarCarPawn::Steering);
		EnhancedInputComponent->BindAction(SteeringAction, ETriggerEvent::Completed, this, &AYeongCarCarPawn::Steering);

		EnhancedInputComponent->BindAction(ThrottleAction, ETriggerEvent::Triggered, this, &AYeongCarCarPawn::Throttle);
		EnhancedInputComponent->BindAction(ThrottleAction, ETriggerEvent::Completed, this, &AYeongCarCarPawn::Throttle);

		EnhancedInputComponent->BindAction(BrakeAction, ETriggerEvent::Triggered, this, &AYeongCarCarPawn::Brake);
		EnhancedInputComponent->BindAction(BrakeAction, ETriggerEvent::Started, this, &AYeongCarCarPawn::StartBrake);
		EnhancedInputComponent->BindAction(BrakeAction, ETriggerEvent::Completed, this, &AYeongCarCarPawn::StopBrake);

		EnhancedInputComponent->BindAction(HandbrakeAction, ETriggerEvent::Started, this, &AYeongCarCarPawn::StartHandbrake);
		EnhancedInputComponent->BindAction(HandbrakeAction, ETriggerEvent::Completed, this, &AYeongCarCarPawn::StopHandbrake);

		EnhancedInputComponent->BindAction(LookAroundAction, ETriggerEvent::Triggered, this, &AYeongCarCarPawn::LookAround);

		EnhancedInputComponent->BindAction(ToggleCameraAction, ETriggerEvent::Triggered, this, &AYeongCarCarPawn::ToggleCamera);

		EnhancedInputComponent->BindAction(ResetVehicleAction, ETriggerEvent::Triggered, this, &AYeongCarCarPawn::ResetVehicle);

		EnhancedInputComponent->BindAction(ToggleCameraViewAction, ETriggerEvent::Started, this, &AYeongCarCarPawn::ToggleSensorView);
		EnhancedInputComponent->BindAction(ToggleLidarViewAction, ETriggerEvent::Started, this, &AYeongCarCarPawn::ToggleLidarView);
	}
	else
	{
		UE_LOG(LogTemplateVehicle, Error, TEXT("'%s' Failed to find an Enhanced Input component!"), *GetNameSafe(this));
	}
}

void AYeongCarCarPawn::BeginPlay()
{
	Super::BeginPlay();

	GetWorld()->GetTimerManager().SetTimer(FlipCheckTimer, this, &AYeongCarCarPawn::FlippedCheck, FlipCheckTime, true);
}

void AYeongCarCarPawn::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	GetWorld()->GetTimerManager().ClearTimer(FlipCheckTimer);

	Super::EndPlay(EndPlayReason);
}

void AYeongCarCarPawn::Tick(float Delta)
{
	Super::Tick(Delta);

	bool bMovingOnGround = ChaosVehicleMovement->IsMovingOnGround();
	GetMesh()->SetAngularDamping(bMovingOnGround ? 0.0f : 3.0f);

	float CameraYaw = BackSpringArm->GetRelativeRotation().Yaw;
	CameraYaw = FMath::FInterpTo(CameraYaw, 0.0f, Delta, 1.0f);

	BackSpringArm->SetRelativeRotation(FRotator(0.0f, CameraYaw, 0.0f));
}

void AYeongCarCarPawn::Steering(const FInputActionValue& Value)
{
	DoSteering(Value.Get<float>());
}

void AYeongCarCarPawn::Throttle(const FInputActionValue& Value)
{
	DoThrottle(Value.Get<float>());
}

void AYeongCarCarPawn::Brake(const FInputActionValue& Value)
{
	DoBrake(Value.Get<float>());
}

void AYeongCarCarPawn::StartBrake(const FInputActionValue& Value)
{
	DoBrakeStart();
}

void AYeongCarCarPawn::StopBrake(const FInputActionValue& Value)
{
	DoBrakeStop();
}

void AYeongCarCarPawn::StartHandbrake(const FInputActionValue& Value)
{
	DoHandbrakeStart();
}

void AYeongCarCarPawn::StopHandbrake(const FInputActionValue& Value)
{
	DoHandbrakeStop();
}

void AYeongCarCarPawn::LookAround(const FInputActionValue& Value)
{
	DoLookAround(Value.Get<float>());
}

void AYeongCarCarPawn::ToggleCamera(const FInputActionValue& Value)
{
	DoToggleCamera();
}

void AYeongCarCarPawn::ResetVehicle(const FInputActionValue& Value)
{
	DoResetVehicle();
}

void AYeongCarCarPawn::ToggleSensorView(const FInputActionValue& Value)
{
	DoToggleSensorView();
}

void AYeongCarCarPawn::ToggleLidarView(const FInputActionValue& Value)
{
	DoToggleLidarView();
}

void AYeongCarCarPawn::DoSteering(float SteeringValue)
{
	ChaosVehicleMovement->SetSteeringInput(SteeringValue);
}

void AYeongCarCarPawn::DoThrottle(float ThrottleValue)
{
	ChaosVehicleMovement->SetThrottleInput(ThrottleValue);
	ChaosVehicleMovement->SetBrakeInput(0.0f);
}

void AYeongCarCarPawn::DoBrake(float BrakeValue)
{
	ChaosVehicleMovement->SetBrakeInput(BrakeValue);
	ChaosVehicleMovement->SetThrottleInput(0.0f);
	BrakeLights(BrakeValue > 0.05f);
}

void AYeongCarCarPawn::DoBrakeStart()
{
	BrakeLights(true);
}

void AYeongCarCarPawn::DoBrakeStop()
{
	BrakeLights(false);
	ChaosVehicleMovement->SetBrakeInput(0.0f);
}

void AYeongCarCarPawn::DoHandbrakeStart()
{
	ChaosVehicleMovement->SetHandbrakeInput(true);
	BrakeLights(true);
}

void AYeongCarCarPawn::DoHandbrakeStop()
{
	ChaosVehicleMovement->SetHandbrakeInput(false);
	BrakeLights(false);
}

void AYeongCarCarPawn::DoLookAround(float YawDelta)
{
	BackSpringArm->AddLocalRotation(FRotator(0.0f, YawDelta, 0.0f));
}

void AYeongCarCarPawn::DoToggleCamera()
{
	bFrontCameraActive = !bFrontCameraActive;

	FrontCamera->SetActive(bFrontCameraActive);
	BackCamera->SetActive(!bFrontCameraActive);
}

void AYeongCarCarPawn::DoResetVehicle()
{
	FVector ResetLocation = GetActorLocation() + FVector(0.0f, 0.0f, 50.0f);
	FRotator ResetRotation = GetActorRotation();
	ResetRotation.Pitch = 0.0f;
	ResetRotation.Roll = 0.0f;

	SetActorTransform(FTransform(ResetRotation, ResetLocation, FVector::OneVector), false, nullptr, ETeleportType::TeleportPhysics);

	GetMesh()->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
	GetMesh()->SetPhysicsLinearVelocity(FVector::ZeroVector);
}

void AYeongCarCarPawn::FlippedCheck()
{
	const float UpDot = FVector::DotProduct(FVector::UpVector, GetMesh()->GetUpVector());

	if (UpDot < FlipCheckMinDot)
	{
		if (bPreviousFlipCheck)
			DoResetVehicle();

		bPreviousFlipCheck = true;
	}
	else
	{
		bPreviousFlipCheck = false;
	}
}

void AYeongCarCarPawn::DoToggleSensorView()
{
	AYCCPlayerController* PC = Cast<AYCCPlayerController>(GetController());
	if (PC == nullptr)
		return;

	UTextureRenderTarget2D* CamRT = CameraSensor ? CameraSensor->GetRednerTarget() : nullptr;
	PC->ToggleSensorView(CamRT);
}

void AYeongCarCarPawn::DoToggleLidarView()
{
	AYCCPlayerController* PC = Cast<AYCCPlayerController>(GetController());
	if (PC == nullptr)
		return;

	UTexture2D* LidarRT = LidarSensor ? LidarSensor->GetBevRenderTarget() : nullptr;
	PC->ToggleLidarView(LidarRT);

	if (LidarSensor)
	{
		if (PC->IsLidarViewVisible())
			LidarSensor->StartScan();
		else
			LidarSensor->StopScan();
	}
}

#undef LOCTEXT_NAMESPACE
