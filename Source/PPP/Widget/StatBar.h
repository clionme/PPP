// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "StatBar.generated.h"

/**
 * 
 */
UCLASS()
class PPP_API UStatBar : public UUserWidget
{
	GENERATED_BODY()

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* HPText;

	UPROPERTY(meta = (BindWidget))
	class UProgressBar* HPBar;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* HPDelta;

	FTimerHandle ClearTimerHandle;

protected:
	virtual bool Initialize() override;

public:
	void SetHP(int,int);
	void SetColor(FColor);
	void SetHPDelta(int);
	void ClearHPDelta();

};
