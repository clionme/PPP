#include "base.h"


string Cell::toStr_() {
	string sub;
	if (unit_)
		sub = to_string(unit_->id_);
	else
		sub = "-";
	sub = string(" ", 3 - sub.size()) + sub;
	return sub;
}


Grid::Grid(int xin, int yin) {
	xMax_ = xin;
	yMax_ = yin;

	for (int x = 0; x < xin; x++) {
		vector<Cell> sub;
		for (int y = 0; y < yin; y++) {
			sub.push_back(Cell( Pos(x, y) ));
		}
		vCell_.push_back(sub);
	}
}


Cell* Grid::getCell_(Pos& p) {
	try {
		return &vCell_.at(p.x_).at(p.y_);
	}
	catch (const out_of_range& ) {
	}
	return NULL;
}


bool Grid::isEmpty_(Pos& p) {
Cell* cell = getCell_(p);
if (!cell) {
	return false;
}
return !cell->unit_;
}


bool Grid::isInBound_(Pos& p) {
	Cell* cell = getCell_(p);
	if (!cell) {
		return false;
	}
	return true;
}


bool Grid::place_(Unit& u, Pos& p) {
	Cell* cell = getCell_(p);
	if (!cell) {
		return false;
	}
	u.detachFromCell_();
	u.at_ = cell;
	cell->unit_ = &u;
	return true;
}


vector<Pos> Grid::getPath_(Pos& from, Pos& to) {
	map<Pos, int> costMap;
	queue<Pos> candidates;

	costMap.insert(pair<Pos, int>(from, 0));
	candidates.push(from);

	int cost = 0;
	while (!candidates.empty()) {
		Pos pos = candidates.front();
		candidates.pop();
		cost = costMap[pos];

		if (pos == to) {
			// backtrace and return path
			vector<Pos> path;
			while (true) {
				path.push_back(pos);
				if (cost == 0)
					break;
				for (Pos other : pos.adjacent()) {
					auto value = costMap.find(other);
					if (value == costMap.end() || value->second >= cost)
						continue;
					cost = value->second;
					pos = other;
					break;
				}
			}
			return path;
		}

		// add candidates
		cost++;
		for (Pos other : pos.adjacent()) {
			if (isEmpty_(other) && costMap.find(other) == costMap.end()) {
				costMap.insert(pair<Pos, int>(other, cost));
				candidates.push(other);
			}
		}
	}

	return vector<Pos>();
}


int getMoveCost(Pos, Pos) { return 1; };

vector<Pos> Grid::getReachablePos_(Unit * u) {
	int moveRange = u->unitClass_->prm_.moveRange_;
	Pos from = u->getPos_();
	vector<Pos> reachable;
	map<Pos, int> candidates;
	candidates.insert(make_pair(from, 0));

	while (!candidates.empty()) {
		Pos closest = Pos(-1, -1);
		int cost = INT_MAX;
		for (auto elem : candidates) {
			if (elem.second < cost) {
				closest = elem.first;
				cost = elem.second;
			}
		}
		reachable.push_back(closest);
		candidates.erase(closest);

		for (Pos to : closest.adjacent()) {
			int nextCost = getMoveCost(closest, to) + cost;
			if (nextCost > moveRange
				|| !isEmpty_(to)
				|| find(reachable.begin(), reachable.end(), to) != reachable.end()
				|| candidates.find(to) != candidates.end()) {
				continue;
			}
			candidates.insert(make_pair(to, nextCost));
		}
	}

	return reachable;
}


map < Pos, vector<Pos>> Grid::getActablePosMap_(Unit* u, vector<Pos> reachable) {
	map < Pos, vector<Pos> > actable;
	int range = u->unitClass_->defaultItemClass_.range_;
	vector<Unit*> otherUnits = getUnits_();
	
	for (Pos p : reachable) {
		vector<Pos> sub = getUseCandidates_(u->unitClass_->defaultItemClass_, p);
		if (!sub.empty()) {
			actable.insert(pair<Pos, vector<Pos>>( p, sub ));
		}
	}

	return actable;
}


vector<Pos> Grid::getUseCandidates_(ItemClass& itemCls, Pos pos) {
	int range = itemCls.range_;
	vector<Pos> actable;

	for (Unit* unit : getUnits_()) {
		Pos unitPos = unit->getPos_();
		if (unitPos.mDist_(pos) <= range) {
			actable.push_back(unitPos);
		}
	}
	return actable;

	// TODO - allow action on empty cell
	/*
	for (int dx = -range; dx <= range; dx++) {
		for (int dy = -range; dy <= range; dy++) {
			Pos candidate = pos + Pos(dx, dy);
			if (getCell_(candidate)) {
				actable.push_back(candidate);
			}
		}
	}
	*/
	return actable;
}


