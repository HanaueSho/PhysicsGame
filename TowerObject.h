/*
	TowerObject.h
	20250914 hanaue sho
	�^���[�̃I�u�W�F�N�g
*/
#ifndef TOWEROBJECT_H_
#define TOWEROBJECT_H_
#include "GameObject.h"

class TowerObject : public GameObject
{
private:
	GameObject* m_pTable;
	GameObject* m_pBlocks[9];

public:
	void Init() override;
	void Update(float dt) override;
	
	void CreateTable(const Vector3& worldPosition);
	void CreateTower(int index, const Vector3& offset);
	void CreateBridge();

	// �N���A����`�F�b�N
	bool CheckBlocks();

};

#endif