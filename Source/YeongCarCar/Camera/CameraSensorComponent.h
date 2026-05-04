// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Camera/CameraType.h"
#include "CameraSensorComponent.generated.h"



class USceneCaptureComponent2D;
class UTextureRenderTarget2D;	
UCLASS(ClassGroup = (Sensor), meta = (BlueprintSpawnableComponent), BlueprintType)



class YEONGCARCAR_API UCameraSensorComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UCameraSensorComponent();

	UTextureRenderTarget2D* GetRednerTarget() const { return RenderTarget; }
	
protected:
	virtual void BeginPlay() override;
	virtual void OnRegister() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	
private:

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	
	UFUNCTION(BlueprintCallable, Category = "CameraSensor")
	void ApplyPreset(ECameraSensorPreset NewPreset);
	
	UFUNCTION(BlueprintCallable, Category = "CameraSensor")
	void RefreshSettings();
	
	UFUNCTION(BlueprintCallable, Category = "CameraSensor")
	void CaptureOnce();
	
	UFUNCTION(BlueprintCallable, Category = "CameraSensor")
	void SetCaptureRate(float Hz);
	
	void InitializeCapture();
	void ConfigureSceneCapture();
	void CreateRenderTarget();
	void StopCaptureTimer();
	void ApplyPostProcessSettings();
	void ApplyLensDistortion();
	void StartCaptureTimer();
	void OnCaptureTimer();
	
	void SaveCameraImage();
	
	
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CameraSensor|Output",
	meta=(AllowPrivateAccess="true"))
	TObjectPtr<UTextureRenderTarget2D> RenderTarget;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSensor|Config",
	meta=(AllowPrivateAccess="true"))
	ECameraSensorPreset Preset = ECameraSensorPreset::TeslaHW3_Wide;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSensor|Intrinsics",
	meta=(AllowPrivateAccess="true"))
	FCameraSensorIntrinsics Intrinsics;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSensor|Distortion",
		meta=(AllowPrivateAccess="true"))
	FLensDistortionParams Distortion;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSensor|Noise",
		meta=(AllowPrivateAccess="true"))
	FSensorNoiseParams Noise;

	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSensor|PostProcess",
		meta=(AllowPrivateAccess="true"))
	FCameraPostProcessEffects PostProcess;

	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSensor|Exposure",
		meta=(AllowPrivateAccess="true"))
	FAutoExposureParams Exposure;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSensor|Config",
	meta=(AllowPrivateAccess="true"))
	bool bSensorEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSensor|DataSave",
		meta=(AllowPrivateAccess="true"))
	bool bIsDataSaving = false;
	
	//이미지를 저장할 때 필요한 정보들을 담고 있는 구조체
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSensor|DataSave",
	meta=(AllowPrivateAccess="true"))
	FSensorDataSaveConfig DataSaveConfig;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CameraSensor|Output",
	meta=(AllowPrivateAccess="true"))
	int64 FrameCount;
	//모든 타입의 머테리얼을 담을수있는 상자
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSensor|Distortion",
		meta=(AllowPrivateAccess="true"))
	TObjectPtr<UMaterialInterface> LensDistortionMaterial;
	// 동적 머테리얼 인스턴스를 담기위한 상자
	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> DistortionMID;

	
	// 이미지 생성용 카메라 
	UPROPERTY()
	TObjectPtr<USceneCaptureComponent2D> SceneCapture;
	
	FTimerHandle CaptureTimerHandle;
};
