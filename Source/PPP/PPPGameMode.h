// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Logic/game.h"
#include "Misc.h"

#include "Cell.h"
#include "Unit.h"

#include "EngineGlobals.h"
#include "Engine.h"

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "PPPGameMode.generated.h"

/**
 * 
 */
UCLASS()
class PPP_API APPPGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
	virtual void BeginPlay() override;
	FTimerHandle ProcessTimerHandle, TestTimerHandle;

	void ProcessLogicEvent();
	void PushTestInput();

	void SetCell(ViewerEvent);
	void SetUnit(ViewerEvent);
	void ClearStage();
	void RemoveUnit(int);
	void MoveUnit(int, TArray<FIntPoint>);
	void UseItem(int, int, int);
	void SetUnitCommand(int, TArray<FIntPoint>, TArray<FIntPoint>);
	void ClearUnitCommand();

	AUnit* GetUnit(int);
	ACell* GetCell(FIntPoint);
	
	TMap<FIntPoint, ACell*> CellMap;
	TMap<int,AUnit*> UnitMap;
	
public:
	class GameLogic : public Game {};

	GameLogic gLogic = GameLogic();

	LogicEventToken EventToken;

	FVector GetCellPos(FIntPoint);

	void StartMoveTargetSelection();
	void EndMoveTargetSelection();

	void StartActTargetSelection();
	void EndActTargetSelection();
	
	void OnUnitCommandEnd();
	void OnUnitCommandMove(FIntPoint);
	void OnUnitCommandAct(FIntPoint);
};
