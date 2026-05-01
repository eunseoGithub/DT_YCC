#include "YCCAgentDataLoggerComponent.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/Pawn.h"

UYCCAgentDataLoggerComponent::UYCCAgentDataLoggerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}

void UYCCAgentDataLoggerComponent::BeginPlay()
{
	Super::BeginPlay();

	OriginUtmZone = GetUtmZone(OriginLongitude);
	LatLonToUtm(OriginLatitude, OriginLongitude, OriginUtmZone, OriginUtmEasting, OriginUtmNorthing);

	if (bEnableLogging)
	{
		StartRecording();
	}
}

void UYCCAgentDataLoggerComponent::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	StopRecording();
	Super::EndPlay(EndPlayReason);
}

void UYCCAgentDataLoggerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bIsRecording)
	{
		return;
	}

	ElapsedRecordingTime += DeltaTime;
	TimeSinceLastSave += DeltaTime;

	const float SaveInterval = 1.0f / FMath::Max(SaveFrequencyHz, 0.1f);
	if (TimeSinceLastSave >= SaveInterval)
	{
		AppendRow();
		TimeSinceLastSave -= SaveInterval;
	}
}

// ─────────────────────────────────────────────────────────────────────────────
//  Recording control
// ─────────────────────────────────────────────────────────────────────────────

void UYCCAgentDataLoggerComponent::StartRecording()
{
	if (bIsRecording)
	{
		return;
	}

	CreateCsvFile();
	bIsRecording = true;
	TimeSinceLastSave = 0.0f;
	ElapsedRecordingTime = 0.0f;
	bHasPreviousFrame = false;
	AccumulatedDistanceCm = 0.0f;
}

void UYCCAgentDataLoggerComponent::StopRecording()
{
	bIsRecording = false;
}

// ─────────────────────────────────────────────────────────────────────────────
//  UTM helpers
// ─────────────────────────────────────────────────────────────────────────────

int32 UYCCAgentDataLoggerComponent::GetUtmZone(double Longitude)
{
	return FMath::FloorToInt((Longitude + 180.0) / 6.0) + 1;
}

void UYCCAgentDataLoggerComponent::LatLonToUtm(double Lat, double Lon, int32 Zone,
	double& OutEasting, double& OutNorthing)
{
	constexpr double a = 6378137.0;
	constexpr double f = 1.0 / 298.257223563;
	constexpr double k0 = 0.9996;

	const double e2 = 2.0 * f - f * f;
	const double ep2 = e2 / (1.0 - e2);

	const double LatRad = FMath::DegreesToRadians(Lat);
	const double CentralMeridian = (Zone - 1) * 6.0 - 180.0 + 3.0;
	const double DeltaLon = FMath::DegreesToRadians(Lon - CentralMeridian);

	const double SinLat = FMath::Sin(LatRad);
	const double CosLat = FMath::Cos(LatRad);
	const double TanLat = FMath::Tan(LatRad);

	const double N = a / FMath::Sqrt(1.0 - e2 * SinLat * SinLat);
	const double T = TanLat * TanLat;
	const double C = ep2 * CosLat * CosLat;
	const double A = CosLat * DeltaLon;

	const double e4 = e2 * e2;
	const double e6 = e4 * e2;
	const double M = a * (
		(1.0 - e2 / 4.0 - 3.0 * e4 / 64.0 - 5.0 * e6 / 256.0) * LatRad
		- (3.0 * e2 / 8.0 + 3.0 * e4 / 32.0 + 45.0 * e6 / 1024.0) * FMath::Sin(2.0 * LatRad)
		+ (15.0 * e4 / 256.0 + 45.0 * e6 / 1024.0) * FMath::Sin(4.0 * LatRad)
		- (35.0 * e6 / 3072.0) * FMath::Sin(6.0 * LatRad));

	const double A2 = A * A;
	const double A4 = A2 * A2;
	const double A6 = A4 * A2;

	OutEasting = k0 * N * (
		A
		+ (1.0 - T + C) * A2 * A / 6.0
		+ (5.0 - 18.0 * T + T * T + 72.0 * C - 58.0 * ep2) * A4 * A / 120.0
		) + 500000.0;

	OutNorthing = k0 * (M + N * TanLat * (
		A2 / 2.0
		+ (5.0 - T + 9.0 * C + 4.0 * C * C) * A4 / 24.0
		+ (61.0 - 58.0 * T + T * T + 600.0 * C - 330.0 * ep2) * A6 / 720.0
		));

	if (Lat < 0.0)
		OutNorthing += 10000000.0;
}

