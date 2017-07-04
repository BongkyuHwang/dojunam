#include "ComsatManager.h"

using namespace MyBot;

ComsatManager::ComsatManager()
{

}

void ComsatManager::setScanPosition(){
	//1. ��ĵ ����� ����
	BWAPI::Unitset cloakUnits;
	UnitUtil::getAllCloakUnits(cloakUnits);

	//2. ��� ������ �츮 ������ �󸶳� �ִ��� üũ
	//�����ؾߵǴ� �� ������ ������ �ʹ� ������(���� 3�� �̸�) ��ĵ����
	std::vector<std::pair<BWAPI::Position, double>> cloakUnitInfo;
	for (auto &cu : cloakUnits){
		double tmpDps = 0.0;
		for (auto &u : cu->getUnitsInRadius(_scan_radius_offset)){
			BWAPI::WeaponType tmpWeapon = UnitUtil::GetWeapon(u, cu);
			if (tmpWeapon != BWAPI::WeaponTypes::None || tmpWeapon != BWAPI::WeaponTypes::Unknown){
				tmpDps += (double)(tmpWeapon.damageAmount() / tmpWeapon.damageCooldown());
			}
		}

		if (tmpDps > _scan_dps_offset){
			cloakUnitInfo.push_back(std::make_pair(cu->getPosition(), tmpDps));
		}
	}

	//3. �ߺ� ��ĵ���� ����
	for (int i = 0; i < cloakUnitInfo.size(); i++){
		for (int j = 0; j < cloakUnitInfo.size(); j++){
			if (j == i){
				continue;
			}

		}
	}
}