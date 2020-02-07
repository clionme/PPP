#pragma once

#include <iostream>
#include <vector>
#include <queue>
#include <map>
#include <string>
#include <chrono>
#include <ctime>

using namespace std;

#define TURN_PERIOD_BASE 100
#define MAP_SIZE 7

struct Pos {
	Pos(int x, int y) : x_(x), y_(y) {};
	int x_, y_;
	bool operator== (const Pos& other) {
		return (x_ == other.x_) && (y_ == other.y_);
	}
	bool operator!= (const Pos& other) {
		return (x_ != other.x_) || (y_ != other.y_);
	}
	Pos operator+(const Pos& other) {
		return Pos(x_ + other.x_, y_ + other.y_);
	}
	Pos operator-(const Pos& other) {
		return Pos(x_ - other.x_, y_ - other.y_);
	}
	friend bool operator <(const Pos& left, const Pos& right) {
		return left.x_ < right.x_ \
			|| ( left.x_ == right.x_ && left.y_ < right.y_ );
	}
	inline int mDist_( const Pos& other ) {
		return abs( other.x_ - x_ ) + abs( other.y_ - y_ );
	}
	vector<Pos> adjacent(){
		vector<Pos> adj;
		adj.push_back(up_());
		adj.push_back(down_());
		adj.push_back(left_());
		adj.push_back(right_());
		return adj;
	};
	inline Pos up_() {
		return Pos(x_, y_ + 1);
	}
	inline Pos left_() {
		return Pos(x_ - 1, y_);
	}
	inline Pos right_() {
		return Pos(x_ + 1, y_);
	}
	inline Pos down_() {
		return Pos(x_, y_ - 1);
	}	
};


enum ItemTypes;
struct ItemClass;
class Item;
struct UnitClass;
class Unit;
class UnitGroup;
class IDManager;


struct CellInfo {
	CellInfo::CellInfo(Pos p, int height) 
		: p_(p), height_(height) {};
	Pos p_;
	int height_;
public:
	Pos getPos_() { return p_; };
	int getHeight_() { return height_; };
};


class Cell {
public:
	Cell::Cell( Pos p, int height ) 
		: info_(CellInfo(p, height)) {};
	string toStr_();

	CellInfo info_;	
	Unit* unit_ = NULL;
	Pos getPos_() { return info_.getPos_(); }
	CellInfo getInfo_() { return info_; }
};


class MapGenerator {
	const static int componentSize_ = 3;
	vector<vector<vector<int>>> components_
		= {
			{
				{ 3,4,3 },
				{ 2,3,2 },
				{ 1,0,1 },
			},{
				{ 3,0,1 },
				{ 2,1,1 },
				{ 1,0,0 },
			},{
				{ 3,3,2 },
				{ 3,4,1 },
				{ 0,1,1 },
			},{
				{ 3,2,1 },
				{ 2,1,2 },
				{ 3,0,3 },
			},{
				{ 0,0,0 },
				{ 1,0,0 },
				{ 0,0,1 },
			},
	};

public:
	map<Pos, int> generateHeightMap_(int xSize, int ySize);
};

class Grid {
private:
	vector<vector<Cell>> vCell_;
	Cell* getCell_(Pos&);
	
public:
	Grid::Grid(int xin, int yin);
	string toStr_();
	bool place_(Unit&, Pos&);	
	bool isEmpty_(Pos&);
	bool isInBound_(Pos&);
	vector<Pos> getPath_(Pos&, Pos&);
	vector<Pos> getReachablePos_(Unit *);
	map <Pos, vector<Pos>> getActablePosMap_(Unit*, vector<Pos>);
	vector<Pos> getUseCandidates_(ItemClass&, Pos);
	vector<Unit*> getUnits_();
	Unit* getUnitAt_(Pos&);
	vector<CellInfo> getCellInfo_();
		
	int xMax_, yMax_;
};