void UYCCAgentDataLoggerComponent::WorldToUtm(const FVector& WorldLocation,
	double& OutEasting, double& OutNorthing) const
{
	const double OffsetEastM = WorldLocation.X * 0.01;
	const double OffsetNorthM = -WorldLocation.Y * 0.01;

	OutEasting = OriginUtmEasting + OffsetEastM;
	OutNorthing = OriginUtmNorthing + OffsetNorthM;
}

// ─────────────────────────────────────────────────────────────────────────────
//  색상 매핑: 0 km/h → 파랑(HSV 240°) / MaxSpeedForColor → 빨강(HSV 0°)
// ─────────────────────────────────────────────────────────────────────────────

FColor UYCCAgentDataLoggerComponent::SpeedToColor(float SpeedKmh) const
{
	const float T = FMath::Clamp(SpeedKmh / FMath::Max(MaxSpeedForColor, 1.0f), 0.0f, 1.0f);

	// 파랑(240°) → 빨강(0°): HSV Hue를 240에서 0으로 선형 감소
	const FLinearColor SlowColor = FLinearColor::MakeFromHSV8(170, 255, 255); // 파랑
	const FLinearColor FastColor = FLinearColor::MakeFromHSV8(0, 255, 255); // 빨강
	const FLinearColor Result = FLinearColor::LerpUsingHSV(SlowColor, FastColor, T);

	return Result.ToFColor(true);
}

// ─────────────────────────────────────────────────────────────────────────────
//  CSV 파일 생성
// ─────────────────────────────────────────────────────────────────────────────

void UYCCAgentDataLoggerComponent::CreateCsvFile()
{
	const FString OutputDir = FPaths::Combine(FPaths::ProjectDir(), TEXT("Output"));
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	PlatformFile.CreateDirectoryTree(*OutputDir);

	const FDateTime Now = FDateTime::Now();
	const FString   FileName = FString::Printf(
		TEXT("AgentData-%04d_%02d-%02d-%02d-%02d-%02d.csv"),
		Now.GetYear(), Now.GetMonth(), Now.GetDay(),
		Now.GetHour(), Now.GetMinute(), Now.GetSecond()
	);

	CsvFilePath = FPaths::Combine(OutputDir, FileName);

	// ── 컬럼 추가: Acceleration_kmh_s (가속도), SteeringInput ────────────
	const FString Header =
		TEXT("Timestamp,World_X,World_Y,World_Z,UTM_Easting,UTM_Northing,UTM_Zone,"
			"Velocity_kmh,Yaw,Acceleration_kmh_s,SteeringInput\n");

	FFileHelper::SaveStringToFile(Header, *CsvFilePath,
		FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);

	UE_LOG(LogTemp, Log, TEXT("[AgentDataLogger] Recording to: %s  (%.1f Hz)"),
		*CsvFilePath, SaveFrequencyHz);
}

// ─────────────────────────────────────────────────────────────────────────────
//  매 기록 주기마다 호출: CSV 한 행 추가 + 디버그 드로우
// ─────────────────────────────────────────────────────────────────────────────

