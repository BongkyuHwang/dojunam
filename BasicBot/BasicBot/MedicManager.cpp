#include "MedicManager.h"
#include "CommandUtil.h"

using namespace MyBot;

MedicManager::MedicManager() 
{ 
}

void MedicManager::executeMicro(const BWAPI::Unitset & targets) 
{
	const BWAPI::Unitset & medics = getUnits();

	//@���ֳ� ������ �ĺ� ������ ���� ���Ѵ�.
	int countCB = 0;
	// create a set of all medic targets
	BWAPI::Unitset medicTargets;	
    for (auto & unit : BWAPI::Broodwar->self()->getUnits())
    {
		
		if (unit->getHitPoints() < unit->getType().maxHitPoints() && !unit->getType().isMechanical() && !unit->getType().isBuilding() )// && unit->getOrderTargetPosition().getDistance(order.getPosition()) < 500 )
		//if (!unit->getType().isWorker() && !unit->getType().isMechanical() && !unit->getType().isBuilding()) //@���ֳ� ������ �߰��ߴٰ� ����
        {
            medicTargets.insert(unit);
        }
		if (unit->getHitPoints() > 0 && unit->getType().canAttack() && !unit->getType().isMechanical() && !unit->getType().isBuilding() 
			//&& unit->getOrderTargetPosition().getDistance(order.getPosition()) < 500
			)
		{
			countCB++;
		}
    }
	//std::cout << "MedicManager::executeMicro medicTargets.size(" << medicTargets.size()<< std::endl;	
    BWAPI::Unitset availableMedics(medics);

	//@���ֳ� ������ �߻����غ��� // �� ���̽��� ���� ����� melee ������ ������
	//double closestMeleeUnitToEBDist = std::numeric_limits<double>::infinity();
	//BWAPI::Position closestMeleeP;
	

    // for each target, send the closest medic to heal it
    for (auto & target : medicTargets)
    {
		//closestMeleeP = target->getPosition();
		//_mainBaseLocations[selfPlayer]
		//int gDist = target->getDistance(InformationManager::Instance().getMainBaseLocation(InformationManager::Instance().enemyPlayer)->getPosition());
		//
		//	if (gDist < closestMeleeUnitToEBDist && target->getType() !=BWAPI::UnitTypes::Terran_Medic)
		//{
		//	std::cout << "in 414141 [" << gDist << std::endl;
		//	closestMeleeP = target->getPosition();
		//	std::cout << "in x [" << closestMeleeP.x << "] y[ " << closestMeleeP.y << std::endl;
		//	closestMeleeUnitToEBDist = gDist;
		//}
		
        // only one medic can heal a target at a time
		if (target->isBeingHealed() && countCB > 0)
        {
            continue;
        }

        double closestMedicDist = std::numeric_limits<double>::infinity();
        BWAPI::Unit closestMedic = nullptr;
		
        for (auto & medic : availableMedics)
        {
			if (countCB == 0){			
				BWAPI::UnitCommand currentCommand(medic->getLastCommand());
				Micro::SmartMove(medic, currentCommand.getTargetPosition());
				continue;
			}

            double dist = medic->getDistance(target);

            if (!closestMedic || (dist < closestMedicDist))
            {
                closestMedic = medic;
                closestMedicDist = dist;
            }
        }

        // if we found a medic, send it to heal the target
        if (closestMedic)
        {
            closestMedic->useTech(BWAPI::TechTypes::Healing, target);
            availableMedics.erase(closestMedic);
        }
        // otherwise we didn't find a medic which means they're all in use so break
        else
        {
            break;
        }
    }
	
    // the remaining medics should head to the squad order position
    for (auto & medic : availableMedics)
    {
		//if (closestMeleeUnitToEBDist < 
		//	MapTools::Instance().getGroundDistance(medic->getPosition(), InformationManager::Instance().getMainBaseLocation(InformationManager::Instance().enemyPlayer)->getPosition()) )
		//{
		//	std::cout << "closestMeleeUnitToEBDist " << closestMeleeUnitToEBDist << std::endl;
		//	Micro::SmartAttackMove(medic, closestMeleeP);
		//}
		//else
		//@���ֳ� ������ ��� �޵��� ����Ȥ�� �ĺ� �߽����� �����ش�.  �� �ȵǰڴ� ������ ������ ��ũ����Ʈ�� ���ƿ´�
		if (countCB == 0)
		{			
			BWAPI::UnitCommand currentCommand(medic->getLastCommand());
			Micro::SmartMove(medic, currentCommand.getTargetPosition());			
		}
		else
		{
			//if (medic->getDistance(order.getPosition()) > 100 && order.getFarUnit()->getDistance(medic->getPosition()) > BWAPI::UnitTypes::Terran_Siege_Tank_Siege_Mode.groundWeapon().maxRange() - 100 && order.getFarUnit()->getID() != medic->getID())
			{
				//std::cout << "MedicManager::executeMicro 115 done " << std::endl;
				BWAPI::Broodwar->drawTextMap(medic->getPosition() + BWAPI::Position(0, 30), "%s", "Marking Front ");
				if (order.getClosestUnit())
				{
					//std::cout << "MedicManager::executeMicro 119 done " << std::endl;
					Micro::SmartMove(medic, order.getClosestUnit()->getPosition());
				}
				else
				{
					//std::cout << "MedicManager::executeMicro 124 done " << std::endl;
					BWAPI::UnitCommand currentCommand(medic->getLastCommand());
					Micro::SmartMove(medic, currentCommand.getTargetPosition());
				}
				//std::cout << "MedicManager::executeMicro 121 done " << std::endl;
			}
		}
		//	Micro::SmartAttackMove(medic, InformationManager::Instance().getSecondChokePoint(InformationManager::Instance().selfPlayer)->getCenter());
		//	
    }
	
}