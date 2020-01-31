// Fill out your copyright notice in the Description page of Project Settings.

#include "StatBar.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "EngineGlobals.h"
#include "Engine.h"

bool UStatBar::Initialize() {
	bool Success = Super::Initialize();
	if (!Success) return false;

	if (!ensure(HPBar)) { return false; } 
	if (!ensure(HPText)) { return false; }
	if (!ensure(HPDelta)) { return false; }

	ClearHPDelta();

	return true;
}

void UStatBar::SetColor(FColor colorIn) {
	FLinearColor color = FLinearColor(colorIn);
	HPBar->SetFillColorAndOpacity(color);
	HPText->SetColorAndOpacity(color);
}

void UStatBar::SetHP(int hp, int maxHP) {
	HPText->SetText(FText::FromString(FString::Printf(TEXT("%d"), hp)));
	HPBar->SetPercent( ((float)hp) /maxHP);	
}

void UStatBar::SetHPDelta(int delta) {
	HPDelta->SetText(FText::FromString(FString::Printf(TEXT("-%d"), delta)));
	GetWorld()->GetTimerManager().SetTimer(ClearTimerHandle, this, &UStatBar::ClearHPDelta, 1.0f, false);
}

void UStatBar::ClearHPDelta() {	
	HPDelta->SetText(FText::FromString(FString("")));
}
