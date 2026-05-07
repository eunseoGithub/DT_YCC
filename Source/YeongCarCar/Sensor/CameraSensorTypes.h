#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraType.h"
#include "CameraSensorTypes.generated.h"

UENUM(BlueprintType)
enum class ELidarSensorPreset : uint8
{
	Custom           UMETA(DisplayName = "Custom"),
	VelodyneVLP16    UMETA(DisplayName = "Velodyne VLP-16"),
	VelodyneVLP32    UMETA(DisplayName = "Velodyne VLP-32C"),
	OusterOS1_64     UMETA(DisplayName = "Ouster OS1-64"),
	Livox_Mid360     UMETA(DisplayName = "Livox Mid-360")
};

USTRUCT(BlueprintType)
struct FLidarSensorConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LidarConfig", meta = (ClampMin = "1", ClampMax = "128"))
	int32 NumChannels = 16;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LidarConfig", meta = (ClampMin = "10", ClampMax = "3600"))
	int32 PointsPerChannel = 450;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LidarConfig", meta = (ClampMin = "1.0", ClampMax = "30.0"))
	float RotationRate = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LidarConfig", meta = (ClampMin = "100.0"))
	float MaxRange = 10000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LidarConfig", meta = (ClampMin = "0.0"))
	float MinRange = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LidarConfig", meta = (ClampMin = "-90.0", ClampMax = "90.0"))
	float VerticalFOVUpper = 15.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LidarConfig", meta = (ClampMin = "-90.0", ClampMax = "90.0"))
	float VerticalFOVLower = -15.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LidarConfig", meta = (ClampMin = "1.0", ClampMax = "360.0"))
	float HorizontalFOV = 360.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LidarConfig", meta = (ClampMin = "0.0", ClampMax = "50.0"))
	float NoiseStdDev = 2.0f;

	int32 GetTotalPoints() const { return NumChannels * PointsPerChannel; }
};

USTRUCT(BlueprintType)
struct FLidarPointCloudData
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PointCloud")
	TArray<FVector> Points;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PointCloud")
	TArray<float> Intensities;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PointCloud")
	int32 PointCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PointCloud")
	int64 FrameNumber = 0;

	void Reset()
	{
		Points.Reset();
		Intensities.Reset();
		PointCount = 0;
	}
};

USTRUCT(BlueprintType)
struct FBevRenderConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BEV", meta = (ClampMin = "64", ClampMax = "2048"))
	int32 ImageSize = 512;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BEV", meta = (ClampMin = "100.0"))
	float ViewRange = 10000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BEV")
	FLinearColor PointColor = FLinearColor(0.0f, 1.0f, 0.2f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BEV")
	FLinearColor BackgroundColor = FLinearColor(0.0f, 0.0f, 0.0f, 0.85f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BEV", meta = (ClampMin = "1.0", ClampMax = "8.0"))
	float PointSize = 2.0f;
};