vector<Unit*> Grid::getUnits_() {
	vector<Unit*> units;
	for (auto sub : vCell_) {
		for (Cell cell : sub) {
			if ( cell.unit_ )
				units.push_back(cell.unit_);
		}
	}
	return units;
}


Unit* Grid::getUnitAt_(Pos & pos) {
	Cell *cell = getCell_(pos);
	if (cell)
		return cell->unit_;
	return NULL;
}


vector<Pos> Grid::getCellInfo_() {
	vector<Pos> ret;
	for (int y = 0; y < yMax_; y++) {
		for (int x = 0; x < xMax_; x++) {
			ret.push_back( vCell_[x][y].p_ );
		}
	}
	return ret;
}


string Grid::toStr_() {
	string str;
	for (int y = 0; y < yMax_; y++) {
		for (int x = 0; x < xMax_; x++) {
			str += vCell_[x][y].toStr_();
		}
		str += '\n';
	}
	str += '\n';
	return str;
}


ItemClass::ItemClass(
	string name,
	ItemTypes type,
	int pd,
	int md,
	int weight,
	int range
) {
	name_ = name;
	type_ = type;
	pDmg_ = pd;
	mDmg_ = md;
	weight_ = weight;
	range_ = range;
}


Unit::Unit(
	IDManager& idm,
	string name,
	UnitClass* unitClass,
	vector<Item*> items,
	int teamID
){
	id_ = idm.create_();
	name_ = name;
	maxHP_ = unitClass->prm_.maxHP_;
	hp_ = maxHP_;
	unitClass_ = unitClass;
	teamID_ = teamID;
};


bool Unit::isAlive_() {
	return hp_ > 0;
}


void Unit::recover_() {
	hp_ = maxHP_;
}


void Unit::onTurnStarted_() {
	moveCount_ = 1;
	actionCount_ = 1;
}


void Unit::adjustMoveCount_() {
	moveCount_ = 0;
}


void Unit::adjustActionCount_() {
	actionCount_ = 0;
}


bool Unit::finishedTurn_() {
	return moveCount_ <= 0 && actionCount_ <= 0;
}


bool Unit::canMove_() {
	return moveCount_ > 0;
}


bool Unit::canAct_() {
	return actionCount_ > 0;
}


bool Unit::isAlly_( Unit* other ) {
	return teamID_ == other->teamID_;
}


int Unit::onAttacked_(Unit*u, ItemClass& i) {
	int dmg = getDmg_(u, i);
	hp_ -= dmg;
	hp_ = min(max(0, hp_), maxHP_);
	return dmg;
}


int Unit::getDmg_(Unit *u, ItemClass& i) {
	return max(0, i.pDmg_ - unitClass_->prm_.pDef_)
		+ max(0, i.mDmg_ - unitClass_->prm_.mDef_);
}


void Unit::detachFromCell_() {
	if (at_) {
		at_->unit_ = NULL;
		at_ = NULL;
	}
}


UnitClassParam::UnitClassParam(
	string name,
	int maxHP,
	int pDef,
	int mDef,
	int spd,
	int moveRange
) {
	name_ = name;
	maxHP_ = maxHP;
	pDef_ = pDef;
	mDef_ = mDef;
	spd_ = spd;
	turnPeriod_ = int(ceil(TURN_PERIOD_BASE / spd));
	moveRange_ = moveRange;
};


UnitClass::UnitClass(
	UnitClassParam prm,
	ItemClass& defaultItemClass
) : prm_(prm),
	defaultItemClass_(defaultItemClass) {
};


void UnitGroup::addUnit_( Unit u ) {
	units_.insert(pair<int,Unit>(u.id_, u));
}


void UnitGroup::removeUnit_(Unit u) {
	units_.erase(units_.find(u.id_));
}


void UnitGroup::clear_() {
	units_.clear();
}


bool UnitGroup::isInActive_() {
	for (auto p : units_) {
		if ( p.second.isAlive_() ) {
			return false;
		}
	}
	return true;
}


vector<int> UnitGroup::getUnitIDs() {
	vector<int> sub;
	for (auto p : units_) {
		sub.push_back(p.first);
	}
	return sub;
}


Unit* UnitGroup::getUnit(int id) {
	auto elem = units_.find(id);
	if (elem == units_.end()) {
		return NULL;
	}
	return &elem->second;
}


vector<string> splitString(string in, string delim) {
	vector<string> token;
	size_t pos = 0;
	while ((pos = in.find_first_of(delim)) != string::npos) {
		token.push_back(in.substr(0, pos));
		in.erase(0, pos + delim.length());
	}
	token.push_back(in);
	return token;
};
