#include "Squad.h"
#include "CommandUtil.h"

using namespace MyBot;

Squad::Squad()
	: _lastRetreatSwitch(0)
	, _lastRetreatSwitchVal(false)
	, _priority(0)
	, _name("Default")
{
	int a = 10;
}

Squad::Squad(const std::string & name, SquadOrder order, size_t priority)
	: _name(name)
	, _order(order)
	, _lastRetreatSwitch(0)
	, _lastRetreatSwitchVal(false)
	, _priority(priority)
{
}

Squad::~Squad()
{
	clear();
}

void Squad::update()
{
	
	// update all necessary unit information within this squad
	updateUnits();
	
	// determine whether or not we should regroup
	bool needToRegroup = needsToRegroup();
	needToRegroup = false;
	// draw some debug info
	if (Config::Debug::DrawSquadInfo && _order.getType() == SquadOrderTypes::Attack)
	{
		BWAPI::Broodwar->drawTextScreen(200, 350, "%s", _regroupStatus.c_str());

		BWAPI::Unit closest = unitClosestToEnemy();
	}

	// if we do need to regroup, do it
	if (needToRegroup)
	{
		BWAPI::Position regroupPosition = calcRegroupPosition();
	
		if (Config::Debug::DrawCombatSimulationInfo)
		{
			BWAPI::Broodwar->drawTextScreen(200, 150, "REGROUP");
		}
	
		BWAPI::Broodwar->drawCircleMap(regroupPosition.x, regroupPosition.y, 30, BWAPI::Colors::Purple, true);
	
		_meleeManager.regroup(regroupPosition);
		_rangedManager.regroup(regroupPosition);
		_tankManager.regroup(regroupPosition);
		_medicManager.regroup(regroupPosition);
		_vultureManager.regroup(regroupPosition);
	}
	else // otherwise, execute micro
	{
		_meleeManager.execute(_order);		
		_rangedManager.execute(_order);
		_vultureManager.execute(_order);
		_medicManager.execute(_order);
		_tankManager.execute(_order);
		_transportManager.update();
		_detectorManager.setUnitClosestToEnemy(unitClosestToEnemy());
		_detectorManager.execute(_order);
	}
}

bool Squad::isEmpty() const
{
	return _units.empty();
}

size_t Squad::getPriority() const
{
	return _priority;
}

void Squad::setPriority(const size_t & priority)
{
	_priority = priority;
}

void Squad::updateUnits()
{
	setAllUnits();
	if (_vultureManager.miningOn == false)
	{
		_vultureManager.miningPositionSetting();
	}
	setNearEnemyUnits();
	addUnitsToMicroManagers();
}

void Squad::setAllUnits()
{
	// clean up the _units vector just in case one of them died
	BWAPI::Unitset goodUnits;
	int maxDist = 0;
	//@도주남 김지훈 메딕이 힐링이 가능한 캐릭터가 몇명인지 확인한다.  의견을 들어보고 적용 여부 결정
	BWAPI::Unitset organicUnits;
	unitFarToOrderPosition = nullptr;
	for (auto & unit : _units)
	{
		if (unit->isCompleted() &&
			unit->getHitPoints() > 0 &&
			unit->exists() &&
			unit->getType() != BWAPI::UnitTypes::Terran_Vulture_Spider_Mine &&
			unit->getPosition().isValid() &&
			unit->getType() != BWAPI::UnitTypes::Unknown)
		{
			if (unit->isLoaded() && unit->getType() != BWAPI::UnitTypes::Terran_Siege_Tank_Tank_Mode)
				continue;

			goodUnits.insert(unit);
			if ((maxDist < unit->getDistance(_order.getPosition()) || maxDist == 0) && unit->getType() != BWAPI::UnitTypes::Terran_Vulture)
			{
				maxDist = unit->getDistance(_order.getPosition());
				unitFarToOrderPosition = unit;
			}
			if (unit->getType().isOrganic() && unit->getType() != BWAPI::UnitTypes::Terran_Medic)
			{
				organicUnits.insert(unit);
			}
		}
	}	
	_units.clear();
	_units = goodUnits;
	if (organicUnits.size() == 0)
	{
		goodUnits.clear();
		for (auto & unit : _units)
		{
			if (unit->getType() == BWAPI::UnitTypes::Terran_Medic && unit != unitFarToOrderPosition)
			{
				unit->move(InformationManager::Instance().getSecondChokePoint(BWAPI::Broodwar->self())->getCenter());
			}
			else
				goodUnits.insert(unit);
		}
		_units.clear();
		_units = goodUnits;
	}	
	else
		_order.setOrganicUnits(organicUnits);

	_order.setFarUnit(unitFarToOrderPosition);	
	_order.setClosestUnit(unitClosestToEnemyForOrder());	
	if (_order.getClosestUnit() == nullptr)
		_order.setClosestUnit(nullptr);
}

