#pragma once

#include <vector>
#include "mapObject.h"

#define TASK_NONE 0
#define TASK_DEFEND_LOCATION 1
#define TASK_ATTACK_LOCATION 2
#define TASK_SCOUT 3

#define GROUP_STATE_IDLE 0
#define GROUP_STATE_MOVING 1
#define GROUP_STATE_BATTLE 2

class MASTERAI;

template <class T> int Find(std::vector<T>& arr, T object) {
	for (int i = 0; i < arr.size(); i++) {
		if (arr[i] == object)
			return i;
	}
	return -1;
}

class GROUPAI {
	friend class MASTERAI;
	friend class SMALL_ATTACK;
	MASTERAI* _pMaster;
	std::vector<MAPOBJECT*> _members;
	std::vector<MAPOBJECT*> _visibleEnemies;
	int _task;
	int _state;
	Rect _mapArea;
public:
	GROUPAI(MASTERAI* pmaster);
	~GROUPAI();

	void AddMember(MAPOBJECT* pnewmember);
	void RemoveMember(MAPOBJECT* poldmember);
	void DisbandGroup();

	void EnemiesSpotted(std::vector<MAPOBJECT*>& manyEnemies);
	void SetTask(int newTask, Rect* area);
	GROUPAI* SplitGroup(std::vector<int>& units);

	void GroupAI();
	bool isDead();
	INTPOINT GetCenter();

	//Orders
	void Goto(Rect area);
	void Attack(std::vector<MAPOBJECT*>& enemies);
	void RetreatTo(Rect ma);
	void Shuffle();
};