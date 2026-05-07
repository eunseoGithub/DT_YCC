// Copyright NBC, Inc. All Rights Reserved.

#include "Base/UI/LapTimerWidget.h"
#include "Base/Component/SplineFollowerComponent.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "TimerManager.h"

void ULapTimerWidget::NativeConstruct()
{
	Super::NativeConstruct();
	GetWorld()->GetTimerManager().SetTimer(
		UpdateTimerHandle, this, &ULapTimerWidget::UpdateLapDisplay, 0.05f, true);
}

void ULapTimerWidget::NativeDestruct()
{
	if (GetWorld())
		GetWorld()->GetTimerManager().ClearTimer(UpdateTimerHandle);
	Super::NativeDestruct();
}

void ULapTimerWidget::InitWithSplineFollower(USplineFollowerComponent* InFollower)
{
	SplineFollower = InFollower;
	if (InFollower)
		InFollower->OnLapCompleted.AddUObject(this, &ULapTimerWidget::OnLapCompleted);
}

void ULapTimerWidget::UpdateLapDisplay()
{
	if (!SplineFollower.IsValid() || !CurrentLapText)
		return;

	const int32 Lap = SplineFollower->GetLapNumber() + 1;
	CurrentLapText->SetText(FText::FromString(
		FString::Printf(TEXT("Lap %d  %s"), Lap, *FormatTime(SplineFollower->GetLapElapsed()))));
}

void ULapTimerWidget::OnLapCompleted(int32 LapNumber, float LapTime)
{
	if (!LapListBox)
		return;

	UTextBlock* Entry = NewObject<UTextBlock>(this);
	Entry->SetText(FText::FromString(
		FString::Printf(TEXT("Lap %d  %s"), LapNumber, *FormatTime(LapTime))));
	LapListBox->AddChild(Entry);
}

FString ULapTimerWidget::FormatTime(float Seconds)
{
	const int32 Min = FMath::FloorToInt(Seconds / 60.f);
	const float Sec = FMath::Fmod(Seconds, 60.f);
	return FString::Printf(TEXT("%d:%05.2f"), Min, Sec);
}
