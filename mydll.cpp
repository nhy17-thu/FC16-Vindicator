#include<stdio.h>
#include<algorithm>
#include<cmath>
#include"mydll.h"
#include"definition.h"
/*
基本的四个添加指令的命令
info.myCommandList.addCommand(Produce,aim_tower_id,HeavyArcher);//造兵命令，第二个参数是造兵的塔id，第三个是造的兵种
info.myCommandList.addCommand(Attack,aim_soldier_id,x_position,y_position);//攻击命令，第二个参数是发起攻击的士兵id，第三和第四个参数是目标的x，y位置
info.myCommandList.addCommand(Upgrade,aim_tower_id);//升级命令，第二个参数是欲升级的塔id
info.myCommandList.addCommand(Move,aim_soldier_id,UP,distance);//移动命令，第二个参数是欲移动的士兵id，第三个参数是移动方向，第四个参数是移动距离
*/

//在开始策略中使用的全局变量，后续可优化
bool startStageFinished = false;
bool towerFound = false;
TowerInfo targetTower;
struct mySoldier;
struct enemyTarget;
double GetDistance(TPoint& p1, TPoint& p2);
int GetDistance1(TPoint& p1, TPoint& p2);
void MoveToTarget(Info& info, TSoldier& soldier, TPoint& tar);	//最简单的移动方式：横着走再竖着走，从soldier.position到tar
void MoveToTarget(Info& info, mySoldier& soldier, TPoint& tar);
void StartStrategy(Info& info);	//开局策略：生产轻骑兵并抢占最近的塔
void updateTargetsPriority(Info& info);
int unitattack(Info& info, mySoldier attacker, enemyTarget target);

//以下是存储信息的全局变量
int myid = 0;
int mytownum = 1;
int mysodnum = 0;
int mode = 0;
struct mySoldier
{
	int player = 0;
	int id = 0;
	int type = 0;
	int blood = 100;
	bool defense = 0;//是否是用来防守的重弓兵
	TPoint lastpos;
	//int outdated;//信息过时几回合
	double range;

};
struct myTower
{
	int player = 0;
	int id = 0;
	int level = 0;
	int blood = 100;
	TPoint pos;
};
struct enemyTarget {
	bool type;//0 tower ;1 sodier
	int index;//在enemytow和enemysod中的下标
	int id;
	int danger;
	int attackpriority;
	TPoint pos;
	bool disappeared;//在最新的info里没找到
};
vector<myTower> mytow;
vector<myTower> enemytow;
vector<mySoldier> mysod;
vector<mySoldier> enemysod;
vector<enemyTarget> target;

int findtarget(Info &info) {
	//update target
	for (unsigned int i = 0; i < target.size(); i++) target[i].disappeared = true;
	for (unsigned int i = 0; i < enemysod.size(); i++) {//soldier
		bool found = false;
		for (unsigned int j = 0; j < target.size(); j++) {
			if (!target[j].type) continue;
			if (target[j].id == enemysod[i].id) {//update old enemy soldier
				found = true;
				target[j].disappeared = false;
				target[j].index = i;
				target[j].pos = enemysod[i].lastpos;//此处也假定了lastpos = pos
			}
		}
		//add new enemy soldier
		if (found) continue;
		enemyTarget t;
		t.disappeared = false;
		t.id = enemysod[i].id;
		t.index = i;
		t.pos = enemysod[i].lastpos;//此处也假定了lastpos = pos
		t.type = 1;
		target.push_back(t);
	}
	for (unsigned int i = 0; i < enemytow.size(); i++) {
		bool found = false;
		for (unsigned int j = 0; j < target.size(); j++) {
			if (target[j].type) continue;
			if (target[j].id == enemytow[i].id) {
				found = true;
				target[j].disappeared = false;
				target[j].index = i;
				target[j].pos = enemytow[i].pos;
			}
		}
		//add new enemy tower
		if (found) continue;
		enemyTarget t;
		t.disappeared = false;
		t.id = enemytow[i].id;
		t.index = i;
		t.pos = enemytow[i].pos;//此处也假定了lastpos = pos
		t.type = 0;
		target.push_back(t);
	}
	//calculate the priority of each target
	for (int i = target.size() - 1; i >= 0; i--) {
		if (target[i].disappeared) target.erase(target.begin() + i);
	}
	updateTargetsPriority(info);
	return 0;
}


