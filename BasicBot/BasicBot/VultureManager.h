#pragma once;

#include "Common.h"
#include "MicroManager.h"

namespace MyBot
{
	class VultureManager : public MicroManager
	{
	public:
		//@���ֳ� ������
		int pathTileCount = 0;		
		std::vector<BWAPI::Position> chokePointForVulture;
		void VultureManager::miningPositionSetting();
		bool miningOn = false;
		VultureManager();
		void executeMicro(const BWAPI::Unitset & targets);
		BWAPI::Unit chooseTarget(BWAPI::Unit vultureUnit, const BWAPI::Unitset & targets, std::map<BWAPI::Unit, int> & numTargeting);
		BWAPI::Unit closestvultureUnit(BWAPI::Unit target, std::set<BWAPI::Unit> & vultureUnitsToAssign);
		std::pair<BWAPI::Unit, BWAPI::Unit> findClosestUnitPair(const BWAPI::Unitset & attackers, const BWAPI::Unitset & targets);

		int getAttackPriority(BWAPI::Unit vultureUnit, BWAPI::Unit target);
		BWAPI::Unit getTarget(BWAPI::Unit vultureUnit, const BWAPI::Unitset & targets);

		void assignTargetsNew(const BWAPI::Unitset & targets);
		void assignTargetsOld(const BWAPI::Unitset & targets);
	};
}