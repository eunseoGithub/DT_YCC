#include "YCCPlayerController.h"

#include "EnhancedInputSubsystems.h"
#include "Base/Car/YeongCarCarPawn.h"
#include "Components/SlateWrapperTypes.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "UI/YCCSensorViewWidget.h"

void AYCCPlayerController::ToggleSensorView(UTextureRenderTarget2D* InCameraRT)
{
	if (!SensorViewWidget) return;

	if (InCameraRT)
	{
		SensorViewWidget->SetRenderTarget(InCameraRT);
	}
	SensorViewWidget->ToggleCameraView();
}

void AYCCPlayerController::ToggleLidarView(UTexture2D* InLidarRT)
{
	if (!SensorViewWidget) return;

	if (InLidarRT)
	{
		SensorViewWidget->SetLidarRenderTarget(InLidarRT);
	}
	SensorViewWidget->ToggleLidarView();
}

bool AYCCPlayerController::IsLidarViewVisible() const
{
	return SensorViewWidget && SensorViewWidget->IsLidarViewVisible();
}

void AYCCPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	bAttachToPawn = true;

	if (SensorViewWidgetClass)
	{
		SensorViewWidget = CreateWidget<UYCCSensorViewWidget>(this, SensorViewWidgetClass);
		if (SensorViewWidget)
		{
			SensorViewWidget->AddToViewport(10);
			SensorViewWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		}
	}
}

void AYCCPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	
	if (IsLocalPlayerController())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
			{
				Subsystem->AddMappingContext(CurrentContext, 0);
			}
		}
	}
}

void AYCCPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	VehiclePawn = CastChecked<AYeongCarCarPawn>(InPawn);
	VehiclePawn->OnDestroyed.AddDynamic(this, &AYCCPlayerController::OnPawnDestroyed);
}

void AYCCPlayerController::OnPawnDestroyed(AActor* DestroyedPawn)
{
	TArray<AActor*> ActorList;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), ActorList);
	
	if (ActorList.Num() > 0)
	{
		const FTransform SpawnTransform = ActorList[0]->GetActorTransform();
		
		if (AYeongCarCarPawn* RespawnVehicle = GetWorld()->SpawnActor<AYeongCarCarPawn>(VehiclePawnClass, SpawnTransform))
			Possess(RespawnVehicle);
	}
}
