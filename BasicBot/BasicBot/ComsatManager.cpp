#include "ComsatManager.h"

using namespace MyBot;

ComsatManager::ComsatManager()
{
	clearScanPosition();
	_next_enable_frame = -1;
}

ComsatManager & ComsatManager::Instance()
{
	static ComsatManager instance;
	return instance;
}

void ComsatManager::clearScanPosition(){
	_scan_position.x = -1;
	_scan_position.y = -1;
}

void ComsatManager::setNextEnableFrame(){
	_next_enable_frame = BWAPI::Broodwar->getFrameCount() + 24; //1�� ����
}

void ComsatManager::setScanPosition(){
	//1. ��ĵ ����� ����
	BWAPI::Unitset cloakUnits;
	UnitUtil::getAllCloakUnits(cloakUnits);

	//2. ��� ������ �츮 ������ �󸶳� �ִ��� üũ
	//�����ؾߵǴ� �� ������ ������ �ʹ� ������(���� 3�� �̸�) ��ĵ����
	//Ÿ���� �����Ҽ� �־�� ��(����Ÿ��, �������)
	std::vector<std::pair<BWAPI::Position, double>> cloakUnitInfo;
	for (auto &cu : cloakUnits){
		double tmpDps = 0.0;

		for (auto &u : cu->getUnitsInRadius(_scan_radius_offset)){	
			BWAPI::WeaponType tmpWeapon = UnitUtil::GetWeapon(u, cu); //���ݰ��� ���� �Ǵ� 
			int tmpDistance = u->getDistance(cu); //�Ÿ� �Ǵ�
			if (tmpWeapon != BWAPI::WeaponTypes::None && tmpWeapon != BWAPI::WeaponTypes::Unknown &&
				tmpDistance <= tmpWeapon.maxRange())
			{
				tmpDps += tmpWeapon.damageAmount() / (double)tmpWeapon.damageCooldown();
			}
		}

		if (tmpDps > _scan_dps_offset){
			cloakUnitInfo.push_back(std::make_pair(cu->getPosition(), tmpDps));
		}
	}

	//Ÿ���� ������ ����
	if (cloakUnitInfo.empty()) return;

	//3. �ߺ� ��ĵ���� ����
	//�� ȭ�� �ȿ� ���� ������ ���� ������ ���� (�� ȭ�� 20Ÿ��(32*32))
	int tmpOffset = 20 * 32;
	std::vector<int> tmpInd;
	std::size_t maxCnt = 0;
	BWAPI::Position maxAreaPosition;
	for (std::size_t i = 0; i < cloakUnitInfo.size(); i++){
		for (std::size_t j = 0; j < cloakUnitInfo.size(); j++){
			tmpInd.clear();

			if (cloakUnitInfo[i].first.x <= cloakUnitInfo[j].first.x && cloakUnitInfo[i].first.x + tmpOffset > cloakUnitInfo[j].first.x &&
				cloakUnitInfo[i].first.y <= cloakUnitInfo[j].first.y && cloakUnitInfo[i].first.y + tmpOffset > cloakUnitInfo[j].first.y){
				tmpInd.push_back(j);
			}
		}

		if (tmpInd.size() > maxCnt){
			maxCnt = tmpInd.size();

			//���� ���� ��ġ�� ����Ʈ���� ���
			int tmpX = 0;
			int tmpY = 0;
			for (std::size_t k = 0; k < tmpInd.size(); k++){
				tmpX += cloakUnitInfo[k].first.x;
				tmpY += cloakUnitInfo[k].first.y;
			}
			maxAreaPosition.x = tmpX / tmpInd.size();
			maxAreaPosition.y = tmpY / tmpInd.size();
		}
	}

	//4. ���� ���� ������ �����ǥ�� Ÿ������ ����
	_scan_position.x = maxAreaPosition.x;
	_scan_position.y = maxAreaPosition.y;

}

void ComsatManager::setCommand(){
	if (_scan_position.x < 0 || _scan_position.y < 0) return;

	BWAPI::Unitset us;
	for (auto &u : BWAPI::Broodwar->self()->getUnits()){
		if (u->getType() == BWAPI::UnitTypes::Terran_Comsat_Station){
			us.insert(u);
		}
	}

	int maxEnergy = 49;
	int maxId = -1;
	for (auto &u : us){
		if (maxEnergy < u->getEnergy()){
			maxEnergy = u->getEnergy();
			maxId = u->getID();
		}
	}

	if (maxId > 0){
		BWAPI::Broodwar->getUnit(maxId)->useTech(BWAPI::TechTypes::Scanner_Sweep, _scan_position);
		setNextEnableFrame(); //1�ʰ� ��ĵ �Ⱦ�
	}
}

void ComsatManager::update(){
	// 1��(24������)�� 4�� ������ �����ص� ����ϴ�
	if (BWAPI::Broodwar->getFrameCount() % 6 != 0) return;

	//��ĵ �ѹ� ���� 1�� ������
	if (BWAPI::Broodwar->getFrameCount() > _next_enable_frame){
		clearScanPosition();
		setScanPosition();
		setCommand();
	}

	//meleeUnit->useTech(BWAPI::TechTypes::Stim_Packs);
}