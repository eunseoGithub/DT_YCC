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

    //생성된 렌더 타겟을 외부(UI 등)에서 참조하기 위한 Getter
    UTextureRenderTarget2D* GetRednerTarget() const { return RenderTarget; }
    
protected:
    virtual void BeginPlay() override;
    virtual void OnRegister() override;   // 컴포넌트 등록 시 SceneCapture 연결 확인
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override; // 타이머 정리
    
private:
#if WITH_EDITOR
    // 에디터에서 디테일 패널의 수치가 바뀔 때 프리셋 등을 즉시 갱신 
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
    
    /// 미리 정의된 카메라 사양(Tesla, Waymo 등)을 적용 
    UFUNCTION(BlueprintCallable, Category = "CameraSensor")
    void ApplyPreset(ECameraSensorPreset NewPreset);
    
    //바뀐 설정값들을 실제 렌더 타겟과 씬 캡처에 다시 반영 
    UFUNCTION(BlueprintCallable, Category = "CameraSensor")
    void RefreshSettings();
    
    // 타이머와 별개로 즉시 한 프레임을 캡처 
    UFUNCTION(BlueprintCallable, Category = "CameraSensor")
    void CaptureOnce();
    
    /** 초당 캡처 횟수(Hz)를 변경하고 타이머 재시작 */
    UFUNCTION(BlueprintCallable, Category = "CameraSensor")
    void SetCaptureRate(float Hz);
    
    // --- 내부 초기화 및 처리 함수 ---
    void InitializeCapture();       // 전체 시스템 초기설정 실행
    void ConfigureSceneCapture();   // 씬 캡처 컴포넌트의 화각, 소스 등 설정
    void CreateRenderTarget();      // 메모리에 이미지 결과물을 담을 공간 생성
    void StopCaptureTimer();        // 가동 중인 캡처 타이머 중지
    void ApplyPostProcessSettings();// 노출, 비네팅 등 후처리 효과 적용
    void ApplyLensDistortion();     // 머티리얼을 이용한 광각 렌즈 왜곡 적용
    void StartCaptureTimer();       // 설정된 FrameRate 주기로 캡처 시작
    void OnCaptureTimer();          // 타이머마다 실행되어 캡처 및 저장 호출
    
    // 렌더 타겟의 데이터를 읽어 JPEG 파일로 디스크 저장 
    //JPEG란 사진과 같은 복잡한 이미지를 효율적으로 압축해서 파일 크기를 줄이기 위해 만든 표준 규격
    //Joint Photographic Experts Group
    void SaveCameraImage();
    
    // --- 속성(Properties) ---

    // 카메라의 최종 결과물이 그려지는 도화지 
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CameraSensor|Output", meta=(AllowPrivateAccess="true"))
    TObjectPtr<UTextureRenderTarget2D> RenderTarget;
    
    // 선택된 카메라 모델 프리셋 (TeslaHW3, Waymo 등) 
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSensor|Config", meta=(AllowPrivateAccess="true"))
    ECameraSensorPreset Preset = ECameraSensorPreset::TeslaHW3_Wide;
    
    // 해상도, 프레임 레이트, FOV 등 물리적 수치 
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSensor|Intrinsics", meta=(AllowPrivateAccess="true"))
    FCameraSensorIntrinsics Intrinsics;

    // K1, K2 등 렌즈 왜곡 계수 
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSensor|Distortion", meta=(AllowPrivateAccess="true"))
    FLensDistortionParams Distortion;

    //센서 노이즈 관련 설정 (현재 로직엔 미포함이나 확장용) 
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSensor|Noise", meta=(AllowPrivateAccess="true"))
    FSensorNoiseParams Noise;

    // 비네팅, 블룸, 색수차 등 렌더링 효과 
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSensor|PostProcess", meta=(AllowPrivateAccess="true"))
    FCameraPostProcessEffects PostProcess;

    // 자동 노출 및 눈 적응 속도 설정
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSensor|Exposure", meta=(AllowPrivateAccess="true"))
    FAutoExposureParams Exposure;
    
    // 센서 작동 여부 
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSensor|Config", meta=(AllowPrivateAccess="true"))
    bool bSensorEnabled = true;

    // 현재 프레임을 파일로 저장할지 여부 
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSensor|DataSave", meta=(AllowPrivateAccess="true"))
    bool bIsDataSaving = false;
    
    // 저장 경로 및 JPG 품질 설정 
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSensor|DataSave", meta=(AllowPrivateAccess="true"))
    FSensorDataSaveConfig DataSaveConfig;
    
    // 저장된 이미지 파일의 번호를 매기기 위한 카운터 
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CameraSensor|Output", meta=(AllowPrivateAccess="true"))
    int64 FrameCount;

    // 왜곡 효과를 계산하는 베이스 머티리얼 (PostProcess용) 
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSensor|Distortion", meta=(AllowPrivateAccess="true"))
    TObjectPtr<UMaterialInterface> LensDistortionMaterial;

    // 실행시간에 K1, K2 값을 머티리얼에 주입하기 위한 인스턴스 
    UPROPERTY()
    TObjectPtr<UMaterialInstanceDynamic> DistortionMID;

    // 실제 렌더링 월드를 캡처하는 핵심 엔진 컴포넌트 
    UPROPERTY()
    TObjectPtr<USceneCaptureComponent2D> SceneCapture;
    
    // 프레임 레이트 유지를 위한 타이머 핸들 
    FTimerHandle CaptureTimerHandle;
};