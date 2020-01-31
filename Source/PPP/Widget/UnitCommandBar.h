// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UnitCommandBar.generated.h"

/**
 * 
 */
UCLASS()
class PPP_API UUnitCommandBar : public UUserWidget
{
	GENERATED_BODY()
	
	UPROPERTY(meta = (BindWidget))
	class UButton* MoveButton;

	UPROPERTY(meta = (BindWidget))
	class UButton* ActionButton;

	UPROPERTY(meta = (BindWidget))
	class UButton* EndButton;

protected:
	virtual bool Initialize() override;

	UFUNCTION()
	void OnEndButtonClicked();

	UFUNCTION()
	void OnMoveButtonClicked();

	UFUNCTION()
	void OnActButtonClicked();

public:
	void SetAvailable(bool, bool);
};