int attack(Info& info)
{
	int size = target.size();
	for (int i = 0; i < size; i++)
	{
		for (int j = size - 2; j >= i; j--)
		{
			if (target[j].attackpriority < target[j + 1].attackpriority) {
				swap(target[j], target[j + 1]);
			}
		}
	}
	for (int i = 0; i < mysodnum; i++)
	{
		unitattack(info, mysod[i], target[0]);
	}
	for (int i = 0; i < mytownum; i++)
	{
		info.myCommandList.addCommand(Produce, mytow[i].id, HeavyKnight);
	}
	return 0;
}
int judgemode(Info& info)
{
	//not ready;建造重弓兵，并移动至合适的位置
	//safe;没有战事，但自己也没有多余兵力进攻
	//attack;准备进攻敌方塔
	//underattack;自己的塔被攻击/正在和敌人交火
	return 0;
}
int updateinfo(Info& info) {
	mytow.clear();
	enemytow.clear();
	mysod.clear();
	enemysod.clear();
	//target.clear();
	myid = info.myID;
	mytownum = 0;
	mysodnum = 0;
	//mode = 0?
	//towers
	for (unsigned int i = 0; i < info.towerInfo.size(); i++) {
		myTower temp;
		if (info.towerInfo[i].owner == info.myID) {
			mytownum++;
			temp.player = info.myID;
			temp.id = info.towerInfo[i].id;
			temp.level = info.towerInfo[i].level;
			temp.blood = info.towerInfo[i].blood;
			temp.pos = info.towerInfo[i].position;
			mytow.push_back(temp);
		}
		else {
			temp.player = info.towerInfo[i].owner;
			temp.id = info.towerInfo[i].id;
			temp.level = info.towerInfo[i].level;
			temp.blood = info.towerInfo[i].blood;
			temp.pos = info.towerInfo[i].position;
			enemytow.push_back(temp);
		}
	}
	//soldiers
	for (unsigned int i = 0; i < info.soldierInfo.size(); i++) {
		mySoldier temp;
		if (info.soldierInfo[i].owner == info.myID) {
			mysodnum++;
			temp.player = info.myID;
			temp.id = info.soldierInfo[i].id;
			temp.type = info.soldierInfo[i].type;
			temp.blood = info.soldierInfo[i].blood;
			//假设重工兵都是用来防守的
			temp.defense = (info.soldierInfo[i].type == HeavyArcher ? 1 : 0);
			temp.lastpos = info.soldierInfo[i].position;
			temp.range = info.soldierInfo[i].range;
			mysod.push_back(temp);
		}
		else {
			temp.player = info.soldierInfo[i].owner;
			temp.id = info.soldierInfo[i].id;
			temp.type = info.soldierInfo[i].type;
			temp.blood = info.soldierInfo[i].blood;
			temp.defense = 0;
			temp.lastpos = info.soldierInfo[i].position;
			temp.range = info.soldierInfo[i].range;
			enemysod.push_back(temp);
		}
	}
	return 0;
}
int unitattack(Info& info, mySoldier attacker, enemyTarget target)
{
	bool inplace = false;
	int x_pos = 0;
	int y_pos = 0;
	if (target.type)
	{
		double dis = GetDistance(attacker.lastpos, target.pos);
		if (dis <= attacker.range + 0.1)
		{
			inplace = true;
			x_pos = target.pos.x;
			y_pos = target.pos.y;
		}
	}
	else
	{
		for (int i = 0; i < 1; i++)
		{
			double dis = GetDistance(attacker.lastpos, target.pos);
			if (dis <= attacker.range + 0.1)
			{
				inplace = true;
				x_pos = target.pos.x;
				y_pos = target.pos.y;
				break;
			}
			TPoint target1 = target.pos;
			target1.x -= 1;
			dis = GetDistance(attacker.lastpos, target1);
			if (dis <= attacker.range + 0.1)
			{
				inplace = true;
				x_pos = target1.x;
				y_pos = target1.y;
				break;
			}
			TPoint target2 = target.pos;
			target2.x -= 1;
			target2.y += 1;
			dis = GetDistance(attacker.lastpos, target2);
			if (dis <= attacker.range + 0.1)
			{
				inplace = true;
				x_pos = target2.x;
				y_pos = target2.y;
				break;
			}
			TPoint target3 = target.pos;
			target3.y += 1;
			dis = GetDistance(attacker.lastpos, target3);
			if (dis <= attacker.range + 0.1)
			{
				inplace = true;
				x_pos = target3.x;
				y_pos = target3.y;
				break;
			}
			TPoint target4 = target.pos;
			target4.x += 1;
			target4.y += 1;
			dis = GetDistance(attacker.lastpos, target4);
			if (dis <= attacker.range + 0.1)
			{
				inplace = true;
				x_pos = target4.x;
				y_pos = target4.y;
				break;
			}
			TPoint target5 = target.pos;
			target5.x += 1;
			dis = GetDistance(attacker.lastpos, target5);
			if (dis <= attacker.range + 0.1)
			{
				inplace = true;
				x_pos = target5.x;
				y_pos = target5.y;
				break;
			}
			TPoint target6 = target.pos;
			target6.x += 1;
			target6.y -= 1;
			dis = GetDistance(attacker.lastpos, target6);
			if (dis <= attacker.range + 0.1)
			{
				inplace = true;
				x_pos = target6.x;
				y_pos = target6.y;
				break;
			}
			TPoint target7 = target.pos;
			target7.y -= 1;
			dis = GetDistance(attacker.lastpos, target7);
			if (dis <= attacker.range + 0.1)
			{
				inplace = true;
				x_pos = target7.x;
				y_pos = target7.y;
				break;
			}
			TPoint target8 = target.pos;
			target8.x -= 1;
			target8.y -= 1;
			dis = GetDistance(attacker.lastpos, target8);
			if (dis <= attacker.range + 0.1)
			{
				inplace = true;
				x_pos = target8.x;
				y_pos = target8.y;
				break;
			}
		}
	}
	if (inplace)
	{
		info.myCommandList.addCommand(Attack, attacker.id, x_pos, y_pos);
	}
	else
	{
		TPoint movetarget;
		int distance1 = INT_MAX;
		if (target.type)
		{
			TPoint movetargetlist[4];
			movetargetlist[0].x = target.pos.x;
			movetargetlist[0].y = target.pos.y + 1;
			movetargetlist[1].x = target.pos.x + 1;
			movetargetlist[1].y = target.pos.y;
			movetargetlist[2].x = target.pos.x;
			movetargetlist[2].y = target.pos.y - 1;
			movetargetlist[3].x = target.pos.x - 1;
			movetargetlist[3].y = target.pos.y;
			for (int i = 0; i < 4; i++)
			{
				int temp_dis = GetDistance1(attacker.lastpos, movetargetlist[i]);
				if (!(info.pointInfo[movetargetlist[i].x][movetargetlist[i].y].occupied) && temp_dis < distance1)
				{
					movetarget = movetargetlist[i];
					distance1 = temp_dis;
				}
			}
		}
		else
		{
			TPoint movetargetlist[12];
			movetargetlist[0].x = target.pos.x - 1;
			movetargetlist[0].y = target.pos.y + 2;
			movetargetlist[1].x = target.pos.x;
			movetargetlist[1].y = target.pos.y + 2;
			movetargetlist[2].x = target.pos.x + 1;
			movetargetlist[2].y = target.pos.y + 2;
			movetargetlist[3].x = target.pos.x + 2;
			movetargetlist[3].y = target.pos.y + 1;
			movetargetlist[4].x = target.pos.x + 2;
			movetargetlist[4].y = target.pos.y;
			movetargetlist[5].x = target.pos.x + 2;
			movetargetlist[5].y = target.pos.y - 1;
			movetargetlist[6].x = target.pos.x + 1;
			movetargetlist[6].y = target.pos.y - 2;
			movetargetlist[7].x = target.pos.x;
			movetargetlist[7].y = target.pos.y - 2;
			movetargetlist[8].x = target.pos.x - 1;
			movetargetlist[8].y = target.pos.y - 2;
			movetargetlist[9].x = target.pos.x - 2;
			movetargetlist[9].y = target.pos.y - 1;
			movetargetlist[10].x = target.pos.x - 2;
			movetargetlist[10].y = target.pos.y;
			movetargetlist[11].x = target.pos.x - 2;
			movetargetlist[11].y = target.pos.y + 1;
			for (int i = 0; i < 12; i++)
			{
				int temp_dis = GetDistance1(attacker.lastpos, movetargetlist[i]);
				if (!(info.pointInfo[movetargetlist[i].x][movetargetlist[i].y].occupied) && temp_dis < distance1)
				{
					movetarget = movetargetlist[i];
					distance1 = temp_dis;
				}
			}
		}
		if (distance1 < 100)
		{
			MoveToTarget(info, attacker, movetarget);
		}
	}
	//乱打
	TPoint tem;
	for (int i = 0; i < 50; i++)
	{
		for (int j = 0; j < 50; j++)
		{
			tem.x = i; tem.y = j;
			if (GetDistance(attacker.lastpos, tem) <= attacker.range)
			{
				info.myCommandList.addCommand(Attack, attacker.id, tem.x, tem.y);
			}
		}
	}
	return 0;
}

