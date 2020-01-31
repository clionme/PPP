// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

enum LogicEventTypes { 
	NOOP_ = 0, 
	REMOVE_ = 1, 
	MOVE_ = 2, 
	USE_ = 3,
	COMMAND_ = 4,
};

class LogicEventToken {
	bool Active;

public:
	LogicEventToken() { 
		Active = false; 
		Type = LogicEventTypes::NOOP_;
	};

	LogicEventToken(LogicEventTypes eventType) {
		Active = true;
		Type = eventType;
	};

	void Finished() {
		Active = false;
	}

	bool IsActive() {
		return Active;
	}

	LogicEventTypes Type;
	float EndAfter, Elapsed;
	TArray<FIntPoint> Path;

	int UnitID = -1;
	TArray<FIntPoint> Reachable;
	TArray<FIntPoint> Actable;
};
