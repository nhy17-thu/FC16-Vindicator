#include<stdio.h>
#include"mydll.h"
#include"definition.h"
/*
�������ĸ����ָ�������
info.myCommandList.addCommand(Produce,aim_tower_id,HeavyArcher);//�������ڶ����������������id������������ı���
info.myCommandList.addCommand(Attack,aim_soldier_id,x_position,y_position);//��������ڶ��������Ƿ��𹥻���ʿ��id�������͵��ĸ�������Ŀ���x��yλ��
info.myCommandList.addCommand(Upgrade,aim_tower_id);//��������ڶ�������������������id
info.myCommandList.addCommand(Move,aim_soldier_id,UP,distance);//�ƶ�����ڶ������������ƶ���ʿ��id���������������ƶ����򣬵��ĸ��������ƶ�����
*/
void player_ai(Info& info)
{

	for(int i = 0;i<info.towerNum;i++){
		if(info.towerInfo[i].owner == info.myID){
			if(!info.towerInfo[i].recruiting)
				info.myCommandList.addCommand(Produce,i,HeavyArcher);
		}
	}
	int useless = -1;
}

 