void player_ai(Info& info)
{
	if (!startStageFinished) {
		StartStrategy(info);
		return;
	}
	{
		updateinfo(info);
		mode = judgemode(info);
		findtarget(info);
		attack(info);
	}
	cout << info.round << endl;
}



double GetDistance(TPoint& p1, TPoint& p2) {
	return sqrt((p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y));
}

int GetDistance1(TPoint& p1, TPoint& p2) {
	return (abs(p1.x - p2.x) + abs(p1.y - p2.y));
}


//最简单的移动方式：横着走再竖着走，从soldier.position到tar
void MoveToTarget(Info& info, TSoldier& soldier, TPoint& tar) {
	{
		int deltaX = tar.x - soldier.x_position;
		int deltaY = tar.y - soldier.y_position;
		if (deltaX == 0 && deltaY == 0)	//无需移动
			return;
		if (deltaX > 0) {
			info.myCommandList.addCommand(Move, soldier.id, RIGHT, deltaX);
		}
		else if (deltaX < 0) {
			info.myCommandList.addCommand(Move, soldier.id, LEFT, -deltaX);
		}
		if (deltaY > 0) {
			info.myCommandList.addCommand(Move, soldier.id, UP, deltaY);
		}
		else if (deltaY < 0) {
			info.myCommandList.addCommand(Move, soldier.id, DOWN, -deltaY);
		}
	}
}

