#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "YCCAgentDataLoggerComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class YEONGCARCAR_API UYCCAgentDataLoggerComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UYCCAgentDataLoggerComponent();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, 
		FActorComponentTickFunction* ThisTickFunction) override;

private:
	static int32 GetUtmZone(double Longitude);
	static void LatLonToUtm(double Lat, double Lon, int32 Zone, double& OutEasting, double& OutNorthing);
	void WorldToUtm(const FVector& WorldLocation, double& OutEasting, double& OutNorthing) const;
	void CreateCsvFile();
	void AppendRow();

	/** 속도(km/h)를 파랑→빨강 색상으로 매핑 */
	FColor SpeedToColor(float SpeedKmh) const;

	UFUNCTION(BlueprintCallable, Category = "Data Logger")
	void StartRecording();

	UFUNCTION(BlueprintCallable, Category = "Data Logger")
	void StopRecording();

	UFUNCTION(BlueprintPure, Category = "Data Logger")
	bool IsRecording() const { return bIsRecording; }

private:
	// ── 기본 설정 ────────────────────────────────────────────────
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data Logger",
		meta = (AllowPrivateAccess = "true"))
	bool bEnableLogging = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data Logger",
		meta = (ClampMin = "0.1", ClampMax = "100.0", Units = "Hz", AllowPrivateAccess = "true"))
	float SaveFrequencyHz = 10.0f;

	// ── UTM 원점 ─────────────────────────────────────────────────
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data Logger|UTM Reference",
		meta = (ClampMin = "-90.0", ClampMax = "90.0", Units = "deg", AllowPrivateAccess = "true"))
	double OriginLatitude = 36.4800;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data Logger|UTM Reference",
		meta = (ClampMin = "-180.0", ClampMax = "180.0", Units = "deg", AllowPrivateAccess = "true"))
	double OriginLongitude = 127.0000;

	// ── 디버그 드로우 설정 ────────────────────────────────────────
	/** DrawDebugLine 궤적 표시 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data Logger|Debug Draw",
		meta = (AllowPrivateAccess = "true"))
	bool bDrawTrajectory = true;

	/** 궤적 선 두께 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data Logger|Debug Draw",
		meta = (AllowPrivateAccess = "true"))
	float TrajectoryLineThickness = 2.0f;

	/** 색상 매핑 기준 최고 속도 (km/h) — 이 속도 이상이면 완전히 빨강 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data Logger|Debug Draw",
		meta = (ClampMin = "1.0", Units = "km/h", AllowPrivateAccess = "true"))
	float MaxSpeedForColor = 100.0f;

	/** 급감속 판정 임계값 (km/h per second) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data Logger|Debug Draw",
		meta = (ClampMin = "0.1", AllowPrivateAccess = "true"))
	float HardBrakeThresholdKmhPerSec = 20.0f;

	/** 급감속 포인트 크기 (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data Logger|Debug Draw",
		meta = (ClampMin = "1.0", AllowPrivateAccess = "true"))
	float HardBrakePointSize = 30.0f;

	/** 속도/yaw 텍스트를 표시하는 거리 간격 (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data Logger|Debug Draw",
		meta = (ClampMin = "100.0", Units = "cm", AllowPrivateAccess = "true"))
	float LabelIntervalCm = 1000.0f;

	/** DrawDebug 선/포인트/텍스트 유지 시간 (초, -1이면 영구) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data Logger|Debug Draw",
		meta = (AllowPrivateAccess = "true"))
	float DebugDrawLifeTime = -1.0f;

private:
	// ── 런타임 상태 ───────────────────────────────────────────────
	double OriginUtmEasting = 0.0;
	double OriginUtmNorthing = 0.0;
	int32  OriginUtmZone = 0;

	FString CsvFilePath;
	bool  bIsRecording = false;
	float TimeSinceLastSave = 0.0f;
	float ElapsedRecordingTime = 0.0f;

	// 이전 프레임 상태 (궤적 선·급감속·가속도 계산용)
	FVector PreviousLocation = FVector::ZeroVector;
	float   PreviousSpeedKmh = 0.0f;
	bool    bHasPreviousFrame = false;

	// 거리 누적 (텍스트 라벨 간격 계산용)
	float AccumulatedDistanceCm = 0.0f;
};
