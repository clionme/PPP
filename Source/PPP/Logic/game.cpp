#include "game.h"


void TurnList::update_() {
	if (turnMap_.empty())
		return;
	
	int shortest = INT_MAX;
	int next = -1;
	for (auto elem : turnMap_) {
		if (shortest > elem.second.first) {
			shortest = elem.second.first;
			next = elem.first;
		}
	}
	for (auto elem : turnMap_) {
		turnMap_[elem.first] = make_pair( 
			elem.second.first - shortest,
			elem.second.second );
	}
	turnMap_[next].first = turnMap_[next].second;
	current_ = next;
	count_++;
}


int TurnList::getCurrent_() {
	return current_;
}


int TurnList::getTurnCount_() {
	return count_;
}


void TurnList::addUnit_(Unit* u) {
	int period = u->unitClass_->prm_.turnPeriod_;
	turnMap_[u->id_] = make_pair(period, period);
}


void TurnList::removeUnit_(Unit* u) {
	turnMap_.erase(turnMap_.find(u->id_));
	if (current_ == u->id_)
		update_();
}


Stage::Stage(map<int,Unit*> unitMap, int size)
	: unitMap_(unitMap),
	grid_(Grid(size, size)),	
	currentUnit_(NULL)
{};


void Stage::initialize_() {
	for (auto elem : unitMap_) {
		Deploy d = Deploy(&grid_, elem.second);
		d.apply_(getStartPivot_(elem.second->teamID_));
		turnList_.addUnit_(elem.second);
	}

	uaSelector_.setGrid_(&grid_);
}


ViewerEvent Stage::getInitInfo_() {
	auto evt = ViewerEvent(ViewerEvent::Types_::init);

	for (auto elem : unitMap_) {
		evt.unit_.insert(pair<int, ViewerEvent::UnitInfo>(\
			elem.first, ViewerEvent::UnitInfo( elem.second )));
	}
	evt.cell_ = grid_.getCellInfo_();
	return evt;
}


ViewerEvent Stage::getCurrentUnitActions_() {
	ViewerEvent evt = ViewerEvent(ViewerEvent::Types_::setUnitCommand);
	if (currentUnit_) {
		if (currentUnit_->canMove_()) {
			evt.reachable_ = grid_.getReachablePos_(currentUnit_);
		}
		if (currentUnit_->canAct_()) {
			evt.actable_ = grid_.getUseCandidates_(
				currentUnit_->unitClass_->defaultItemClass_,
				currentUnit_->getPos_());
		}
		evt.unitID_ = currentUnit_->id_;
	}
	return evt;
}


vector<ViewerEvent> Stage::updateStage_() {
	vector<ViewerEvent> events;

	updateState_();
	vector<ViewerEvent> ret = updateTurn_();
	events.insert(events.end(), ret.begin(), ret.end());

	if (state_ == Stage::State_::run
		&& currentUnit_ 
		&& currentUnit_->teamID_ == TEAM_O
	) 
	{
		int turnCount = turnList_.getTurnCount_();
		for (UnitEvent event : uaSelector_.selectNext_(currentUnit_)) {
			vector<ViewerEvent> ret = onEvent_(event);
			events.insert(events.end(), ret.begin(), ret.end());
			if (turnCount != turnList_.getTurnCount_())
				break;
		}
	}
	return events;
}


void Stage::updateState_() {
	if (state_ != Stage::State_::run) {
		return;
	}

	map<int, int> count;
	for (auto elem : unitMap_) {
		Unit *u = elem.second;
		count[u->teamID_] += 1;
	}
	if (count[TEAM_P] == 0) {
		state_ = Stage::State_::fin_defeated;
	}
	else if (count[TEAM_O] == 0) {
		state_ = Stage::State_::fin_completed;
	}
}


vector<ViewerEvent> Stage::updateTurn_() {
	vector<ViewerEvent> events;
	while (true) {
		if (currentUnit_ 
			&& currentUnit_->isAlive_()
			&& !currentUnit_->finishedTurn_() ){
			break;
		}
		turnList_.update_();
		int unitID = turnList_.getCurrent_();
		currentUnit_ = getUnit_(unitID);
		if (!currentUnit_) {
			break;
		}
		currentUnit_->onTurnStarted_();
		auto evt = ViewerEvent(ViewerEvent::Types_::turnChanged);
		evt.unitID_ = unitID;
		events.push_back(evt);
	}		
	return events;
}


