#pragma once

#include "base.h"


class UnitEvent;


class UnitActionSelector {
	Grid* g_;	

public:
	UnitActionSelector() {};

	void setGrid_(Grid* g) { 
		g_ = g; 
	};
	int getActUtilValue_(Unit*, Pos, Pos);
	int getMoveUtilValue_(Unit*, Pos);
	vector<UnitEvent> selectNext_(Unit*);
};


class UnitEvent {
public:
	enum Types_ { noop = -1, mv = 0, use = 1, endTurn = 2 };
	Types_ strToTypes_(string op) {
		if (op == "mv" || op == "m") {
			return Types_::mv;
		}
		else if (op == "use" || op == "u") {
			return Types_::use;
		}
		else if (op == "end" || op == "e") {
			return Types_::endTurn;
		}
		return Types_::noop;
	}

	UnitEvent(string op, vector<string> token) {
		try {
			op_ = strToTypes_(op);;
			vector<string> crd = splitString(token.at(0), ",");
			target_ = Pos(stoi(crd.at(0)), stoi(crd.at(1)));
		}
		catch (const out_of_range&) {}
		catch (const invalid_argument&) {}
	};

	UnitEvent(Types_ op, Pos target) {
		op_ = op;
		target_ = target;
	}

	UnitEvent() { op_ = Types_::noop; };

	Types_ op_;
	Pos target_ = Pos(-1, -1);
};


class Command {
public:
	Command(Grid* g, Unit* u) { g_ = g; u_ = u; };
	virtual bool apply_(Pos) { return false; };

	Grid* g_;
	Unit* u_;
};


class Deploy : Command {
public:
	Deploy(Grid* g, Unit* u) : Command(g, u) {};
	virtual bool apply_(Pos);
};


class MoveUnit : Command {
public:
	MoveUnit(Grid* g, Unit* u) : Command(g, u) {};
	virtual ~MoveUnit() {};
	virtual bool apply_(Pos);

	vector<Pos> path_;
};


class UseItem : Command {
public:
	UseItem(Grid* g, Unit* u) : Command(g, u) {};
	virtual ~UseItem() { affected_.clear(); };
	virtual bool apply_(Pos);
	
	vector<pair<Unit*,int>> affected_;
};
