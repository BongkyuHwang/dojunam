#include "RangedManager.h"
#include "CommandUtil.h"

using namespace MyBot;

RangedManager::RangedManager() 
{ 
	bunkerNum = 0; marinInBunkerNum = 0;	
}

RangedManager & RangedManager::Instance()
{
	static RangedManager instance;
	return instance;
}

void RangedManager::executeMicro(const BWAPI::Unitset & targets) 
{
	checkBunkerNum();
	assignTargetsOld(targets);
}

void RangedManager::checkBunkerNum()
{
	bunkerNum = UnitUtil::GetAllUnitCount(BWAPI::UnitTypes::Terran_Bunker);	
	if (bunkerNum > 0)
	{
		bunkerUnit = UnitUtil::GetClosestUnitTypeToTarget(BWAPI::UnitTypes::Terran_Bunker, BWAPI::Position(BWAPI::Broodwar->self()->getStartLocation()));	
		if (bunkerUnit->getLoadedUnits().size() >= 3)
			bunkerNum = 0;
	}
}

void RangedManager::assignTargetsOld(const BWAPI::Unitset & targets)
{
    const BWAPI::Unitset & rangedUnits = getUnits();
	
	// figure out targets
	BWAPI::Unitset rangedUnitTargets;
    std::copy_if(targets.begin(), targets.end(), std::inserter(rangedUnitTargets, rangedUnitTargets.end()), [](BWAPI::Unit u){ return u->isVisible(); });
	
    for (auto & rangedUnit : rangedUnits)
	{
		if (order.getType() == SquadOrderTypes::Idle && rangedUnit->getType() == BWAPI::UnitTypes::Terran_Marine
			&& rangedUnit->getHitPoints() == rangedUnit->getType().maxHitPoints())
		{
			if (bunkerNum > 0)
			{				
				rangedUnit->load(bunkerUnit);
				continue;
			}			
		}
		// train sub units such as scarabs or interceptors
		//trainSubUnits(rangedUnit);
		bool nearChokepoint = false;
		for (auto & choke : BWTA::getChokepoints())
		{
			//@도주남 김지훈 64 라는 절대적인 수치 기준으로 , choke point 진입여부를 판단하고 있음 , 다른 getDistance 기준 64 미만의 경우
			// 근접해있다고 판단해도 무방할 것으로 보임
			if ((InformationManager::Instance().getSecondChokePoint(BWAPI::Broodwar->enemy()) == choke || InformationManager::Instance().getFirstChokePoint(BWAPI::Broodwar->enemy()) == choke) && choke->getCenter().getDistance(rangedUnit->getPosition()) < 64)
			{
				//std::cout << "choke->getWidth() Tank In Choke Point half " << std::endl;
				BWAPI::Broodwar->drawTextMap(rangedUnit->getPosition() + BWAPI::Position(0, 50), "%s", "In Choke Point");
				nearChokepoint = true;
				break;
			}
		}

		// if the order is to attack or defend
		if (order.getType() == SquadOrderTypes::Attack || order.getType() == SquadOrderTypes::Defend || order.getType() == SquadOrderTypes::Idle)
        {
			// if there are targets
			if (!rangedUnitTargets.empty())
			{
				// find the best target for this zealot
				BWAPI::Unit target = getTarget(rangedUnit, rangedUnitTargets);
                
                if (target && Config::Debug::DrawUnitTargetInfo) 
	            {
		            BWAPI::Broodwar->drawLineMap(rangedUnit->getPosition(), rangedUnit->getTargetPosition(), BWAPI::Colors::Purple);
	            }

				if (rangedUnit->getStimTimer() == 0
					&& rangedUnit->getType() == BWAPI::UnitTypes::Terran_Marine
					&& rangedUnit->getHitPoints() == rangedUnit->getType().maxHitPoints() 
					&& //@도주남 김지훈 상대가 공격 범위에 들어오면  스팀팩을 사용한다.
					target->getPosition().getDistance(rangedUnit->getPosition()) < rangedUnit->getType().groundWeapon().maxRange() + 32 )
				{
					rangedUnit->useTech(BWAPI::TechTypes::Stim_Packs);
				}
				else if (rangedUnit->getStimTimer() > 0 && rangedUnit->getType() == BWAPI::UnitTypes::Terran_Marine)
				{
					std::string stimPacksUsed = "stimPacks On";
					BWAPI::Broodwar->drawTextMap(rangedUnit->getPosition().x , rangedUnit->getPosition().y + 50, "%s", stimPacksUsed.c_str());
				}

				// attack it
                if (Config::Micro::KiteWithRangedUnits)
                {
                    if (rangedUnit->getType() == BWAPI::UnitTypes::Zerg_Mutalisk || rangedUnit->getType() == BWAPI::UnitTypes::Terran_Vulture)
                    {
				        Micro::MutaDanceTarget(rangedUnit, target);
                    }
                    else
                    {
                        Micro::SmartKiteTarget(rangedUnit, target);
                    }
                }
                else
                {
                    Micro::SmartAttackUnit(rangedUnit, target);
                }
			}
			// if there are no targets
			else
			{
				//if (rangedUnit->getDistance(order.getPosition()) > 100 
				//	&& order.getFarUnit() != nullptr
				//	&& order.getFarUnit()->getDistance(rangedUnit->getPosition()) > BWAPI::UnitTypes::Terran_Siege_Tank_Siege_Mode.groundWeapon().maxRange() - 86
				//	//&& order.getFarUnit()->getDistance(rangedUnit->getPosition()) < BWAPI::UnitTypes::Terran_Siege_Tank_Siege_Mode.groundWeapon().maxRange() + 500
				//	&& order.getFarUnit()->getID() != rangedUnit->getID()
				//	&& order.getStatus() != "Move Out"
				//	&& !nearChokepoint
				//	)
				if (
					order.getClosestUnit() != nullptr
					&& order.getClosestUnit() == rangedUnit
					&& !nearChokepoint
					&& order.getFarUnit() != nullptr
					&& order.getFarUnit()->getDistance(rangedUnit->getPosition()) > BWAPI::UnitTypes::Terran_Siege_Tank_Siege_Mode.groundWeapon().maxRange() - 64
					&& order.getFarUnit() != rangedUnit
					)
				{
					//std::cout << "Marin  " << rangedUnit->getID() << std::endl;
					//Micro::SmartMove(rangedUnit, order.getFarUnit()->getPosition() - rangedUnit->getPosition() + rangedUnit->getPosition());
					BWAPI::Broodwar->drawTextMap(rangedUnit->getPosition() + BWAPI::Position(0, 30), "%s", "Hold On Position");
					rangedUnit->holdPosition();
				}
				else
				// if we're not near the order position
				//if (rangedUnit->getDistance(order.getPosition()) > 20)
				{
					// move to it
					Micro::SmartAttackMove(rangedUnit, order.getPosition());
				}
			}
		}
	}
}

