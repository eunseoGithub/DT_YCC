#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "YCCPlayerController.generated.h"

class AYeongCarCarPawn;
class UInputMappingContext;

UCLASS()
class YEONGCARCAR_API AYCCPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	void ToggleSensorView(UTextureRenderTarget2D* InCameraRT);
	void ToggleLidarView(UTexture2D* InLidarRT);
	bool IsLidarViewVisible() const;
protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void OnPossess(APawn* InPawn) override;
	
	UFUNCTION()
	void OnPawnDestroyed(AActor* DestroyedPawn);
private:
	UPROPERTY(EditAnywhere, Category = "Input|IMC")
	TArray<UInputMappingContext*> DefaultMappingContexts;
	
	UPROPERTY(EditAnywhere, Category = "Input|IMC")
	TArray<UInputMappingContext*> MobileExcludedMappingContexts;
	
	UPROPERTY(EditAnywhere, Category = "Input|Touch Controls")
	TSubclassOf<UUserWidget> MobileControlsWidgetClass;
	
	UPROPERTY(EditAnywhere, Category = "Input|Steering Wheel Controls", meta = (EditCondition = "bUseSteeringWheelControls"))
	TObjectPtr<UInputMappingContext> SteeringWheelInputMappingContext;
	
	UPROPERTY(EditAnywhere, Category = "Vehicle|Respawn")
	TSubclassOf<AYeongCarCarPawn> VehiclePawnClass;
	
	// UPROPERTY(EditAnywhere, Category="Vehicle|UI")
	// TSubclassOf<USensorViewWidget> SensorViewWidgetClass;
	
	UPROPERTY()
	TObjectPtr<AYeongCarCarPawn> VehiclePawn;
	
	UPROPERTY()
	TObjectPtr<UUserWidget> MobileControlWidget;
	
	// UPROPERTY()
	// TObjectPtr<UYCCSensorViewWidget> SensorViewWidget;
};
