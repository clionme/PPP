#include "events.h"
#include <algorithm>


int UnitActionSelector::getActUtilValue_( Unit* u, Pos fromPos, Pos targetPos ) {
	Unit* target = g_->getUnitAt_(targetPos);
	if (!target) {
		return 0;
	}

	if (u->isAlly_(target)) {
		return INT_MIN;
	}

	int dist = fromPos.mDist_(targetPos);

	int dmgDone = 0, dmgTaken = 0;

	if (dist <= u->unitClass_->defaultItemClass_.range_) {
		dmgDone = target->getDmg_(u, u->unitClass_->defaultItemClass_);
		if (dmgDone > target->hp_) {
			return INT_MAX;
		}
		if (dist <= target->unitClass_->defaultItemClass_.range_) {
			dmgTaken = u->getDmg_(target, target->unitClass_->defaultItemClass_);
			if (dmgTaken > u->hp_) {
				return -dmgTaken;
			}
		}
	}
	return dmgDone - dmgTaken;
}


int UnitActionSelector::getMoveUtilValue_(Unit* u, Pos destPos) {
	int value = 0;
	for (Unit* other : g_->getUnits_()) {
		if (!u->isAlly_(other)) {
			value -= destPos.mDist_(other->getPos_());
		}
	}
	return value;
}


vector<UnitEvent> UnitActionSelector::selectNext_(Unit* u) {
	vector<UnitEvent> ret;
	vector<Pos> reachable;

	if (u->canMove_()) {
		reachable  = g_->getReachablePos_(u);
	}
	else {
		reachable.push_back(u->getPos_());
	}

	if (u->canAct_()) {
		Pos actPos = Pos(-1, -1);
		Pos movePos = Pos(-1, -1);
		map<Pos, vector<Pos>> actable = g_->getActablePosMap_(u, reachable);

		int maxUtilValue = INT_MIN;
		for (auto elem : actable) {
			Pos fromPos = elem.first;
			for (Pos targetPos : elem.second) {
				int utilValue = getActUtilValue_(u, fromPos, targetPos);
				if (maxUtilValue < utilValue) {
					maxUtilValue = utilValue;
					movePos = fromPos;
					actPos = targetPos;
				}
			}
		}
		if (g_->isInBound_(movePos) && g_->isInBound_(actPos) ){
			if ( movePos != u->getPos_() )
				ret.push_back(UnitEvent(UnitEvent::Types_::mv, movePos));
			ret.push_back(UnitEvent(UnitEvent::Types_::use, actPos));
		}
	}

	if (u->canMove_() && ret.empty()) {
		Pos movePos = Pos(-1, -1);
		int maxUtilValue = INT_MIN;
		for (Pos targetPos : reachable) {
			int utilValue = getMoveUtilValue_(u, targetPos);
			if (maxUtilValue < utilValue) {
				maxUtilValue = utilValue;
				movePos = targetPos;
			}
		}

		if (g_->isInBound_(movePos)) {
			ret.push_back(UnitEvent(UnitEvent::Types_::mv, movePos));
		}
	}

	ret.push_back(UnitEvent(UnitEvent::Types_::endTurn, Pos(-1,-1)));
	return ret;
}


bool Deploy::apply_(Pos pivot) {
	vector<Pos> candidates;	
	int delta = 0;
	
	while (delta < g_->xMax_ ) {
		candidates.clear();
		for (int dx = -delta; dx <= delta; dx++) {
			int dy = delta - abs(dx);
			candidates.push_back(pivot + Pos(dx, dy));
			if (dy != 0 )
				candidates.push_back(pivot + Pos(dx, -dy));
		}
		random_shuffle(candidates.begin(), candidates.end());
		for (auto cand : candidates) {
			if (g_->isEmpty_(cand)) {
				g_->place_(*u_, cand);
				return true;
			}
		}
		delta += 1;
	}
	return false;
}


bool MoveUnit::apply_(Pos to) {
	if (!u_->canMove_()) {
		return false;
	}
	u_->adjustMoveCount_();

	Pos from = u_->getPos_();
	vector<Pos> path = g_->getPath_(from, to);

	int pathSize = path.size();
	if (pathSize == 0){
		return false;
	}
	else if (pathSize - 1 > u_->unitClass_->prm_.moveRange_) {
		return false;
	}

	g_->place_(*u_, path.front());

	path_ = path;
	return true;
}


bool UseItem::apply_(Pos to) {
	if (!u_->canAct_()) {
		return false;
	}
	u_->adjustActionCount_();
	
	Unit* target = g_->getUnitAt_(to);
	if (!target)
		return false;

	ItemClass& itemClassA = u_->unitClass_->defaultItemClass_;
	ItemClass& itemClassB = target->unitClass_->defaultItemClass_;
	
	int dist = u_->getPos_().mDist_(target->getPos_());

	if (ItemTypes::wep == itemClassA.type_ 
		&& u_->isAlive_()
		&& target->isAlive_()
		&& dist <= itemClassA.range_ ) {
		int dmgA = target->onAttacked_(u_, itemClassA);
		affected_.push_back(make_pair(target,dmgA));
		if ( u_->id_ != target->id_
			&& ItemTypes::wep == itemClassB.type_
			&& u_->isAlive_()
			&& target->isAlive_()
			&& dist <= itemClassB.range_) {
			int dmgB = u_->onAttacked_(target, itemClassB);
			affected_.push_back(make_pair(u_, dmgB));
		}
		return true;
	}

	return false;
}
