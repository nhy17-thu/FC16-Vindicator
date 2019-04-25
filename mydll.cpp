#include<stdio.h>
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

double GetDistance(TPoint& p1, TPoint& p2);
int GetDistance1(TPoint& p1, TPoint& p2);
void MoveToTarget(Info& info, TSoldier& soldier, TPoint& tar);	//最简单的移动方式：横着走再竖着走，从soldier.position到tar
void StartStrategy(Info& info);	//开局策略：生产轻骑兵并抢占最近的塔

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
struct enemyTarget
{
	bool type;//0 tower ;1 sodier
	int id;
	int danger;
	int attackpriority;
	TPoint pos;
};
vector<myTower> mytow;
vector<myTower> enemytow;
vector<mySoldier> mysod;
vector<mySoldier> enemysod;
vector<enemyTarget> target;

int findtarget(Info& info)
{

	return 0;
}

int attack(Info& info)
{

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
	for (int i = 0; i < info.towerInfo.size(); i++) {
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
	for (int i = 0; i < info.soldierInfo.size(); i++) {
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
}
int unitattack(Info& info,mySoldier attacker,enemyTarget target)
{
	bool inplace = 0;
	int x_pos = 0;
	int y_pos = 0;
	if (target.type == 1)
	{
		double dis = GetDistance(attacker.lastpos, target.pos);
		if (dis < attacker.range) 
		{ 
			inplace = 1; 
			x_pos = target.pos.x;
			y_pos = target.pos.y;
		}
	}
	else
	{
		for (int i = 0; i < 1; i++)
		{
			double dis = GetDistance(attacker.lastpos, target.pos);
			if (dis < attacker.range)
			{
				inplace = 1;
				x_pos = target.pos.x;
				y_pos = target.pos.y;
				break;
			}
			TPoint target1 = target.pos;
			target1.x -= 1;
			dis = GetDistance(attacker.lastpos, target1);
			if (dis < attacker.range)
			{
				inplace = 1;
				x_pos = target1.x;
				y_pos = target1.y;
				break;
			}
			TPoint target2 = target.pos;
			target2.x -= 1;
			target2.y += 1;
			dis = GetDistance(attacker.lastpos, target2);
			if (dis < attacker.range)
			{
				inplace = 1;
				x_pos = target2.x;
				y_pos = target2.y;
				break;
			}
			TPoint target3 = target.pos;
			target3.y += 1;
			dis = GetDistance(attacker.lastpos, target3);
			if (dis < attacker.range)
			{
				inplace = 1;
				x_pos = target3.x;
				y_pos = target3.y;
				break;
			}
			TPoint target4 = target.pos;
			target4.x += 1;
			target4.y += 1;
			dis = GetDistance(attacker.lastpos, target4);
			if (dis < attacker.range)
			{
				inplace = 1;
				x_pos = target4.x;
				y_pos = target4.y;
				break;
			}
			TPoint target5 = target.pos;
			target5.x += 1;
			dis = GetDistance(attacker.lastpos, target5);
			if (dis < attacker.range)
			{
				inplace = 1;
				x_pos = target5.x;
				y_pos = target5.y;
				break;
			}
			TPoint target6 = target.pos;
			target6.x += 1;
			target6.y -= 1;
			dis = GetDistance(attacker.lastpos, target6);
			if (dis < attacker.range)
			{
				inplace = 1;
				x_pos = target6.x;
				y_pos = target6.y;
				break;
			}
			TPoint target7 = target.pos;
			target7.y -= 1;
			dis = GetDistance(attacker.lastpos, target7);
			if (dis < attacker.range)
			{
				inplace = 1;
				x_pos = target7.x;
				y_pos = target7.y;
				break;
			}
			TPoint target8 = target.pos;
			target8.x -= 1;
			target8.y -= 1;
			dis = GetDistance(attacker.lastpos, target8);
			if (dis < attacker.range)
			{
				inplace = 1;
				x_pos = target8.x;
				y_pos = target8.y;
				break;
			}
		}
	}
	if (inplace == 1)
	{
		info.myCommandList.addCommand(Attack,attacker.id, x_pos, y_pos);
	}
	else
	{
		TPoint movetarget;
		if (target.type = 1)
		{
			////////////////////////////////////
		}
	}
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
	while (soldier.move_left > 0) {		//todo：下达move命令以后会立刻扣除移动力吗？
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
		startStageFinished = 1;
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
