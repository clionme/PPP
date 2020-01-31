// Fill out your copyright notice in the Description page of Project Settings.

#include "PPPGameMode.h"
#include "Algo/Reverse.h"

void APPPGameMode::BeginPlay() {
	GetWorldTimerManager().SetTimer(ProcessTimerHandle , this, &APPPGameMode::ProcessLogicEvent, 0.1f, true);
	GetWorldTimerManager().SetTimer(TestTimerHandle, this, &APPPGameMode::PushTestInput, 0.1f, true);
}

void APPPGameMode::SetCell(ViewerEvent evt)
{
	UE_LOG(LogTemp, Log, TEXT("set cell"));
	FVector orig = FVector(0.0f, 0.0f, 0.0f);

	for (auto cellInfo : evt.cell_) {		
		ACell *added = (ACell*)GetWorld()->SpawnActor(ACell::StaticClass(), &orig);
		FIntPoint pos = FIntPoint(cellInfo.x_, cellInfo.y_);
		added->setPos(pos);
		CellMap.Add(pos, added);
	}
};

void APPPGameMode::SetUnit(ViewerEvent evt)
{
	UE_LOG(LogTemp, Log, TEXT("set unit"));
	FVector orig = FVector(0.0f, 0.0f, 0.0f);

	for (auto unitInfo : evt.unit_) {
		AUnit *added = (AUnit*)GetWorld()->SpawnActor(AUnit::StaticClass(), &orig);
		added->setPos(FIntPoint(unitInfo.second.pos_.x_, unitInfo.second.pos_.y_));
		added->ID = unitInfo.first;
		added->SetUnitInfo(unitInfo.second.teamID_, unitInfo.second.maxHP_, unitInfo.second.hp_);
		UnitMap.Add(added->ID, added);
	}
};

void APPPGameMode::ClearStage()
{
	UE_LOG(LogTemp, Log, TEXT("clear stage"));
	for (auto elem : UnitMap) {		
		elem.Value->Destroy();
	}
	UnitMap.Empty();
	for (auto elem : CellMap) {
		elem.Value->Destroy();
	}
	CellMap.Empty();
}

void APPPGameMode::RemoveUnit( int unitID )
{
	UE_LOG(LogTemp, Log, TEXT("remove %d"), unitID);
	AUnit*u = GetUnit(unitID);
	if (u) {
		EventToken = LogicEventToken(LogicEventTypes::REMOVE_);
		u->RemoveFromGrid(&EventToken);
	}
	int removed = UnitMap.Remove(unitID);
	UE_LOG(LogTemp, Log, TEXT("remove %d -- %d"), unitID, removed);
}

void APPPGameMode::MoveUnit(int unitID, TArray<FIntPoint> movePath)
{
	UE_LOG(LogTemp, Log, TEXT("move %d"), unitID);
	for (int i = 0; i < movePath.Num(); i++) {
		UE_LOG(LogTemp, Log, TEXT("move index : %d to(%d, %d)"), i, movePath[i].X, movePath[i].Y);
	}
	AUnit*u = GetUnit(unitID);
	if (u) {
		EventToken = LogicEventToken(LogicEventTypes::MOVE_);
		EventToken.Path = movePath;
		u->MoveOnGrid(&EventToken);
	}
}

void APPPGameMode::UseItem(int unitID, int targetUnitID, int value)
{
	UE_LOG(LogTemp, Log, TEXT("use %d, %d, %d"), unitID, targetUnitID, value);
	AUnit*target = GetUnit(targetUnitID);
	if (target) {
		EventToken = LogicEventToken(LogicEventTypes::USE_);
		target->OnAttacked(&EventToken, value);
	}
}

AUnit* APPPGameMode::GetUnit(int unitID) {
	if (UnitMap.Contains(unitID)) {
		return UnitMap[unitID];
	}
	return NULL;
}

ACell* APPPGameMode::GetCell(FIntPoint pos) {
	if (CellMap.Contains(pos)) {
		return CellMap[pos];
	}
	return NULL;
}

void APPPGameMode::PushTestInput() {
	if (EventToken.IsActive())
		return;
	GameInput in = GameInput("_", UnitEvent(UnitEvent::Types_::noop, Pos(-1, -1)));
	gLogic.onInput_(in);
}