std::pair<BWAPI::Unit, BWAPI::Unit> RangedManager::findClosestUnitPair(const BWAPI::Unitset & attackers, const BWAPI::Unitset & targets)
{
    std::pair<BWAPI::Unit, BWAPI::Unit> closestPair(nullptr, nullptr);
    double closestDistance = std::numeric_limits<double>::max();

    for (auto & attacker : attackers)
    {
        BWAPI::Unit target = getTarget(attacker, targets);
        double dist = attacker->getDistance(attacker);

        if (!closestPair.first || (dist < closestDistance))
        {
            closestPair.first = attacker;
            closestPair.second = target;
            closestDistance = dist;
        }
    }

    return closestPair;
}

// get a target for the zealot to attack
BWAPI::Unit RangedManager::getTarget(BWAPI::Unit rangedUnit, const BWAPI::Unitset & targets)
{
	int bestPriorityDistance = 1000000;
    int bestPriority = 0;
    
    double bestLTD = 0;

    int highPriority = 0;
	double closestDist = std::numeric_limits<double>::infinity();
	BWAPI::Unit closestTarget = nullptr;

    for (const auto & target : targets)
    {
        double distance         = rangedUnit->getDistance(target);
        double LTD              = UnitUtil::CalculateLTD(target, rangedUnit);
        int priority            = getAttackPriority(rangedUnit, target);
        bool targetIsThreat     = LTD > 0;
        
		if (!closestTarget || (priority > highPriority) || (priority == highPriority && distance < closestDist))
		{
			closestDist = distance;
			highPriority = priority;
			closestTarget = target;
		}       
    }

    return closestTarget;
}

	// get the attack priority of a type in relation to a zergling
