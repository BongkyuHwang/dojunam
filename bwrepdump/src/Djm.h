#pragma once

#include "Utils.h"
#include <string>

class DjmVO{
public:
	BWAPI::Race race;
	std::map<BWAPI::UnitType, int> selfUnits;
	std::map<BWAPI::UnitType, int> enemyUnits;
	std::size_t mineral;
	std::size_t gas;
	std::size_t supplyTotal;
	std::size_t supplyUsed;

	void init(std::vector<BWAPI::UnitType> &enableUnits);
	bool compare(DjmVO &vo);

	void writeVO(std::ofstream &outFile); //���� ���� �޼ҵ�
	void writeHeader(std::ofstream &outFile); //���� ���� �޼ҵ�
};

class Djm{
public:
	std::ofstream outFile;

	BWAPI::Player self;
	BWAPI::Player enemy;

	Djm();
	~Djm();

	void onFrame();
	void onUnitDestroy(BWAPI::Unit unit); //���� ���ְ����Ҷ� ��������

	BWAPI::UnitType checkUnit(BWAPI::UnitType &in_type); //�����ͼ����� �� ����Ÿ�� üũ
	std::vector<BWAPI::UnitType> enableUnits; //�����ͼ� ���� Ÿ������ ����

	bool error_replay; //�׶��� ���� ���÷��̴� �����Ҷ�

	bool first_time; //ù ������ Ȯ�ο�
	DjmVO old_vo; //���� �����Ӱ� ������ �ƿ� �����ϸ� �������� ����

	BWAPI::Unitset enemyUnitset;
};

