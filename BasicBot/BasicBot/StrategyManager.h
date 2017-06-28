﻿#pragma once

#include "Common.h"

#include "UnitData.h"
#include "BuildOrderQueue.h"
#include "InformationManager.h"
#include "WorkerManager.h"
#include "BuildManager.h"
#include "ConstructionManager.h"
#include "ScoutManager.h"
#include "StrategyManager.h"
#include "BuildOrder.h"

namespace MyBot
{

	typedef std::pair<MetaType, size_t> MetaPair;
	typedef std::vector<MetaPair> MetaPairVector;

	struct Strategy
	{
		std::string next_strategy_name;
		std::string pre_strategy_name;
		std::map<std::string, int> num_unit_limit;
	};

	/// 상황을 판단하여, 정찰, 빌드, 공격, 방어 등을 수행하도록 총괄 지휘를 하는 class
	/// InformationManager 에 있는 정보들로부터 상황을 판단하고, 
	/// BuildManager 의 buildQueue에 빌드 (건물 건설 / 유닛 훈련 / 테크 리서치 / 업그레이드) 명령을 입력합니다.
	/// 정찰, 빌드, 공격, 방어 등을 수행하는 코드가 들어가는 class
	class StrategyManager
	{
		StrategyManager();

		bool isInitialBuildOrderFinished;
		void setInitialBuildOrder();

		void executeWorkerTraining();
		void executeSupplyManagement();
		void executeBasicCombatUnitTraining();

		bool isFullScaleAttackStarted;
		void executeCombat();

		std::map<std::string, Strategy> _strategies;
		BuildOrder _openingBuildOrder;
		std::string _main_strategy;
		const BuildOrder                _emptyBuildOrder;
		const MetaPairVector getProtossBuildOrderGoal() const;
		const MetaPairVector getTerranBuildOrderGoal();
		const MetaPairVector getZergBuildOrderGoal() const;
		void setOpeningBookBuildOrder();
		bool changeMainStrategy(std::map<std::string, int> & numUnits);
		bool checkStrategyLimit(std::string &name, std::map<std::string, int> & numUnits);

		std::map<std::string, std::vector<int>> unit_ratio_table;

	public:
		/// static singleton 객체를 리턴합니다
		static StrategyManager &	Instance();

		/// 경기가 시작될 때 일회적으로 전략 초기 세팅 관련 로직을 실행합니다
		void onStart();

		///  경기가 종료될 때 일회적으로 전략 결과 정리 관련 로직을 실행합니다
		void onEnd(bool isWinner);

		/// 경기 진행 중 매 프레임마다 경기 전략 관련 로직을 실행합니다
		void update();

		const BuildOrder & getOpeningBookBuildOrder() const;
		const MetaPairVector getBuildOrderGoal();
		const bool shouldExpandNow() const;
	};
}