enum ItemTypes {
	wep = 0,
	consumable = 1
};


struct ItemClass {
	ItemClass::ItemClass() {};
	ItemClass::ItemClass(
		string, ItemTypes, int, int, int, int);
	
	string name_;
	ItemTypes type_;
	int pDmg_, mDmg_, weight_, range_;
};


class Item {
	Item::Item(ItemClass* itemClass) { itemClass_ = itemClass; };
	ItemClass* itemClass_;
};


struct UnitClassParam {
public:
	UnitClassParam(string, int, int, int, int, int );
	string name_;
	int maxHP_, pDef_, mDef_, spd_, moveRange_;
	int turnPeriod_;
};


struct UnitClass {
public:
	UnitClass( UnitClassParam, ItemClass& );
	UnitClassParam prm_;
	ItemClass& defaultItemClass_;
};


class Unit {
public:
	Unit(IDManager&, string, UnitClass*, vector<Item*>, int);
	bool isAlive_();	
	bool isAlly_( Unit* );
	friend bool operator==( const Unit& left, const Unit& right ) {
		return &left == &right;
	}
	Pos getPos_() {
		if (at_)
			return at_->getPos_();
		return Pos(-1, -1);
	}
	int onAttacked_(Unit*, ItemClass&);
	int getDmg_(Unit*, ItemClass&);
	void detachFromCell_();
	void recover_();
	void onTurnStarted_();
	void adjustMoveCount_();
	void adjustActionCount_();
	bool finishedTurn_();
	bool canMove_();
	bool canAct_();

	int id_;
	string name_;
	UnitClass* unitClass_;	

	int teamID_;
	int hp_, maxHP_;
	Cell* at_ = NULL;
	int moveCount_ = -1;
	int actionCount_ = -1;
};


class UnitGroup {
	map<int,Unit> units_;	

public:
	UnitGroup() {};	
	void addUnit_( Unit );
	void removeUnit_( Unit );
	void clear_();
	bool isInActive_();
	vector<int> getUnitIDs();
	Unit* getUnit(int);	
};


class GameClasses {
	map<string, ItemClass> itemMap;
	map<string, UnitClass> unitMap;

public:
	ItemClass& getItemClass(string name) {
		return itemMap.find(name)->second;
	}

	UnitClass& getUnitClass(string name) {
		return unitMap.find(name)->second;
	}

	GameClasses() {
		// initialize data
		itemMap.insert(pair<string, ItemClass>("IS0", ItemClass("name IS0", ItemTypes::wep, 5, 0, 3, 1)));
		itemMap.insert(pair<string, ItemClass>("IL0", ItemClass("name IL0", ItemTypes::wep, 3, 0, 4, 1)));
		itemMap.insert(pair<string, ItemClass>("IB0", ItemClass("name IB0", ItemTypes::wep, 3, 0, 1, 3)));
		itemMap.insert(pair<string, ItemClass>("IT0", ItemClass("name IT0", ItemTypes::wep, 0, 4, 2, 2)));

		unitMap.insert(pair<string, UnitClass>("UW0", UnitClass(UnitClassParam( "UW0", 8, 2, 0, 3, 4), getItemClass("IS0") )));
		unitMap.insert(pair<string, UnitClass>("UA0", UnitClass(UnitClassParam( "UA0", 7, 1, 1, 4, 4),  getItemClass("IB0") )));
		unitMap.insert(pair<string, UnitClass>("UM0", UnitClass(UnitClassParam( "UM0", 6, 0, 2, 2, 4),  getItemClass("IT0") )));
	}
};


class IDManager {
	int count_;
public:
	IDManager() : count_(0) {};
	int create_() {
		count_++;
		if (count_ == 0) {
			throw "unexpected number of units";
		}
		return count_-1;
	};
	void reset_(int count = 0) {
		count_ = count;
	}
};


vector<string> splitString(string in, string delim);
