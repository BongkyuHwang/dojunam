﻿#include "Config.h"

namespace Config
{
	// kyj
	namespace Micro
	{
		bool UseSparcraftSimulation = true;
		bool KiteWithRangedUnits = true;
		std::set<BWAPI::UnitType> KiteLongerRangedUnits;
		bool WorkersDefendRush = false;
		int RetreatMeleeUnitShields = 0;
		int RetreatMeleeUnitHP = 0;
		int CombatRadius = 1000;     // radius of combat to consider units for Micro Search
		int CombatRegroupRadius = 300;      // radius of units around frontmost unit we consider in regroup calculation
		int UnitNearEnemyRadius = 600;      // radius to consider a unit 'near' to an enemy unit
	}

	namespace Debug
	{
		//bool DrawGameInfo = true;
		//bool DrawUnitHealthBars = true;
		//bool DrawProductionInfo = true;
		bool DrawBuildOrderSearchInfo = false;
		//bool DrawScoutInfo = false;
		//bool DrawResourceInfo = false;
		//bool DrawWorkerInfo = false;
		bool DrawModuleTimers = false;
		//bool DrawReservedBuildingTiles = false;
		bool DrawCombatSimulationInfo = false;
		//bool DrawBuildingInfo = false;
		//bool DrawMouseCursorInfo = false;
		//bool DrawEnemyUnitInfo = false;
		//bool DrawBWTAInfo = false;
		//bool DrawMapGrid = false;
		//bool DrawUnitTargetInfo = false;
		bool DrawSquadInfo = false;
		bool DrawBOSSStateInfo = false;
		bool PrintModuleTimeout = false;
		bool createTrackingLog = false;

		std::string ErrorLogFilename = "UAB_ErrorLog.txt";
		bool LogAssertToErrorFile = false;

		BWAPI::Color ColorLineTarget = BWAPI::Colors::White;
		BWAPI::Color ColorLineMineral = BWAPI::Colors::Cyan;
		BWAPI::Color ColorUnitNearEnemy = BWAPI::Colors::Red;
		BWAPI::Color ColorUnitNotNearEnemy = BWAPI::Colors::Green;

		bool DrawGameInfo = true;
		bool DrawUnitHealthBars = true;
		bool DrawProductionInfo = true;
		bool DrawScoutInfo = true;
		bool DrawResourceInfo = true;
		bool DrawWorkerInfo = true;
		bool DrawReservedBuildingTiles = true;
		bool DrawBuildingInfo = true;
		bool DrawMouseCursorInfo = true;
		bool DrawEnemyUnitInfo = true;
		bool DrawBWTAInfo = true;
		bool DrawMapGrid = false;
		bool DrawUnitTargetInfo = true;


	}
	namespace Strategy
	{
		std::string ProtossStrategyName = "Protoss_ZealotRush";
		std::string TerranStrategyName = "Terran_MarineRush";
		std::string ZergStrategyName = "Zerg_3HatchMuta";
		std::string StrategyName = "Protoss_ZealotRush";
		std::string ReadDir = "bwapi-data/read/";
		std::string WriteDir = "bwapi-data/write/";
		bool GasStealWithScout = false;
		bool ScoutHarassEnemy = true;
		bool UseEnemySpecificStrategy = false;
		bool FoundEnemySpecificStrategy = false;
	}
	namespace Modules
	{
		// the default tournament bot modules
		bool UsingGameCommander = true;     // toggle GameCommander, effectively UAlbertaBot
		bool UsingScoutManager = true;
		bool UsingCombatCommander = true;
		bool UsingBuildOrderSearch = true;     // toggle use of Build Order Search, currently no backup
		bool UsingAutoObserver = false;
		bool UsingStrategyIO = false;    // toggle the use of file io for strategy
		bool UsingUnitCommandManager = false;    // handles all unit commands

		// extra things, don't enable unless you know what they are
		bool UsingBuildOrderDemo = false;
	}

	/////////////////////////////////////////////////////////////
	namespace BotInfo
	{
		std::string BotName = "MyBot";
		std::string BotAuthors = "NoName";
	}

    namespace Files
    {
		std::string LogFilename = "log.txt";
		std::string ReadDirectory = "bwapi-data\\AI\\MyBot\\read\\";
		std::string WriteDirectory = "bwapi-data\\AI\\MyBot\\write\\";		
    }

	namespace BWAPIOptions
	{
		int SetLocalSpeed = 0;
		int SetFrameSkip = 0;
		bool EnableUserInput = true;
		bool EnableCompleteMapInformation = false;
		bool SetGUI = true;
	}
	
	namespace Tools
	{
		extern int MAP_GRID_SIZE = 32;      
	}

	namespace Macro
	{
		int WorkersPerRefinery = 3;
		int BuildingSpacing = 2;
		int BuildingResourceDepotSpacing = 0;
		int BuildingPylonEarlyStageSpacing = 4;
		int BuildingPylonSpacing = 2;
		int BuildingSupplyDepotSpacing = 0;
		int BuildingDefenseTowerSpacing = 0;
	}

	/*namespace Debug
	{
		bool DrawGameInfo = true;
		bool DrawUnitHealthBars = true;
		bool DrawProductionInfo = true;
		bool DrawScoutInfo = true;
		bool DrawResourceInfo = true;
		bool DrawWorkerInfo = true;
		bool DrawReservedBuildingTiles = true;
		bool DrawBuildingInfo = true;
		bool DrawMouseCursorInfo = true;
		bool DrawEnemyUnitInfo = true;
		bool DrawBWTAInfo = false;
		bool DrawMapGrid = false;
		bool DrawUnitTargetInfo = true;
	}*/

}