void Squad::setNearEnemyUnits()
{
	_nearEnemy.clear();
	for (auto & unit : _units)
	{
		int x = unit->getPosition().x;
		int y = unit->getPosition().y;

		int left = unit->getType().dimensionLeft();
		int right = unit->getType().dimensionRight();
		int top = unit->getType().dimensionUp();
		int bottom = unit->getType().dimensionDown();

		_nearEnemy[unit] = unitNearEnemy(unit);
		if (_nearEnemy[unit])
		{
			if (Config::Debug::DrawSquadInfo) BWAPI::Broodwar->drawBoxMap(x - left, y - top, x + right, y + bottom, Config::Debug::ColorUnitNearEnemy);
		}
		else
		{
			if (Config::Debug::DrawSquadInfo) BWAPI::Broodwar->drawBoxMap(x - left, y - top, x + right, y + bottom, Config::Debug::ColorUnitNotNearEnemy);
		}
	}
}

void Squad::addUnitsToMicroManagers()
{
	BWAPI::Unitset meleeUnits;
	BWAPI::Unitset rangedUnits;
	BWAPI::Unitset detectorUnits;
	BWAPI::Unitset transportUnits;
	BWAPI::Unitset tankUnits;
	BWAPI::Unitset medicUnits;
	BWAPI::Unitset vultureUnits;
	// add _units to micro managers
	for (auto & unit : _units)
	{
		if (unit->isCompleted() && unit->getHitPoints() > 0 && unit->exists())
		{
			// select dector _units
			if (unit->getType() == BWAPI::UnitTypes::Terran_Medic)
			{
				medicUnits.insert(unit);
			}
			else if (unit->getType() == BWAPI::UnitTypes::Terran_Siege_Tank_Siege_Mode || unit->getType() == BWAPI::UnitTypes::Terran_Siege_Tank_Tank_Mode)
			{
				tankUnits.insert(unit);
			}
			//@도주남 김지훈
			else if (unit->getType() == BWAPI::UnitTypes::Terran_Vulture)
			{
				vultureUnits.insert(unit);
			}
			else if (unit->getType().isDetector() && !unit->getType().isBuilding())
			{
				detectorUnits.insert(unit);
			}
			// select transport _units
			else if (unit->getType() == BWAPI::UnitTypes::Protoss_Shuttle || unit->getType() == BWAPI::UnitTypes::Terran_Dropship)
			{
				transportUnits.insert(unit);
			}			
			// select ranged _units
			else if ((unit->getType().groundWeapon().maxRange() > 32) || (unit->getType() == BWAPI::UnitTypes::Protoss_Reaver) || (unit->getType() == BWAPI::UnitTypes::Zerg_Scourge))
			{
				rangedUnits.insert(unit);
			}
			// select melee _units
			else if (unit->getType().groundWeapon().maxRange() <= 32)
			{
				meleeUnits.insert(unit);
			}
		}
	}

	_meleeManager.setUnits(meleeUnits);
	_rangedManager.setUnits(rangedUnits);
	_detectorManager.setUnits(detectorUnits);
	_transportManager.setUnits(transportUnits);
	_tankManager.setUnits(tankUnits);
	_vultureManager.setUnits(vultureUnits);
	//_medicManager.meleeUnitsetCenterP = _meleeManager.calcCenter();
	_medicManager.setUnits(medicUnits);

}

