﻿#pragma once
#include "Common.h"
#include "WorkerManager.h"

namespace MyBot
{
	class ExpansionManager{
		std::vector<BWAPI::Unit> expansions;

	public:
		static ExpansionManager & Instance();
		void onSendText(std::string text);
		void update();

		const std::vector<BWAPI::Unit> & getExpansions();
		void ExpansionManager::onUnitDestroy(BWAPI::Unit unit);
		void ExpansionManager::onUnitComplete(BWAPI::Unit unit);
		bool shouldExpandNow();
	};
}