void APPPGameMode::SetUnitCommand(int unitID, TArray<FIntPoint> reachable, TArray<FIntPoint> actable) {
	EventToken = LogicEventToken(LogicEventTypes::COMMAND_);
	EventToken.Finished();

	EventToken.UnitID = unitID;
	EventToken.Reachable = reachable;
	EventToken.Actable = actable;

	AUnit* unit = GetUnit(unitID);
	if (unit) {
		unit->SetCommand(reachable.Num() != 0, actable.Num() !=0);
	}	
}

void APPPGameMode::ClearUnitCommand() {
	EndActTargetSelection();
	EndMoveTargetSelection();

	if (EventToken.Type == LogicEventTypes::COMMAND_) {
		AUnit* unit = GetUnit(EventToken.UnitID);
		if (unit) {
			unit->ClearCommand();
		}
	}
}

void APPPGameMode::StartMoveTargetSelection() {
	EndActTargetSelection();

	if (EventToken.Type == LogicEventTypes::COMMAND_) {
		for (FIntPoint p : EventToken.Reachable) {
			ACell* cell = GetCell(p);
			if (cell) cell->SetReachable(true);
		}
	}
}

void APPPGameMode::EndMoveTargetSelection() {
	if (EventToken.Type == LogicEventTypes::COMMAND_) {
		for (FIntPoint p : EventToken.Reachable) {
			ACell* cell = GetCell(p);
			if (cell) cell->SetReachable(false);
		}
	}
}

void APPPGameMode::StartActTargetSelection() {
	EndMoveTargetSelection();

	if (EventToken.Type == LogicEventTypes::COMMAND_) {
		for (FIntPoint p : EventToken.Actable) {
			ACell* cell = GetCell(p);
			if (cell) cell->SetActable(true);
		}
	}
}

void APPPGameMode::EndActTargetSelection() {
	if (EventToken.Type == LogicEventTypes::COMMAND_) {
		for (FIntPoint p : EventToken.Actable) {
			ACell* cell = GetCell(p);
			if (cell) cell->SetActable(false);
		}
	}
}

void APPPGameMode::OnUnitCommandEnd() {
	ClearUnitCommand();
	gLogic.onInput_(GameInput("_", UnitEvent(UnitEvent::Types_::endTurn, Pos(-1, -1))));
}

void APPPGameMode::OnUnitCommandMove( FIntPoint pos ) {
	ClearUnitCommand();
	gLogic.onInput_(GameInput("_", UnitEvent(UnitEvent::Types_::mv, Pos(pos.X, pos.Y))));
}

void APPPGameMode::OnUnitCommandAct(FIntPoint pos) {
	ClearUnitCommand();
	gLogic.onInput_(GameInput("_", UnitEvent(UnitEvent::Types_::use, Pos(pos.X, pos.Y))));
}

TArray<FIntPoint> toTArray(vector<Pos> pos);

void APPPGameMode::ProcessLogicEvent() {
	// UE_LOG(LogTemp, Log, TEXT("gate state : %d"), gLogic.getState_());

	while( !EventToken.IsActive() && !gLogic.viewerEvents_.empty() ) {
		ClearUnitCommand();

		ViewerEvent evt = gLogic.viewerEvents_.front();
		gLogic.viewerEvents_.pop();
		switch (evt.op_) {
			case ViewerEvent::Types_::init:
			{
				UE_LOG(LogTemp, Log, TEXT("init"));
				SetCell(evt);
				SetUnit(evt);
			}
			break;
			case ViewerEvent::Types_::clear:
			{
				ClearStage();				
			}
			break;
			case ViewerEvent::Types_::remove:
			{
				RemoveUnit(evt.unitID_);			
			}
			break;
			case ViewerEvent::Types_::move:
			{				
				TArray<FIntPoint> movePath = toTArray(evt.path_);
				Algo::Reverse(movePath);
				MoveUnit(evt.unitID_, movePath);
			}
			break;
			case ViewerEvent::Types_::use:
			{
				UseItem(evt.unitID_, evt.targetUnitID_, evt.value);				
			}
			break;
			case ViewerEvent::Types_::setUnitCommand:
			{				
				SetUnitCommand(evt.unitID_, toTArray(evt.reachable_), toTArray(evt.actable_));
			}
		}
	}
}

TArray<FIntPoint> toTArray( vector<Pos> pos ) {
	TArray<FIntPoint> localized;
	for (auto iter : pos ) {
		localized.Add(FIntPoint(iter.x_, iter.y_));
	}
	return localized;
}
