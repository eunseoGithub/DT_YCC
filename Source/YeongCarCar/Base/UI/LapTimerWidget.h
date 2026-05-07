// Copyright NBC, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LapTimerWidget.generated.h"

class UTextBlock;
class UVerticalBox;
class USplineFollowerComponent;

UCLASS()
class YEONGCARCAR_API ULapTimerWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void InitWithSplineFollower(USplineFollowerComponent* InFollower);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	void UpdateLapDisplay();
	void OnLapCompleted(int32 LapNumber, float LapTime);

	static FString FormatTime(float Seconds);

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> CurrentLapText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> LapListBox;

	TWeakObjectPtr<USplineFollowerComponent> SplineFollower;
	FTimerHandle UpdateTimerHandle;
};
