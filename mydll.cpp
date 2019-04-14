#include<stdio.h>
#include"mydll.h"
#include"definition.h"
/*
基本的四个添加指令的命令
info.myCommandList.addCommand(Produce,aim_tower_id,HeavyArcher);//造兵命令，第二个参数是造兵的塔id，第三个是造的兵种
info.myCommandList.addCommand(Attack,aim_soldier_id,x_position,y_position);//攻击命令，第二个参数是发起攻击的士兵id，第三和第四个参数是目标的x，y位置
info.myCommandList.addCommand(Upgrade,aim_tower_id);//升级命令，第二个参数是欲升级的塔id
info.myCommandList.addCommand(Move,aim_soldier_id,UP,distance);//移动命令，第二个参数是欲移动的士兵id，第三个参数是移动方向，第四个参数是移动距离
*/
void player_ai(Info& info)
{

	for(int i = 0;i<info.towerNum;i++){
		if(info.towerInfo[i].owner == info.myID){
			if(!info.towerInfo[i].recruiting)
				info.myCommandList.addCommand(Produce,i,HeavyArcher);
		}
	}
}

 