int RangedManager::getAttackPriority(BWAPI::Unit rangedUnit, BWAPI::Unit target) 
{
	BWAPI::UnitType rangedType = rangedUnit->getType();
	BWAPI::UnitType targetType = target->getType();

    
    if (rangedUnit->getType() == BWAPI::UnitTypes::Zerg_Scourge)
    {
        if (target->getType() == BWAPI::UnitTypes::Protoss_Carrier)
        {
            
            return 100;
        }

        if (target->getType() == BWAPI::UnitTypes::Protoss_Corsair)
        {
            return 90;
        }
    }

	bool isThreat = rangedType.isFlyer() ? targetType.airWeapon() != BWAPI::WeaponTypes::None : targetType.groundWeapon() != BWAPI::WeaponTypes::None;

    if (target->getType().isWorker())
    {
        isThreat = false;
    }

    if (target->getType() == BWAPI::UnitTypes::Zerg_Larva || target->getType() == BWAPI::UnitTypes::Zerg_Egg)
    {
        return 0;
    }

    if (rangedUnit->isFlying() && target->getType() == BWAPI::UnitTypes::Protoss_Carrier)
    {
        return 101;
    }

    // if the target is building something near our base something is fishy
    BWAPI::Position ourBasePosition = BWAPI::Position(BWAPI::Broodwar->self()->getStartLocation());
    if (target->getType().isWorker() && (target->isConstructing() || target->isRepairing()) && target->getDistance(ourBasePosition) < 1200)
    {
        return 100;
    }

    if (target->getType().isBuilding() && (target->isCompleted() || target->isBeingConstructed()) && target->getDistance(ourBasePosition) < 1200)
    {
        return 90;
    }
    
	// highest priority is something that can attack us or aid in combat
    if (targetType ==  BWAPI::UnitTypes::Terran_Bunker || isThreat)
    {
        return 11;
    }
	// next priority is worker
	else if (targetType.isWorker()) 
	{
        if (rangedUnit->getType() == BWAPI::UnitTypes::Terran_Vulture)
        {
            return 11;
        }

  		return 11;
	}
    // next is special buildings
	else if (targetType == BWAPI::UnitTypes::Zerg_Spawning_Pool)
	{
		return 5;
	}
	// next is special buildings
	else if (targetType == BWAPI::UnitTypes::Protoss_Pylon)
	{
		return 5;
	}
	// next is buildings that cost gas
	else if (targetType.gasPrice() > 0)
	{
		return 4;
	}
	else if (targetType.mineralPrice() > 0)
	{
		return 3;
	}
	// then everything else
	else
	{
		return 1;
	}
}

BWAPI::Unit RangedManager::closestrangedUnit(BWAPI::Unit target, std::set<BWAPI::Unit> & rangedUnitsToAssign)
{
	double minDistance = 0;
	BWAPI::Unit closest = nullptr;

	for (auto & rangedUnit : rangedUnitsToAssign)
	{
		double distance = rangedUnit->getDistance(target);
		if (!closest || distance < minDistance)
		{
			minDistance = distance;
			closest = rangedUnit;
		}
	}
	
	return closest;
}


// still has bug in it somewhere, use Old version
void RangedManager::assignTargetsNew(const BWAPI::Unitset & targets)
{
    const BWAPI::Unitset & rangedUnits = getUnits();

	// figure out targets
	BWAPI::Unitset rangedUnitTargets;
    std::copy_if(targets.begin(), targets.end(), std::inserter(rangedUnitTargets, rangedUnitTargets.end()), [](BWAPI::Unit u){ return u->isVisible(); });

    BWAPI::Unitset rangedUnitsToAssign(rangedUnits);
    std::map<BWAPI::Unit, int> attackersAssigned;

    for (auto & unit : rangedUnitTargets)
    {
        attackersAssigned[unit] = 0;
    }

    // keep assigning targets while we have attackers and targets remaining
    while (!rangedUnitsToAssign.empty() && !rangedUnitTargets.empty())
    {
        auto attackerAssignment = findClosestUnitPair(rangedUnitsToAssign, rangedUnitTargets);
        BWAPI::Unit & attacker = attackerAssignment.first;
        BWAPI::Unit & target = attackerAssignment.second;

        UAB_ASSERT_WARNING(attacker, "We should have chosen an attacker!");

        if (!attacker)
        {
            break;
        }

        if (!target)
        {
            Micro::SmartAttackMove(attacker, order.getPosition());
            continue;
        }

        if (Config::Micro::KiteWithRangedUnits)
        {
            if (attacker->getType() == BWAPI::UnitTypes::Zerg_Mutalisk || attacker->getType() == BWAPI::UnitTypes::Terran_Vulture)
            {
			    Micro::MutaDanceTarget(attacker, target);
            }
            else
            {
                Micro::SmartKiteTarget(attacker, target);
            }
        }
        else
        {
            Micro::SmartAttackUnit(attacker, target);
        }

        // update the number of units assigned to attack the target we found
        int & assigned = attackersAssigned[attackerAssignment.second];
        assigned++;

        // if it's a small / fast unit and there's more than 2 things attacking it already, don't assign more
        if ((target->getType().isWorker() || target->getType() == BWAPI::UnitTypes::Zerg_Zergling) && (assigned > 2))
        {
            rangedUnitTargets.erase(target);
        }
        // if it's a building and there's more than 10 things assigned to it already, don't assign more
        else if (target->getType().isBuilding() && (assigned > 10))
        {
            rangedUnitTargets.erase(target);
        }

        rangedUnitsToAssign.erase(attacker);
    }

    // if there's no targets left, attack move to the order destination
    if (rangedUnitTargets.empty())
    {
        for (auto & unit : rangedUnitsToAssign)    
        {
			if (unit->getDistance(order.getPosition()) > 100)
			{
				// move to it
				Micro::SmartAttackMove(unit, order.getPosition());
			}
        }
    }
}