void UYCCAgentDataLoggerComponent::AppendRow()
{
	const AActor* Owner = GetOwner();
	if (!Owner)
		return;

	UWorld* World = GetWorld();

	const FVector  WorldLoc = Owner->GetActorLocation();   // cm
	const FRotator WorldRot = Owner->GetActorRotation();
	const FVector  Velocity = Owner->GetVelocity();        // cm/s

	const float SpeedKmh = static_cast<float>(Velocity.Size() * 0.01 * 3.6);
	const float Yaw = WorldRot.Yaw;

	// ── 가속도: 이전 프레임 대비 속도 변화 (km/h per second) ─────────────
	const float SaveInterval = 1.0f / FMath::Max(SaveFrequencyHz, 0.1f);
	const float AccelerationKmhS = bHasPreviousFrame
		? (SpeedKmh - PreviousSpeedKmh) / SaveInterval
		: 0.0f;

	// ── 조향 입력: Pawn의 스티어링 축 값 읽기 ────────────────────────────
	float SteeringInput = 0.0f;
	if (const APawn* OwnerPawn = Cast<APawn>(Owner))
	{
		// 프로젝트에서 "Steer" 또는 "Steering" 축을 사용하는 경우에 맞게 수정하세요.
		SteeringInput = OwnerPawn->GetInputAxisValue(TEXT("Steer"));
	}

	// ── UTM 좌표 변환 ─────────────────────────────────────────────────────
	double UtmEasting = 0.0;
	double UtmNorthing = 0.0;
	WorldToUtm(WorldLoc, UtmEasting, UtmNorthing);

	// ─────────────────────────────────────────────────────────────────────
	//  디버그 드로우
	// ─────────────────────────────────────────────────────────────────────
	if (bDrawTrajectory && World)
	{
		const FColor LineColor = SpeedToColor(SpeedKmh);

		// 1) DrawDebugLine — 이전 위치 → 현재 위치
		if (bHasPreviousFrame)
		{
			DrawDebugLine(
				World,
				PreviousLocation,
				WorldLoc,
				LineColor,
				/*bPersistentLines=*/ (DebugDrawLifeTime < 0.0f),
				/*LifeTime=*/          DebugDrawLifeTime,
				/*DepthPriority=*/     0,
				TrajectoryLineThickness
			);
		}

		// 2) DrawDebugPoint — 급감속 감지
		//    AccelerationKmhS 가 음수(감속)이고 절댓값이 임계값 초과
		if (bHasPreviousFrame && (-AccelerationKmhS) > HardBrakeThresholdKmhPerSec)
		{
			DrawDebugPoint(
				World,
				WorldLoc,
				HardBrakePointSize,
				FColor::Yellow,
				/*bPersistentLines=*/ (DebugDrawLifeTime < 0.0f),
				DebugDrawLifeTime
			);
		}

		// 3) DrawDebugString — 일정 거리마다 속도/Yaw 수치 표시
		if (bHasPreviousFrame)
		{
			AccumulatedDistanceCm += FVector::Dist(PreviousLocation, WorldLoc);
		}

		if (AccumulatedDistanceCm >= LabelIntervalCm || !bHasPreviousFrame)
		{
			const FString Label = FString::Printf(
				TEXT("%.1f km/h | Yaw: %.1f°"), SpeedKmh, Yaw);

			DrawDebugString(
				World,
				WorldLoc + FVector(0.0f, 0.0f, 50.0f),   // 살짝 위에 표시
				Label,
				/*TestBaseActor=*/ nullptr,
				FColor::White,
				DebugDrawLifeTime,
				/*bDrawShadow=*/ true
			);

			AccumulatedDistanceCm = 0.0f;
		}
	}

	// ─────────────────────────────────────────────────────────────────────
	//  CSV 행 기록 (컬럼 추가: 가속도, 조향 입력)
	// ─────────────────────────────────────────────────────────────────────
	const FString Row = FString::Printf(
		TEXT("%.3f,%.2f,%.2f,%.2f,%.4f,%.4f,%d,%.2f,%.4f,%.4f,%.4f\n"),
		ElapsedRecordingTime,
		WorldLoc.X, WorldLoc.Y, WorldLoc.Z,
		UtmEasting, UtmNorthing, OriginUtmZone,
		SpeedKmh,
		Yaw,
		AccelerationKmhS,   // 추가
		SteeringInput       // 추가
	);

	FFileHelper::SaveStringToFile(Row, *CsvFilePath,
		FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM,
		&IFileManager::Get(),
		EFileWrite::FILEWRITE_Append
	);

	// ── 다음 프레임을 위한 이전 상태 저장 ────────────────────────────────
	PreviousLocation = WorldLoc;
	PreviousSpeedKmh = SpeedKmh;
	bHasPreviousFrame = true;
}