void MoveToTarget(Info& info, mySoldier& soldier, TPoint& tar) {
	{
		int deltaX = tar.x - soldier.lastpos.x;
		int deltaY = tar.y - soldier.lastpos.y;
		if (deltaX == 0 && deltaY == 0)	//无需移动
			return;
		if (deltaX > 0) {
			info.myCommandList.addCommand(Move, soldier.id, RIGHT, deltaX);
		}
		else if (deltaX < 0) {
			info.myCommandList.addCommand(Move, soldier.id, LEFT, -deltaX);
		}
		if (deltaY > 0) {
			info.myCommandList.addCommand(Move, soldier.id, UP, deltaY);
		}
		else if (deltaY < 0) {
			info.myCommandList.addCommand(Move, soldier.id, DOWN, -deltaY);
		}
	}
}

//开局策略：生产轻骑兵并抢占最近的塔
void StartStrategy(Info& info) {
	if (info.round == 0) {
		for (auto iter : info.towerInfo) {
			if (iter.owner == info.myID) {
				info.myCommandList.addCommand(Produce, iter.id, LightKnight);
			}
		}
	}

	if (info.round > 5)
	{
		startStageFinished = true;
		return;
		//轻骑兵已生产
		if (towerFound && targetTower.owner == info.myID) {
			startStageFinished = true;
			return;
		}
		TSoldier LK;		//选中轻骑兵
		for (auto soldier : info.soldierInfo) {
			if (soldier.owner == info.myID) {
				LK = soldier;
				break;
			}
		}
		//找到最近的野塔
		if (!towerFound) {
			double smallestDis = DBL_MAX;
			for (auto iter : info.towerInfo) {
				double temp = GetDistance(LK.position, iter.position);
				if (temp < smallestDis && iter.owner == -1) {
					smallestDis = temp;
					targetTower = iter;
					towerFound = true;
				}
			}
		}
		//移动到指定地点并攻击
		/*if (GetDistance(LK.position, targetTower.position) > 2.5) { //sqrt5
			MoveToTarget(info, LK, targetTower.position);
		}
		else {
			for (int dx = -1; dx <= 1; ++dx) {
				for (int dy = -1; dy <= 1; ++dy) {
					info.myCommandList.addCommand(Attack, LK.id, LK.x_position + dx, LK.y_position + dy);
				}
			}
		}*/
	}
}
void updateTargetsPriority(Info& info) {
	for (int i = 0; i < target.size(); ++i) {
		int priority = 0;
		int dis = 0x7fffffff;
		for (int j = 0; j < mytownum; ++j) {
			int temp = GetDistance(mytow[j].pos, target[i].pos);
			if (temp < dis) {
				dis = temp;
			}
		}
		target[i].attackpriority = 10000 / (dis * dis);
	}
}
