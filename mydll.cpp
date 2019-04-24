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

double GetDistance(TPoint& p1, TPoint& p2) {
	return sqrt((p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y));
}

//最简单的移动方式：横着走再竖着走，从soldier.position到tar
void MoveToTarget(Info& info, TSoldier& soldier, TPoint& tar) {
	while (soldier.move_left > 0) {
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
	
	if (info.round == 6) {
		TPoint LKPoint;	//选中轻骑兵
		for (auto soldier : info.soldierInfo) {
			if (soldier.owner == info.myID) {
				LKPoint = soldier.position;
				break;
			}
		}

		//找到最近的野塔
		double smallestDis = DBL_MAX;
		int closestNum = -1;
		for (auto iter : info.towerInfo) {
			double temp = GetDistance(LKPoint, iter.position);
			if (temp < smallestDis && iter.owner == -1) {
				smallestDis = temp;
				closestNum = iter.id;
			}
		}



	}
}

void player_ai(Info& info)
{
	/*
	for(int i = 0;i<info.towerNum;i++){
		if(info.towerInfo[i].owner == info.myID){
			if(!info.towerInfo[i].recruiting)
				info.myCommandList.addCommand(Produce,i,HeavyArcher);
		}
	}
	 */
	if (info.round <= 6) {
		StartStrategy(info);
	}
}

 