vector<ViewerEvent> Stage::onEvent_(UnitEvent in) {
	vector<ViewerEvent> events;

	if (!currentUnit_) {
		cout << "event process failed - turn list failure" << endl;
		return events;
	}

	int unitID = currentUnit_->id_;
	vector<Unit *> affected;

	switch (in.op_) {
		case UnitEvent::Types_::mv:
		{
			MoveUnit moveE = MoveUnit(&grid_, currentUnit_);
			if (moveE.apply_(in.target_)) {
				cout << "mv succeeded " << unitID << endl;
				auto evt = ViewerEvent(ViewerEvent::Types_::move);
				evt.unitID_ = unitID;
				evt.path_ = moveE.path_;
				events.push_back(evt);
			}
			else
				cout << "mv failed " << unitID << endl;
		}
		break;
		case UnitEvent::Types_::use:
		{
			UseItem useE = UseItem(&grid_, currentUnit_);
			if (useE.apply_(in.target_)) {
				cout << "use item succeeded " << unitID << endl;
				for (auto elem : useE.affected_) {
					affected.push_back(elem.first);
					auto evt = ViewerEvent(ViewerEvent::Types_::use);
					evt.unitID_ = elem.first->id_;
					evt.value = elem.second;
					evt.targetPos_ = elem.first->getPos_();
					Unit* targetUnit = grid_.getUnitAt_(evt.targetPos_);
					if( targetUnit)
						evt.targetUnitID_ = targetUnit->id_;
					events.push_back(evt);
				}
			}
			else
				cout << "use item failed " << unitID << endl;
		}
		break;
		case UnitEvent::Types_::endTurn:
		{
			currentUnit_->adjustMoveCount_();
			currentUnit_->adjustActionCount_();			
		}
		break;
	}

	for (Unit* other : affected) {
		if (!other->isAlive_()) {
			removeUnit_(other);
			auto evt = ViewerEvent(ViewerEvent::Types_::remove);
			evt.unitID_ = other->id_;
			events.push_back(evt);
		}
	}

	auto ret = updateStage_();
	events.insert(events.end(), ret.begin(), ret.end());	
	return events;
}


void Stage::removeUnit_(Unit * u) {
	u->detachFromCell_();
	unitMap_.erase(unitMap_.find(u->id_));
	turnList_.removeUnit_(u);
}


Unit* Stage::getUnit_(int id) {
	auto elem = unitMap_.find(id);
	if (elem == unitMap_.end()) {
		return NULL;
	}
	return elem->second;
}


Pos Stage::getStartPivot_(int teamID) {
	if (teamID == 0) {
		return Pos(0, 0);
	}
	return Pos(grid_.xMax_, grid_.yMax_);
}


Stage::State_ Stage::getState_() {
	return state_;
}


string Stage::toStr_() {
	string str;
	
	for (auto elem : unitMap_) {		
		Unit* u = elem.second;		
		str += to_string(u->teamID_) + "::"
			+ to_string(u->id_) + " : "
			+ to_string(u->hp_) + '/' + to_string(u->maxHP_) + '\n';
	}
		
	if (currentUnit_) {
		currentUnit_->canAct_();
		str += "\n  UNIT = " + to_string(currentUnit_->id_);
		if (currentUnit_->canMove_()) {
			str += ", Move";
		}
		if (currentUnit_->canAct_()) {
			str += ", Action";
		}
	}
	
	str += '\n' + grid_.toStr_();
	return str;
}


Game::Game() 
{
	gcls_ = GameClasses();
	idman_ = IDManager();
	reset_();
};


void Game::reset_() {
	idman_.reset_();
	stageNumber_ = 0;
	gameState_ = GameStates::front;
}


void Game::loadPlayerUnits_() {
}


