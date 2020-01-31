#pragma once

#include "base.h"
#include "events.h"

#define TEAM_P 0
#define TEAM_O 1

enum GameStates { front = 0, intermission = 1, stage = 2, stageResult = 3 };


class ViewerEvent;

class TurnList {
	map<int, pair<int,int>> turnMap_;
	int current_ = -1;
	int count_ = 0;

public:
	TurnList() {};
	int getCurrent_();
	int getTurnCount_();
	void update_();
	void addUnit_(Unit*);
	void removeUnit_(Unit*);
};


class Stage {
public:
	enum State_ { fin_completed = 0, fin_defeated = 1, run = 2 };

private:	
	void updateState_();
	vector<ViewerEvent> updateTurn_();

	Unit* currentUnit_ = NULL;
	Grid grid_;
	map<int,Unit*> unitMap_;
	TurnList turnList_;
	Stage::State_ state_ = State_::run;;
	UnitActionSelector uaSelector_;

public:
	Stage() : grid_(0, 0) {};
	Stage(map<int,Unit*>, int size);
	void initialize_();
	ViewerEvent getInitInfo_();
	vector<ViewerEvent> updateStage_();
	vector<ViewerEvent> onEvent_(UnitEvent);
	ViewerEvent getCurrentUnitActions_();
	void removeUnit_(Unit*);
	Unit* getUnit_(int);
	State_ getState_();
	Pos getStartPivot_(int);
	string toStr_();
};


class ViewerEvent {
public:
	enum Types_ { 
		init = 0, 
		move = 1, 
		use = 2, 
		remove = 3, 
		clear = 4, 
		turnChanged = 5,
		setUnitCommand = 6 };

	ViewerEvent() {};
	ViewerEvent(Types_ op) { op_ = op; };

	class UnitInfo {
	public:
		UnitInfo(Unit* u) {
			teamID_ = u->teamID_;
			hp_ = u->hp_;
			maxHP_ = u->maxHP_;
			pos_ = u->getPos_();
		};
		int hp_;
		int maxHP_;
		int teamID_;
		Pos pos_ = Pos(-1,-1);
	};

	Types_ op_;
	int unitID_ = 0;
	Pos targetPos_ = Pos(-1, -1);
	int targetUnitID_;
	int value = 0;
	vector<Pos> path_;
	vector<Pos> cell_;
	map<int, UnitInfo> unit_;

	vector<Pos> reachable_;
	vector<Pos> actable_;
};


class GameInput {
	string delim_ = "/";
public:
	GameInput(string in) {
		vector<string> token = splitString(in, delim_);
		try {
			op_ = token.at(0);
		}
		catch (const out_of_range&) {}
		token_ = vector<string>(token.begin() + 1, token.end());
		unitEvent_ = UnitEvent(op_, token_);
	}

	GameInput(string op, UnitEvent event) {
		op_ = op;
		unitEvent_ = event;
	}

	GameInput() {};

	string op_;
	vector<string> token_;
	UnitEvent unitEvent_;
};


class Game {
	virtual void updateView_();
	vector<ViewerEvent> setupStage_();
	void loadPlayerUnits_();
	void createPlayerUnits_();
	void updatePlayerUnits_();
	void createOtherUnits_();
	
	GameClasses gcls_;
	IDManager idman_;
	GameStates gameState_;
	Stage st_;
	int stageNumber_;

	UnitGroup playerUnits_;
	UnitGroup otherUnits_;
	
public:
	Game::Game();
	virtual ~Game() {};

	void onInput_(GameInput);
	void reset_();
	GameStates getState_() { return gameState_; };

	queue<ViewerEvent> viewerEvents_;
};