// calculates whether or not to regroup
bool Squad::needsToRegroup()
{
	if (!Config::Micro::UseSparcraftSimulation)
	{
		return false;
	}

	// if we are not attacking, never regroup
	if (_units.empty() || (_order.getType() != SquadOrderTypes::Attack))
	{
		_regroupStatus = std::string("\x04 No combat units available");
		return false;
	}

	BWAPI::Unit unitClosest = unitClosestToEnemy();

	if (!unitClosest)
	{
		_regroupStatus = std::string("\x04 No closest unit");
		return false;
	}

	// if none of our units are in attack range of any enemy units, don't retreat
	std::vector<UnitInfo> enemyCombatUnits;
	const auto & enemyUnitInfo = InformationManager::Instance().getUnitInfo(BWAPI::Broodwar->enemy());

	bool anyInRange = false;
	for (const auto & eui : enemyUnitInfo)
	{
		bool inRange = false;
		for (const auto & u : _units)
		{
			int range = UnitUtil::GetAttackRange(eui.second.type, u->getType());

			if (range + 128 >= eui.second.lastPosition.getDistance(u->getPosition()))
			{
				inRange = true;
				break;
			}
		}

		if (inRange)
		{
			anyInRange = true;
			break;
		}
	}

	if (!anyInRange)
	{
		_regroupStatus = std::string("\x04 No enemy units in attack range");
		return false;
	}

	//SparCraft::ScoreType score = 0;
	//kyj
	int score = 100;

	//do the SparCraft Simulation!
	//kyj//CombatSimulation sim;


	//kyj//sim.setCombatUnits(unitClosest->getPosition(), Config::Micro::CombatRegroupRadius);
	//kyj//score = sim.simulateCombat();

	// if we are DT rushing and we haven't lost a DT yet, no retreat!
	if (Config::Strategy::StrategyName == "Protoss_DTRush" && (BWAPI::Broodwar->self()->deadUnitCount(BWAPI::UnitTypes::Protoss_Dark_Templar) == 0))
	{
		_regroupStatus = std::string("\x04 DARK TEMPLAR HOOOOO!");
		return false;
	}

	bool retreat = score < 0;
	int switchTime = 100;
	bool waiting = false;

	// we should not attack unless 5 seconds have passed since a retreat
	if (retreat != _lastRetreatSwitchVal)
	{
		if (!retreat && (BWAPI::Broodwar->getFrameCount() - _lastRetreatSwitch < switchTime))
		{
			waiting = true;
			retreat = _lastRetreatSwitchVal;
		}
		else
		{
			waiting = false;
			_lastRetreatSwitch = BWAPI::Broodwar->getFrameCount();
			_lastRetreatSwitchVal = retreat;
		}
	}

	if (retreat)
	{
		_regroupStatus = std::string("\x04 Retreat - simulation predicts defeat");
	}
	else
	{
		_regroupStatus = std::string("\x04 Attack - simulation predicts success");
	}

	return retreat;
}

void Squad::setSquadOrder(const SquadOrder & so)
{
	_order = so;
	updateUnits();
}

bool Squad::containsUnit(BWAPI::Unit u) const
{
	return _units.contains(u);
}

void Squad::clear()
{
	for (auto & unit : getUnits())
	{
		if (unit->getType().isWorker())
		{
			WorkerManager::Instance().finishedWithWorker(unit);
		}
	}

	_units.clear();
}

bool Squad::unitNearEnemy(BWAPI::Unit unit)
{
	assert(unit);

	BWAPI::Unitset enemyNear;

	MapGrid::Instance().getUnitsNear(enemyNear, unit->getPosition(), 400, false, true);

	return enemyNear.size() > 0;
}

BWAPI::Position Squad::calcCenter()
{
	if (_units.empty())
	{
		if (Config::Debug::DrawSquadInfo)
		{
			BWAPI::Broodwar->printf("Squad::calcCenter() called on empty squad");
		}
		return BWAPI::Position(0, 0);
	}

	BWAPI::Position accum(0, 0);
	for (auto & unit : _units)
	{
		accum += unit->getPosition();
		//std::cout << " BWAPI::Position Squad::calcCenter()   " << accum.x << " / " << accum.y << std::endl;
	}
	return BWAPI::Position(accum.x / _units.size(), accum.y / _units.size());
}