void Game::createPlayerUnits_() {
	vector<Item*> items;
	playerUnits_ = UnitGroup();
	playerUnits_.addUnit_(Unit(idman_, "p0", &gcls_.getUnitClass("UW0"), items, TEAM_P));
	playerUnits_.addUnit_(Unit(idman_, "p1", &gcls_.getUnitClass("UA0"), items, TEAM_P));
	playerUnits_.addUnit_(Unit(idman_, "p2", &gcls_.getUnitClass("UM0"), items, TEAM_P));
}


void Game::createOtherUnits_() {
	vector<Item*> items;
	otherUnits_ = UnitGroup();
	string name;
	UnitClass* ucl;

	for (int i = 0; i <= stageNumber_; i++) {
		name = "o" + to_string(i);
		int r = rand() % 3;
		if (r == 0) {
			ucl = &gcls_.getUnitClass("UW0");
		}
		else if (r == 1) {
			ucl = &gcls_.getUnitClass("UA0");
		}
		else {
			ucl = &gcls_.getUnitClass("UM0");
		}
		otherUnits_.addUnit_(Unit(idman_, name, ucl, items, TEAM_O));
	}
}


void Game::updatePlayerUnits_() {
	for (int id : playerUnits_.getUnitIDs()) {
		Unit* u = playerUnits_.getUnit(id);
		if (!u->isAlive_()) {
			playerUnits_.removeUnit_(*u);
		}
		else {
			u->recover_();			
		}
	}
}


vector<ViewerEvent> Game::setupStage_(){
	updatePlayerUnits_();
	createOtherUnits_();

	map<int,Unit*> unitMap;
	for (int id : playerUnits_.getUnitIDs()) {
		unitMap[id] = playerUnits_.getUnit(id);
	}
	for (int id : otherUnits_.getUnitIDs()) {
		unitMap[id] = otherUnits_.getUnit(id);
	}
	
	st_ = Stage(unitMap, 5);
	st_.initialize_();

	vector<ViewerEvent> events;
	events.push_back(st_.getInitInfo_());

	for (auto evt : st_.updateStage_()) {
		events.push_back(evt);
	}
	
	cout << st_.toStr_();
	return events;
};


void Game::updateView_() {
	cout << string(3, '\n');

	switch (gameState_) {
		case GameStates::front:
		{
			cout << "front page" << endl;;
		}
		break;
		case GameStates::intermission: 
		{
			cout << string(50, '\n');
			cout << stageNumber_ << endl;
		}
		break;
		case GameStates::stage:
		{
			cout << st_.toStr_();
		}
		break;
		case GameStates::stageResult:
		{
			switch (st_.getState_()) {
				case Stage::State_::run:
				{
					cout << "error";
				}
				break;
				case Stage::State_::fin_completed:
				{
					cout << "completed";
				}
				break;
				case Stage::State_::fin_defeated:
				{
					cout << "defeated";
				}
				break;
			}
		}
		break;
		default:
			cout << "error";
	}
}


void Game::onInput_(GameInput in) {
	switch (gameState_) {
		case GameStates::front:
		{			
			createPlayerUnits_();
			gameState_ = GameStates::intermission;
		}
		break;
		case GameStates::intermission:
		{
			for (auto viewerEvent : setupStage_()) {
				viewerEvents_.push(viewerEvent);
			}
			viewerEvents_.push(st_.getCurrentUnitActions_());
			gameState_ = GameStates::stage;
		}
		break;
		case GameStates::stage:
		{			
			for (auto viewerEvent : st_.onEvent_(in.unitEvent_)) {
				viewerEvents_.push(viewerEvent);
			}
			if (st_.getState_() != Stage::State_::run) {
				gameState_ = GameStates::stageResult;
			}
			else {				
				if (!viewerEvents_.empty()){
					viewerEvents_.push(st_.getCurrentUnitActions_());
				}
			}
		}
		break;
		case GameStates::stageResult:
		{
			viewerEvents_.push(ViewerEvent(ViewerEvent::Types_::clear));
			if (st_.getState_() == Stage::State_::fin_completed) {
				gameState_ = GameStates::intermission;
				stageNumber_ += 1;
			}
			else {
				gameState_ = GameStates::front;
				reset_();
			}
		}
		break;
	}

	updateView_();
}
