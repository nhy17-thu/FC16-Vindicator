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

double GetDistance(TPoint& point1, TPoint& point2) {
	return sqrt((point1.x - point2.x) * (point1.x - point2.x) + (point1.y - point2.y) * (point1. - point2.y))
}

//开局生产轻骑兵并抢占最近的塔
void StartStrategy() {
	if (info.round == 0) {
		for(int i = 0;i<info.towerNum;i++){					//对于每个塔
			if(info.towerInfo[i].owner == info.myID){		//如果是我的塔
				if(!info.towerInfo[i].recruiting){			//如果当前没有在招募
					info.myCommandList.addCommand(Produce,i,LightKnight);	//就生产士兵
					break;
				}
			}
		}
	} else {
		if (info.round == 6) {
			for ()


			for (vector<TSoldier>::iterator iter = info.soldierInfo.begin(); iter != info.soldierInfo.end(); ++iter) {
				if (iter->owner == info.myID) {

				}
			}
		}
	}
}

void player_ai(Info& info)
{
	/*
	for(int i = 0;i<info.towerNum;i++){					//对于每个塔
		if(info.towerInfo[i].owner == info.myID){		//如果是我的塔
			if(!info.towerInfo[i].recruiting)			//如果当前没有在招募
				info.myCommandList.addCommand(Produce,i,HeavyArcher);	//就生产士兵
		}
	}
	 */
	if (info.round <= 6) {		//开局生产轻骑兵
		StartStrategy();
	}


}
