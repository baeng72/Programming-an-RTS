#ifndef _RTS_GROUPAI_
#define _RTS_GROUPAI_

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

template <class T> int Find(std::vector<T> array, T object)
{
	for (int i = 0; i < array.size(); i++)
		if (array[i] == object)
			return i;

	return -1;
}

class GROUPAI
{
	friend class MASTERAI;
	friend class SMALL_ATTACK;
public:
	GROUPAI(MASTERAI* _master);
	~GROUPAI();

	void AddMember(MAPOBJECT* newMember);
	void RemoveMember(MAPOBJECT* oldMember);
	void DisbandGroup();

	void EnemiesSpotted(std::vector<MAPOBJECT*>& manyEnemies);
	void SetTask(int newTask, RECT* area);
	GROUPAI* SplitGroup(std::vector<int> units);

	void GroupAI();
	bool isDead();
	INTPOINT GetCenter();

	//Orders
	void Goto(RECT mArea);
	void Attack(std::vector<MAPOBJECT*>& enemies);
	void RetreatTo(RECT ma);
	void Shuffle();

private:
	MASTERAI* m_pMaster;
	std::vector<MAPOBJECT*> m_members, m_visibleEnemies;
	int m_task;
	int m_state;
	RECT m_mapArea;
};

#endif