BWAPI::Position Squad::calcRegroupPosition()
{
	BWAPI::Position regroup(0, 0);

	int minDist = 100000;

	for (auto & unit : _units)
	{
		if (!_nearEnemy[unit])
		{
			int dist = unit->getDistance(_order.getPosition());
			if (dist < minDist)
			{
				minDist = dist;
				regroup = unit->getPosition();
			}
		}
	}

	if (regroup == BWAPI::Position(0, 0))
	{
		return BWTA::getRegion(BWTA::getStartLocation(BWAPI::Broodwar->self())->getTilePosition())->getCenter();
	}
	else
	{
		return regroup;
	}
}

BWAPI::Unit Squad::unitClosestToEnemy()
{
	BWAPI::Unit closest = nullptr;
	int closestDist = 100000;

	for (auto & unit : _units)
	{
		if (unit->getType() == BWAPI::UnitTypes::Protoss_Observer)
		{
			continue;
		}

		// the distance to the order position
		int dist = MapTools::Instance().getGroundDistance(unit->getPosition(), _order.getPosition());

		if (dist != -1 && (!closest || dist < closestDist))
		{
			closest = unit;
			closestDist = dist;
		}
	}

	if (!closest)
	{
		for (auto & unit : _units)
		{
			if (unit->getType() == BWAPI::UnitTypes::Protoss_Observer)
			{
				continue;
			}

			// the distance to the order position
			int dist = unit->getDistance(BWAPI::Position(BWAPI::Broodwar->enemy()->getStartLocation()));

			if (dist != -1 && (!closest || dist < closestDist))
			{
				closest = unit;
				closestDist = dist;
			}
		}
	}

	return closest;
}

int Squad::squadUnitsNear(BWAPI::Position p)
{
	int numUnits = 0;

	for (auto & unit : _units)
	{
		if (unit->getDistance(p) < 600)
		{
			numUnits++;
		}
	}

	return numUnits;
}

const BWAPI::Unitset & Squad::getUnits() const
{
	return _units;
}

const SquadOrder & Squad::getSquadOrder()	const
{
	return _order;
}

void Squad::addUnit(BWAPI::Unit u)
{
	_units.insert(u);
}

void Squad::removeUnit(BWAPI::Unit u)
{
	_units.erase(u);
}

const std::string & Squad::getName() const
{
	return _name;
}


BWAPI::Unit Squad::unitClosestToEnemyForOrder()
{
	BWAPI::Unit closest = nullptr;
	int closestDist = 100000;

	for (auto & unit : _units)
	{
		if (unit->getType() == BWAPI::UnitTypes::Terran_Medic)
		{
			continue;
		}
		if (unit->getHitPoints() <= 0)
			continue;
		// the distance to the order position
		int dist = MapTools::Instance().getGroundDistance(unit->getPosition(), _order.getPosition());

		if (dist != -1 && (!closest || dist < closestDist))
		{
			closest = unit;
			closestDist = dist;
		}
	}

	if (!closest)
	{
		for (auto & unit : _units)
		{
			if (unit->getType() == BWAPI::UnitTypes::Terran_Medic)
			{
				continue;
			}
			if (unit->getHitPoints() <= 0)
				continue;

			// the distance to the order position
			int dist = unit->getDistance(BWAPI::Position(BWAPI::Broodwar->enemy()->getStartLocation()));

			if (dist != -1 && (!closest || dist < closestDist))
			{
				closest = unit;
				closestDist = dist;
			}
		}
	}
	if (!closest)
	{
		for (auto & unit : _units)
		{
			if (unit->getHitPoints() <= 0)
				continue;

			// the distance to the order position
			int dist = MapTools::Instance().getGroundDistance(unit->getPosition(), _order.getPosition());

			if (dist != -1 && (!closest || dist < closestDist))
			{
				closest = unit;
				closestDist = dist;
			}
		}
	}
	return closest;
}