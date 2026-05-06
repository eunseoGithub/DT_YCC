#include "YCCSensorViewWidget.h"
#include "Components/Image.h"
#include "Components/Border.h"
#include "Engine/Texture2D.h"
#include "Engine/TextureRenderTarget2D.h"

void UYCCSensorViewWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (SensorBorder)
    {
        SensorBorder->SetVisibility(ESlateVisibility::Collapsed);
    }

    if (LidarBorder)
    {
        LidarBorder->SetVisibility(ESlateVisibility::Collapsed);
    }
}

void UYCCSensorViewWidget::SetRenderTarget(UTextureRenderTarget2D* InRenderTarget)
{
    if (!InRenderTarget || !SensorImage)
    {
        return;
    }

    SensorImage->SetBrushResourceObject(InRenderTarget);
}

void UYCCSensorViewWidget::SetLidarRenderTarget(UTexture2D* InRenderTarget)
{
    if (!InRenderTarget || !LidarImage)
    {
        return;
    }

    LidarImage->SetBrushResourceObject(InRenderTarget);
}

void UYCCSensorViewWidget::ToggleCameraView()
{
    if (!SensorBorder)
    {
        return;
    }

    const ESlateVisibility NewVis = IsCameraViewVisible()
        ? ESlateVisibility::Collapsed
        : ESlateVisibility::SelfHitTestInvisible;

    SensorBorder->SetVisibility(NewVis);
}

void UYCCSensorViewWidget::ToggleLidarView()
{
    if (!LidarBorder)
    {
        return;
    }

    const ESlateVisibility NewVis = IsLidarViewVisible()
        ? ESlateVisibility::Collapsed
        : ESlateVisibility::SelfHitTestInvisible;

    LidarBorder->SetVisibility(NewVis);
}

bool UYCCSensorViewWidget::IsCameraViewVisible() const
{
    return SensorBorder && SensorBorder->GetVisibility() != ESlateVisibility::Collapsed;
}

bool UYCCSensorViewWidget::IsLidarViewVisible() const
{
    return LidarBorder && LidarBorder->GetVisibility() != ESlateVisibility::Collapsed;
}
