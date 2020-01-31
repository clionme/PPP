// Fill out your copyright notice in the Description page of Project Settings.

#include "UnitCommandBar.h"
#include "../PPPGameMode.h"
#include "Components/Button.h"

bool UUnitCommandBar::Initialize() {
	bool Success = Super::Initialize();
	if (!Success) return false;

	if (!ensure(MoveButton)) { return false; }
	MoveButton->OnClicked.AddDynamic(this, &UUnitCommandBar::OnMoveButtonClicked);

	if (!ensure(ActionButton)) { return false; }
	ActionButton->OnClicked.AddDynamic(this, &UUnitCommandBar::OnActButtonClicked);

	if (!ensure(EndButton)) { return false; }
	EndButton->OnClicked.AddDynamic(this, &UUnitCommandBar::OnEndButtonClicked);

	return true;
}

void UUnitCommandBar::OnEndButtonClicked() {
	APPPGameMode* gameMode = Cast<APPPGameMode>(GetWorld()->GetAuthGameMode());
	if (gameMode) {
		gameMode->OnUnitCommandEnd();
	}
}

void UUnitCommandBar::OnMoveButtonClicked() {
	APPPGameMode* gameMode = Cast<APPPGameMode>(GetWorld()->GetAuthGameMode());
	if (gameMode) {
		gameMode->StartMoveTargetSelection();
	}
}

void UUnitCommandBar::OnActButtonClicked() {
	APPPGameMode* gameMode = Cast<APPPGameMode>(GetWorld()->GetAuthGameMode());
	if (gameMode) {
		gameMode->StartActTargetSelection();
	}
}

void UUnitCommandBar::SetAvailable(bool canMove, bool canAct) {
	MoveButton->SetIsEnabled(canMove);
	ActionButton->SetIsEnabled(canAct);
}
