#include<stdio.h>
#include<algorithm>
#include<functional>
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

double GetDistance(TPoint &p1, TPoint &p2);

int GetDistance1(TPoint &p1, TPoint &p2);

int GetDistance3(TPoint &p1, TPoint &p2);

int attack(Info &info);

int findtarget(Info &info);

int judgemode(Info &info);

int updateinfo(Info &info);

void MoveToTarget(Info &info, mySoldier &soldier, TPoint &tar);

void StartStrategy(Info &info);    //开局策略：生产轻骑兵并抢占最近的塔
void updateTargetsPriority(Info &info);

void cal_indiv_priority(Info& info);

int unitattack(Info &info, mySoldier &attacker, enemyTarget &target);

void ProduceAndUpgrade(Info &info);

int ArcherAttack(Info &info, mySoldier &attacker, enemyTarget &attackTarget);

//以下是存储信息的全局变量
int myid = 0;
int mytownum = 1;
int mysodnum = 0;

/*enum mode {
	not_ready,//建造重弓兵，并移动至合适的位置
	safe,//没有战事，但自己也没有多余兵力进攻
	attack_tower,//正在进攻敌方塔
	tower_underattack,//自己的塔被攻击
	fire//正在和敌人交火
};*/

enum TowerMode {
    towerSafe,//没有被打
    attack_safe,//正在被打但安全
    attack_danger,//正在被打且危险
    enemyTower
};

enum SoldierMode {
    soldierSafe,//没有战事，但自己也没有多余兵力进攻
    not_in_pos,//重弓兵未就位
    fire,//正在和敌人交火
    attack_tower,//正在进攻敌方塔
    enemySoldier
};

struct priority_and_index {//个体优先度和该敌方目标在target中的下标
    double pri;
    int index;
};

struct mySoldier {
    int player = 0;
    int id = 0;
    TSoldierType type;
    int blood = 100;
    bool defense = false;//是否是用来防守的重弓兵
    TPoint lastpos;
    //int outdated;//信息过时几回合
    double range;
    int move_left = 0;
    int move = 0;
    SoldierMode mode;
    int blood_last;
    int armor;
    int attack;
    bool disappeared;
    vector<priority_and_index> individual_priority;//记录每个敌方目标对于我方每个士兵的优先度
    int target_index;//存储选定的target在target序列中的下标
};

struct myTower {
    int player = 0;
    int id = 0;
    int level = 0;
    int blood = 100;
    int blood_last;
    TPoint pos;
    TowerMode mode;
    bool disappeared;
};

struct enemyTarget {
    bool type;//false tower; true soldier
    int index;//在enemytow和enemysod中的下标
    int id;
    int danger;
    int attackpriority;
    TPoint pos;
    bool disappeared;//在最新的info里没找到
    int FieldOfVision;
    int blood;
    int blood_last;
    int armor = 0;
    int attack = 0;
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
                target[j].blood = enemysod[i].blood;
                target[j].blood_last = enemysod[i].blood_last;
                target[j].armor = enemysod[i].armor;
                target[j].attack = enemysod[i].attack;
            }
        }
        //add new enemy soldier
        if (found) continue;
        enemyTarget t;
        t.disappeared = false;
        t.id = enemysod[i].id;
        t.index = i;
        t.pos = enemysod[i].lastpos;//此处也假定了lastpos = pos
        t.type = true;
        t.blood = enemysod[i].blood;
        t.blood_last = enemysod[i].blood_last;
        t.armor = enemysod[i].armor;
        t.attack = enemysod[i].attack;
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
                target[j].blood = enemytow[i].blood;
                target[j].blood_last = enemytow[i].blood_last;
            }
        }
        //add new enemy tower
        if (found) continue;
        enemyTarget t;
        t.disappeared = false;
        t.id = enemytow[i].id;
        t.index = i;
        t.pos = enemytow[i].pos;//此处也假定了lastpos = pos
        t.type = false;
        t.blood = enemytow[i].blood;
        t.blood_last = enemytow[i].blood_last;
        t.armor = 0;
        t.attack = 0;
        target.push_back(t);
    }
    //calculate the priority of each target
    for (int i = target.size() - 1; i >= 0; i--) {
        if (target[i].disappeared) target.erase(target.begin() + i);
    }
    updateTargetsPriority(info);
    cal_indiv_priority(info);
    for (int i = 0; i < target.size(); i++) {
        if (target[i].type) {
            if (enemysod[target[i].index].type == LightArcher)target[i].FieldOfVision = 7;
            if (enemysod[target[i].index].type == LightInfantry)target[i].FieldOfVision = 7;
            if (enemysod[target[i].index].type == LightKnight)target[i].FieldOfVision = 11;
            if (enemysod[target[i].index].type == Mangonel)target[i].FieldOfVision = 5;
            if (enemysod[target[i].index].type == HeavyInfantry)target[i].FieldOfVision = 7;
            if (enemysod[target[i].index].type == HeavyArcher)target[i].FieldOfVision = 7;
            if (enemysod[target[i].index].type == HeavyKnight)target[i].FieldOfVision = 9;
        } else {
            if (enemytow[target[i].index].level == 1)target[i].FieldOfVision = 7;
            if (enemytow[target[i].index].level == 2)target[i].FieldOfVision = 9;
            if (enemytow[target[i].index].level == 3)target[i].FieldOfVision = 11;

        }
    }
    return 0;
}


int attack(Info &info) {
    int size = target.size();
    /*
    for (int i = 0; i < size; i++) {
        for (int j = size - 2; j >= i; j--) {
            if (target[j].attackpriority < target[j + 1].attackpriority) {
                swap(target[j], target[j + 1]);
            }
        }
    }
     */
    for (int i = 0; i < mysodnum; i++) {
        for (int a = 0; a < size; a++) {
            for (int b = size - 2; b >= a; b--) {
                if (mysod[i].individual_priority[b].pri < mysod[i].individual_priority[b + 1].pri) {
                    swap(mysod[i].individual_priority[b], mysod[i].individual_priority[b + 1]);
                }
            }
        }

        enemyTarget *tempTarget = nullptr;
        for (int j = 0; j < target.size(); ++j) {
            if (GetDistance1(target[mysod[i].individual_priority[j].index].pos, mysod[i].lastpos) < 15) {
                tempTarget = &target[j];
                break;
            }
        }
        if (tempTarget == nullptr) {
            tempTarget = &target[mysod[i].individual_priority[0].index];
        }

        if (mysod[i].type == HeavyArcher)
            ArcherAttack(info, mysod[i], *tempTarget);
        else
            unitattack(info, mysod[i], *tempTarget);
    }
    return 0;
}

int judgemode(Info &info) {
    //soldier
    for (int i = 0; i < mysod.size(); i++) {
        if (mysod[i].blood_last == mysod[i].blood) mysod[i].mode = soldierSafe;
        for (int j = 0; j < enemytow.size(); j++) {
            if (GetDistance(mysod[i].lastpos, enemytow[j].pos) < 2.5) mysod[i].mode = attack_tower;
            else if ((mysod[i].type == HeavyArcher) && (GetDistance(mysod[i].lastpos, enemytow[j].pos) < 4.25))
                mysod[i].mode = attack_tower;
            //普通的兵认为只有贴着才算打，重弓兵打塔最长距离为3sqrt(2)=4.243
            //没有考虑轻弓兵
            //会覆盖soldiersafe
        }
        if (mysod[i].blood_last > mysod[i].blood) mysod[i].mode = fire;//这会将前面判断打塔的状态覆盖掉

    }
    //tower
    for (int i = 0; i < mytow.size(); i++) {
        if (mytow[i].blood_last > mytow[i].blood) {//attack_safe和attack_danger的界限划定为6回合
            if (mytow[i].blood / double(mytow[i].blood_last - mytow[i].blood) >= 6) mytow[i].mode = attack_safe;
            else mytow[i].mode = attack_danger;
        } else mytow[i].mode = towerSafe;
    }

    return 0;
}

int updateinfo(Info &info) {
    myid = info.myID;
    //mode = 0?
    //towers
    for (unsigned int i = 0; i < mytow.size(); i++) mytow[i].disappeared = true;
    for (unsigned int i = 0; i < enemytow.size(); i++) enemytow[i].disappeared = true;
    for (unsigned int i = 0; i < info.towerInfo.size(); i++) {
        if (info.round < 50 && info.towerInfo[i].id == 8) {
            continue;//50回合之前不打中塔
        }
        if (info.towerInfo[i].owner == info.myID) {
            bool found = false;
            for (unsigned int j = 0; j < mytow.size(); j++) {//遍历已存着的自己的塔
                if (info.towerInfo[i].id != mytow[j].id) continue;
                mytow[j].disappeared = false;
                found = true;
                mytow[j].level = info.towerInfo[i].level;
                mytow[j].blood_last = mytow[j].blood;
                mytow[j].blood = info.towerInfo[j].blood;
                break;
            }
            if (!found) {
                myTower temp;
                temp.player = info.myID;
                temp.id = info.towerInfo[i].id;
                temp.level = info.towerInfo[i].level;
                temp.blood = info.towerInfo[i].blood;
                temp.blood_last = temp.blood;
                temp.pos = info.towerInfo[i].position;
                temp.disappeared = false;
                mytow.push_back(temp);
            }
        } else {
            bool found = false;
            for (int j = 0; j < enemytow.size(); j++) {
                if (enemytow[j].id != info.towerInfo[i].id) continue;
                enemytow[j].disappeared = false;
                found = true;
                enemytow[j].player = info.towerInfo[i].owner;
                enemytow[j].level = info.towerInfo[i].level;
                enemytow[j].blood_last = enemytow[j].blood;
                enemytow[j].blood = info.towerInfo[i].blood;
                break;
            }
            if (!found) {
                myTower temp;
                temp.player = info.towerInfo[i].owner;
                temp.id = info.towerInfo[i].id;
                temp.level = info.towerInfo[i].level;
                temp.blood = info.towerInfo[i].blood;
                temp.blood_last = temp.blood;
                temp.pos = info.towerInfo[i].position;
                temp.mode = enemyTower;
                temp.disappeared = false;
                enemytow.push_back(temp);
            }
        }
    }
    for (int i = 0; i < enemytow.size(); i++)
        if (enemytow[i].disappeared) {
            enemytow.erase(enemytow.begin() + i);
            --i;
        }

    for (int i = 0; i < mytow.size(); i++)
        if (mytow[i].disappeared) {
            mytow.erase(mytow.begin() + i);
            --i;
        }
    //soldiers
    for (unsigned int i = 0; i < mysod.size(); i++) mysod[i].disappeared = true;
    for (unsigned int i = 0; i < enemysod.size(); i++) enemysod[i].disappeared = true;

    for (unsigned int i = 0; i < info.soldierInfo.size(); i++) {
        if (info.soldierInfo[i].owner == info.myID) {
            bool found = false;
            for (unsigned int j = 0; j < mysod.size(); j++) {
                if (mysod[j].id != info.soldierInfo[i].id) continue;
                found = true;
                mysod[j].blood_last = mysod[j].blood;
                mysod[j].blood = info.soldierInfo[i].blood;
                mysod[j].lastpos = info.soldierInfo[i].position;
                mysod[j].move_left = info.soldierInfo[i].move_left;
                mysod[j].move = info.soldierInfo[i].move_ability;
                mysod[j].disappeared = false;
                mysod[j].armor = info.soldierInfo[i].armor;
                mysod[j].attack = info.soldierInfo[i].attack;
            }
            if (!found) {
                mySoldier temp;

                temp.player = info.myID;
                temp.id = info.soldierInfo[i].id;
                temp.type = info.soldierInfo[i].type;
                temp.blood_last = temp.blood;
                temp.blood = info.soldierInfo[i].blood;
                //假设重工兵都是用来防守的
                temp.defense = (info.soldierInfo[i].type == HeavyArcher ? 1 : 0);
                temp.lastpos = info.soldierInfo[i].position;
                temp.range = info.soldierInfo[i].range;
                temp.move_left = info.soldierInfo[i].move_left;
                temp.move = info.soldierInfo[i].move_ability;
                temp.armor = info.soldierInfo[i].armor;
                temp.attack = info.soldierInfo[i].attack;
                temp.disappeared = false;
                mysod.push_back(temp);
            }
        } else {
            bool found = false;
            for (unsigned int j = 0; j < enemysod.size(); j++) {
                if (info.soldierInfo[i].id != enemysod[j].id) continue;
                found = true;
                enemysod[j].player = info.soldierInfo[i].owner;
                enemysod[j].blood_last = enemysod[j].blood;
                enemysod[j].blood = info.soldierInfo[i].blood;
                enemysod[j].lastpos = info.soldierInfo[i].position;
                enemysod[j].move_left = info.soldierInfo[i].move_left;
                enemysod[j].move = info.soldierInfo[i].move_ability;
                enemysod[j].armor = info.soldierInfo[i].armor;
                enemysod[j].attack = info.soldierInfo[i].attack;
                enemysod[j].disappeared = false;
            }
            if (!found) {//所有mode都没法判断
                mySoldier temp;
                temp.player = info.soldierInfo[i].owner;
                temp.id = info.soldierInfo[i].id;
                temp.type = info.soldierInfo[i].type;
                temp.blood_last = info.soldierInfo[i].blood;
                temp.blood = temp.blood_last;
                temp.defense = 0;
                temp.lastpos = info.soldierInfo[i].position;
                temp.range = info.soldierInfo[i].range;
                temp.move_left = info.soldierInfo[i].move_left;
                temp.move = info.soldierInfo[i].move_ability;
                temp.armor = info.soldierInfo[i].armor;
                temp.attack = info.soldierInfo[i].attack;
                temp.disappeared = false;
                enemysod.push_back(temp);
            }

        }
    }
    for (unsigned int i = 0; i < mysod.size(); i++)
        if (mysod[i].disappeared) {
            mysod.erase(mysod.begin() + i);
            --i;
        }
    for (unsigned int i = 0; i < enemysod.size(); i++)
        if (enemysod[i].disappeared) {
            enemysod.erase(enemysod.begin() + i);
            --i;
        }
    mysodnum = mysod.size();
    mytownum = mytow.size();
    return 0;
}

int unitattack(Info &info, mySoldier &attacker, enemyTarget &target) {
    bool inplace = false;
    int x_pos = 0;
    int y_pos = 0;
    if (target.type) {
        double dis = GetDistance(attacker.lastpos, target.pos);
        if (dis <= attacker.range + 0.1) {
            inplace = true;
            x_pos = target.pos.x;
            y_pos = target.pos.y;
        }
    } else {
        for (int i = 0; i < 1; i++) {
            double dis = GetDistance(attacker.lastpos, target.pos);
            if (dis <= attacker.range + 0.1) {
                inplace = true;
                x_pos = target.pos.x;
                y_pos = target.pos.y;
                break;
            }
            TPoint target1 = target.pos;
            target1.x -= 1;
            dis = GetDistance(attacker.lastpos, target1);
            if (dis <= attacker.range + 0.1) {
                inplace = true;
                x_pos = target1.x;
                y_pos = target1.y;
                break;
            }
            TPoint target2 = target.pos;
            target2.x -= 1;
            target2.y += 1;
            dis = GetDistance(attacker.lastpos, target2);
            if (dis <= attacker.range + 0.1) {
                inplace = true;
                x_pos = target2.x;
                y_pos = target2.y;
                break;
            }
            TPoint target3 = target.pos;
            target3.y += 1;
            dis = GetDistance(attacker.lastpos, target3);
            if (dis <= attacker.range + 0.1) {
                inplace = true;
                x_pos = target3.x;
                y_pos = target3.y;
                break;
            }
            TPoint target4 = target.pos;
            target4.x += 1;
            target4.y += 1;
            dis = GetDistance(attacker.lastpos, target4);
            if (dis <= attacker.range + 0.1) {
                inplace = true;
                x_pos = target4.x;
                y_pos = target4.y;
                break;
            }
            TPoint target5 = target.pos;
            target5.x += 1;
            dis = GetDistance(attacker.lastpos, target5);
            if (dis <= attacker.range + 0.1) {
                inplace = true;
                x_pos = target5.x;
                y_pos = target5.y;
                break;
            }
            TPoint target6 = target.pos;
            target6.x += 1;
            target6.y -= 1;
            dis = GetDistance(attacker.lastpos, target6);
            if (dis <= attacker.range + 0.1) {
                inplace = true;
                x_pos = target6.x;
                y_pos = target6.y;
                break;
            }
            TPoint target7 = target.pos;
            target7.y -= 1;
            dis = GetDistance(attacker.lastpos, target7);
            if (dis <= attacker.range + 0.1) {
                inplace = true;
                x_pos = target7.x;
                y_pos = target7.y;
                break;
            }
            TPoint target8 = target.pos;
            target8.x -= 1;
            target8.y -= 1;
            dis = GetDistance(attacker.lastpos, target8);
            if (dis <= attacker.range + 0.1) {
                inplace = true;
                x_pos = target8.x;
                y_pos = target8.y;
                break;
            }
        }
    }
    if (inplace) {
        info.myCommandList.addCommand(Attack, attacker.id, x_pos, y_pos);
    } else {
        TPoint movetarget;
        int distance1 = INT_MAX;
        if (target.type) {
            TPoint movetargetlist[4];
            movetargetlist[0].x = target.pos.x;
            movetargetlist[0].y = target.pos.y + 1;
            movetargetlist[1].x = target.pos.x + 1;
            movetargetlist[1].y = target.pos.y;
            movetargetlist[2].x = target.pos.x;
            movetargetlist[2].y = target.pos.y - 1;
            movetargetlist[3].x = target.pos.x - 1;
            movetargetlist[3].y = target.pos.y;
            for (int i = 0; i < 4; i++) {
                int temp_dis = GetDistance1(attacker.lastpos, movetargetlist[i]);
                if (!(info.pointInfo[movetargetlist[i].x][movetargetlist[i].y].occupied) && temp_dis < distance1) {
                    movetarget = movetargetlist[i];
                    distance1 = temp_dis;
                }
            }
        } else {
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
            for (int i = 0; i < 12; i++) {
                int temp_dis = GetDistance1(attacker.lastpos, movetargetlist[i]);
                if (!(info.pointInfo[movetargetlist[i].x][movetargetlist[i].y].occupied) && temp_dis < distance1) {
                    movetarget = movetargetlist[i];
                    distance1 = temp_dis;
                }
            }
        }
        if (distance1 < 100) {
            MoveToTarget(info, attacker, movetarget);
            info.myCommandList.addCommand(Attack, attacker.id, target.pos.x, target.pos.y);
            info.myCommandList.addCommand(Attack, attacker.id, target.pos.x - 1, target.pos.y);
            info.myCommandList.addCommand(Attack, attacker.id, target.pos.x - 1, target.pos.y + 1);
            info.myCommandList.addCommand(Attack, attacker.id, target.pos.x, target.pos.y + 1);
            info.myCommandList.addCommand(Attack, attacker.id, target.pos.x + 1, target.pos.y + 1);
            info.myCommandList.addCommand(Attack, attacker.id, target.pos.x + 1, target.pos.y);
            info.myCommandList.addCommand(Attack, attacker.id, target.pos.x + 1, target.pos.y - 1);
            info.myCommandList.addCommand(Attack, attacker.id, target.pos.x, target.pos.y - 1);
            info.myCommandList.addCommand(Attack, attacker.id, target.pos.x - 1, target.pos.y - 1);
        }
    }
    //乱打
    TPoint tem;
    for (int i = 0; i < 50; i++) {
        for (int j = 0; j < 50; j++) {
            tem.x = i;
            tem.y = j;
            if (GetDistance(attacker.lastpos, tem) <= attacker.range + 0.1) {
                info.myCommandList.addCommand(Attack, attacker.id, tem.x, tem.y);
            }
        }
    }
    return 0;
}

void player_ai(Info &info) {
    if (!startStageFinished) {
        StartStrategy(info);
        return;
    }
    updateinfo(info);
    judgemode(info);
    findtarget(info);
    attack(info);
    ProduceAndUpgrade(info);
}


double GetDistance(TPoint &p1, TPoint &p2) {
    return sqrt((p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y));
}

int GetDistance1(TPoint &p1, TPoint &p2) {
    return (abs(p1.x - p2.x) + abs(p1.y - p2.y));
}

int GetDistance3(TPoint &p1, TPoint &p2) {
    int a = abs(p1.x - p2.x);
    int b = abs(p2.y - p2.y);
    if (a > b)return a; else return b;
}


//最简单的移动方式：横着走再竖着走，从soldier.position到tar
/*void MoveToTarget(Info& info, TSoldier& soldier, TPoint& tar) {
	int deltaX = tar.x - soldier.x_position;
		int deltaY = tar.y - soldier.y_position;
		if (deltaX == 0 && deltaY == 0)    //无需移动
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
*/
void MoveToTarget(Info &info, mySoldier &soldier, TPoint &tar) {
    int deltaX = 0;
    int deltaY = 0;
    if (GetDistance1(soldier.lastpos, tar) < soldier.move_left) {
        deltaX = tar.x - soldier.lastpos.x;
        deltaY = tar.y - soldier.lastpos.y;
        info.pointInfo[tar.x][tar.y].occupied = true;
        info.pointInfo[soldier.lastpos.x][soldier.lastpos.y].occupied = false;
        soldier.lastpos.x = tar.x;
        soldier.lastpos.y = tar.y;
    } else {
        vector<TPoint> tar1;
        for (int x = soldier.lastpos.x - soldier.move_left; x < soldier.lastpos.x + soldier.move_left + 1; x++) {
            for (int y = soldier.lastpos.y - soldier.move_left; y < soldier.lastpos.y + soldier.move_left + 1; y++) {
                if (x >= 0 && x < 50 && y >= 0 && y < 50) {
                    TPoint temp = info.pointInfo[x][y];
                    if (GetDistance1(soldier.lastpos, temp) > soldier.move_left) continue;
                    if (temp.occupied) continue;
                    tar1.push_back(temp);
                }
            }
        }
        int *score = new int[tar1.size()];
        for (int i = 0; i < tar1.size(); i++) {
            score[i] = -GetDistance1(tar1[i], tar) * 2;
            if (tar1[i].land == River)score[i] -= (soldier.move - 1) * 2;
            if (tar1[i].land == Forest) {
                if (soldier.type == LightArcher) score[i]++;
                if (soldier.type == HeavyArcher) score[i]++;
                if (soldier.type == LightKnight) score[i]++;
                if (soldier.type == HeavyKnight) score[i]++;
            }
            if (tar1[i].land == Mountain) {
                if (soldier.type == LightKnight) score[i] -= 4;
                if (soldier.type == HeavyKnight) score[i] -= 4;
                if (soldier.type == Mangonel) score[i] -= 2;
            }
            if (tar1[i].land == Dorm) {
                if (soldier.type == HeavyArcher) score[i]++;
                if (soldier.type == HeavyInfantry) score[i]++;
                if (soldier.type == HeavyKnight) score[i]++;
            }
        }
        TPoint tarr;
        int scorr = -10000;
        for (int i = 0; i < tar1.size(); i++) {
            if (score[i] > scorr) {
                scorr = score[i];
                tarr = tar1[i];
            }
        }
        deltaX = tarr.x - soldier.lastpos.x;
        deltaY = tarr.y - soldier.lastpos.y;
        info.pointInfo[tarr.x][tarr.y].occupied = true;
        info.pointInfo[soldier.lastpos.x][soldier.lastpos.y].occupied = false;
        soldier.lastpos.x = tarr.x;
        soldier.lastpos.y = tarr.y;
        delete[] score;
    }
    if (deltaX == 0 && deltaY == 0)    //无需移动
        return;
    if (deltaX > 0) {
        info.myCommandList.addCommand(Move, soldier.id, RIGHT, deltaX);
        soldier.move_left -= deltaX;
    } else if (deltaX < 0) {
        info.myCommandList.addCommand(Move, soldier.id, LEFT, -deltaX);
        soldier.move_left += deltaX;
    }
    if (deltaY > 0) {
        info.myCommandList.addCommand(Move, soldier.id, UP, deltaY);
        soldier.move_left -= deltaY;
    } else if (deltaY < 0) {
        info.myCommandList.addCommand(Move, soldier.id, DOWN, -deltaY);
        soldier.move_left += deltaY;
    }
}

//开局策略：生产轻骑兵并抢占最近的塔
void StartStrategy(Info &info) {
    if (info.round == 0) {
        for (auto iter : info.towerInfo) {
            if (iter.owner == info.myID) {
                info.myCommandList.addCommand(Produce, iter.id, HeavyArcher);
            }
        }
    }

    if (info.round > 5) {
        startStageFinished = true;
        return;
        //轻骑兵已生产
        if (towerFound && targetTower.owner == info.myID) {
            startStageFinished = true;
            return;
        }
        TSoldier LK;        //选中轻骑兵
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

void updateTargetsPriority(Info &info) {
    if (mytownum > 0) {
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
    } else {
        for (int i = 0; i < target.size(); ++i) {
            int priority = 0;
            int dis = 0x7fffffff;
            for (int j = 0; j < mysodnum; ++j) {
                int temp = GetDistance(mysod[j].lastpos, target[i].pos);
                if (temp < dis) {
                    dis = temp;
                }
            }
            target[i].attackpriority = 10000 / (dis * dis);
        }
    }
}

/* Thoughts in Produce&Upgrade
 * 选择最安全的，能在死前把兵造出来的塔造兵
 * 只要有资源就造兵
 * 如果人口不够（且资源充足）就升级塔
 */
void ProduceAndUpgrade(Info &info) {
    PlayerInfo me;
    for (auto iter : info.playerInfo) {
        if (iter.id == info.myID) {
            me = iter;
            break;
        }
    }
    if (mytownum == 0)
        return;
    // 找到相对最安全的塔升级/造兵
    // 最安全：没有被打，血量大于阈值，靠边
    myTower *targetTow = nullptr;
    TPoint central;
    central.x = central.y = 25;
    TowerMode status = towerSafe;
    while (!targetTow) {
        if (status == attack_danger) {
            // 挑一个还能活得最久的
            int expectLife = -1;
            for (auto iter : mytow) {
                int tempLife = (iter.blood_last - iter.blood) / iter.blood;
                if (tempLife > expectLife) {
                    expectLife = tempLife;
                    targetTow = &iter;
                }
            }
            break;
        } else {
            double furthermostDis = -1;
            for (auto iter : mytow) {
                if (iter.mode != status)
                    continue;
                double tempDis = GetDistance(iter.pos, central);
                if (tempDis > furthermostDis) {
                    furthermostDis = tempDis;
                    targetTow = &iter;
                }
            }
        }
        if (status == towerSafe) status = attack_safe;
        else if (status == attack_safe) status = attack_danger;
    }

    if (me.population >= me.max_population) {    // 当现有人口数恰等于最大人口数时，是否可以生产？
        info.myCommandList.addCommand(Upgrade, targetTow->id);
    } else {
        // 重骑兵重弓兵1:3
        // todo: 根据敌人部队组成针对性造兵
        int hKnightNum = 0, hArcherNum = 0;
        for (auto iter : mysod) {
            if (iter.type == HeavyKnight)
                ++hKnightNum;
            else if (iter.type == HeavyArcher)
                ++hArcherNum;
        }
        if (hArcherNum <= 2 * hKnightNum) {
            info.myCommandList.addCommand(Produce, targetTow->id, HeavyArcher);
        } else {
            info.myCommandList.addCommand(Produce, targetTow->id, HeavyKnight);
        }
    }
}

int ArcherAttack(Info &info, mySoldier &attacker, enemyTarget &attackTarget) {
    bool inplace = false;
    int x_pos = 0;
    int y_pos = 0;
    if (attackTarget.type) {
        double dis = GetDistance(attacker.lastpos, attackTarget.pos);
        if (dis <= attacker.range + 0.1) {
            inplace = true;
            x_pos = attackTarget.pos.x;
            y_pos = attackTarget.pos.y;
        }
    } else {
        for (int i = 0; i < 1; i++) {
            double dis = GetDistance(attacker.lastpos, attackTarget.pos);
            if (dis <= attacker.range + 0.1) {
                inplace = true;
                x_pos = attackTarget.pos.x;
                y_pos = attackTarget.pos.y;
                break;
            }
            TPoint target1 = attackTarget.pos;
            target1.x -= 1;
            dis = GetDistance(attacker.lastpos, target1);
            if (dis <= attacker.range + 0.1) {
                inplace = true;
                x_pos = target1.x;
                y_pos = target1.y;
                break;
            }
            TPoint target2 = attackTarget.pos;
            target2.x -= 1;
            target2.y += 1;
            dis = GetDistance(attacker.lastpos, target2);
            if (dis <= attacker.range + 0.1) {
                inplace = true;
                x_pos = target2.x;
                y_pos = target2.y;
                break;
            }
            TPoint target3 = attackTarget.pos;
            target3.y += 1;
            dis = GetDistance(attacker.lastpos, target3);
            if (dis <= attacker.range + 0.1) {
                inplace = true;
                x_pos = target3.x;
                y_pos = target3.y;
                break;
            }
            TPoint target4 = attackTarget.pos;
            target4.x += 1;
            target4.y += 1;
            dis = GetDistance(attacker.lastpos, target4);
            if (dis <= attacker.range + 0.1) {
                inplace = true;
                x_pos = target4.x;
                y_pos = target4.y;
                break;
            }
            TPoint target5 = attackTarget.pos;
            target5.x += 1;
            dis = GetDistance(attacker.lastpos, target5);
            if (dis <= attacker.range + 0.1) {
                inplace = true;
                x_pos = target5.x;
                y_pos = target5.y;
                break;
            }
            TPoint target6 = attackTarget.pos;
            target6.x += 1;
            target6.y -= 1;
            dis = GetDistance(attacker.lastpos, target6);
            if (dis <= attacker.range + 0.1) {
                inplace = true;
                x_pos = target6.x;
                y_pos = target6.y;
                break;
            }
            TPoint target7 = attackTarget.pos;
            target7.y -= 1;
            dis = GetDistance(attacker.lastpos, target7);
            if (dis <= attacker.range + 0.1) {
                inplace = true;
                x_pos = target7.x;
                y_pos = target7.y;
                break;
            }
            TPoint target8 = attackTarget.pos;
            target8.x -= 1;
            target8.y -= 1;
            dis = GetDistance(attacker.lastpos, target8);
            if (dis <= attacker.range + 0.1) {
                inplace = true;
                x_pos = target8.x;
                y_pos = target8.y;
                break;
            }
        }
    }
    if (inplace) {
        info.myCommandList.addCommand(Attack, attacker.id, x_pos, y_pos);
    } else {
        TPoint movetarget;
        int distance1 = INT_MAX;
        if (attackTarget.type) {
            TPoint movetargetlist[16];
            movetargetlist[0].x = attackTarget.pos.x;
            movetargetlist[0].y = attackTarget.pos.y + 3;
            movetargetlist[1].x = attackTarget.pos.x + 1;
            movetargetlist[1].y = attackTarget.pos.y + 2;
            movetargetlist[2].x = attackTarget.pos.x + 2;
            movetargetlist[2].y = attackTarget.pos.y + 2;
            movetargetlist[3].x = attackTarget.pos.x + 2;
            movetargetlist[3].y = attackTarget.pos.y + 1;
            movetargetlist[4].x = attackTarget.pos.x + 3;
            movetargetlist[4].y = attackTarget.pos.y;
            movetargetlist[5].x = attackTarget.pos.x + 2;
            movetargetlist[5].y = attackTarget.pos.y - 1;
            movetargetlist[6].x = attackTarget.pos.x + 2;
            movetargetlist[6].y = attackTarget.pos.y - 2;
            movetargetlist[7].x = attackTarget.pos.x + 1;
            movetargetlist[7].y = attackTarget.pos.y - 2;
            movetargetlist[8].x = attackTarget.pos.x;
            movetargetlist[8].y = attackTarget.pos.y - 3;
            movetargetlist[9].x = attackTarget.pos.x - 1;
            movetargetlist[9].y = attackTarget.pos.y - 2;
            movetargetlist[10].x = attackTarget.pos.x - 2;
            movetargetlist[10].y = attackTarget.pos.y - 2;
            movetargetlist[11].x = attackTarget.pos.x - 2;
            movetargetlist[11].y = attackTarget.pos.y - 1;
            movetargetlist[12].x = attackTarget.pos.x - 3;
            movetargetlist[12].y = attackTarget.pos.y;
            movetargetlist[13].x = attackTarget.pos.x - 2;
            movetargetlist[13].y = attackTarget.pos.y + 1;
            movetargetlist[14].x = attackTarget.pos.x - 2;
            movetargetlist[14].y = attackTarget.pos.y + 2;
            movetargetlist[15].x = attackTarget.pos.x - 1;
            movetargetlist[15].y = attackTarget.pos.y + 2;
            for (int i = 0; i < 16; i++) {
                int temp_dis = GetDistance1(attacker.lastpos, movetargetlist[i]);
                if (!(info.pointInfo[movetargetlist[i].x][movetargetlist[i].y].occupied) && temp_dis < distance1) {
                    movetarget = movetargetlist[i];
                    distance1 = temp_dis;
                }
            }
        } else {
            TPoint movetargetlist[24];
            movetargetlist[0].x = attackTarget.pos.x - 4;
            movetargetlist[0].y = attackTarget.pos.y;
            movetargetlist[1].x = attackTarget.pos.x - 4;
            movetargetlist[1].y = attackTarget.pos.y + 1;
            movetargetlist[2].x = attackTarget.pos.x - 3;
            movetargetlist[2].y = attackTarget.pos.y + 2;
            movetargetlist[3].x = attackTarget.pos.x - 3;
            movetargetlist[3].y = attackTarget.pos.y + 3;
            movetargetlist[4].x = attackTarget.pos.x - 2;
            movetargetlist[4].y = attackTarget.pos.y + 3;
            movetargetlist[5].x = attackTarget.pos.x - 1;
            movetargetlist[5].y = attackTarget.pos.y + 4;
            movetargetlist[6].x = attackTarget.pos.x;
            movetargetlist[6].y = attackTarget.pos.y + 4;
            movetargetlist[7].x = attackTarget.pos.x + 1;
            movetargetlist[7].y = attackTarget.pos.y + 4;
            movetargetlist[8].x = attackTarget.pos.x + 2;
            movetargetlist[8].y = attackTarget.pos.y + 3;
            movetargetlist[9].x = attackTarget.pos.x + 3;
            movetargetlist[9].y = attackTarget.pos.y + 3;
            movetargetlist[10].x = attackTarget.pos.x + 3;
            movetargetlist[10].y = attackTarget.pos.y + 2;
            movetargetlist[11].x = attackTarget.pos.x + 4;
            movetargetlist[11].y = attackTarget.pos.y + 1;
            movetargetlist[12].x = attackTarget.pos.x + 4;
            movetargetlist[12].y = attackTarget.pos.y;
            movetargetlist[13].x = attackTarget.pos.x + 4;
            movetargetlist[13].y = attackTarget.pos.y - 1;
            movetargetlist[14].x = attackTarget.pos.x + 3;
            movetargetlist[14].y = attackTarget.pos.y - 2;
            movetargetlist[15].x = attackTarget.pos.x + 3;
            movetargetlist[15].y = attackTarget.pos.y - 3;
            movetargetlist[16].x = attackTarget.pos.x + 2;
            movetargetlist[16].y = attackTarget.pos.y - 3;
            movetargetlist[17].x = attackTarget.pos.x + 1;
            movetargetlist[17].y = attackTarget.pos.y - 4;
            movetargetlist[18].x = attackTarget.pos.x;
            movetargetlist[18].y = attackTarget.pos.y - 4;
            movetargetlist[19].x = attackTarget.pos.x - 1;
            movetargetlist[19].y = attackTarget.pos.y - 4;
            movetargetlist[20].x = attackTarget.pos.x - 2;
            movetargetlist[20].y = attackTarget.pos.y - 3;
            movetargetlist[21].x = attackTarget.pos.x - 3;
            movetargetlist[21].y = attackTarget.pos.y - 3;
            movetargetlist[22].x = attackTarget.pos.x - 3;
            movetargetlist[22].y = attackTarget.pos.y - 2;
            movetargetlist[23].x = attackTarget.pos.x - 4;
            movetargetlist[23].y = attackTarget.pos.y - 1;
            for (int i = 0; i < 24; i++) {
                int temp_dis = GetDistance1(attacker.lastpos, movetargetlist[i]);
                if (!(info.pointInfo[movetargetlist[i].x][movetargetlist[i].y].occupied) && temp_dis < distance1) {
                    movetarget = movetargetlist[i];
                    distance1 = temp_dis;
                }
            }
        }
        if (distance1 < 100) {
            MoveToTarget(info, attacker, movetarget);
            info.myCommandList.addCommand(Attack, attacker.id, attackTarget.pos.x, attackTarget.pos.y);
            info.myCommandList.addCommand(Attack, attacker.id, attackTarget.pos.x - 1, attackTarget.pos.y);
            info.myCommandList.addCommand(Attack, attacker.id, attackTarget.pos.x - 1, attackTarget.pos.y + 1);
            info.myCommandList.addCommand(Attack, attacker.id, attackTarget.pos.x, attackTarget.pos.y + 1);
            info.myCommandList.addCommand(Attack, attacker.id, attackTarget.pos.x + 1, attackTarget.pos.y + 1);
            info.myCommandList.addCommand(Attack, attacker.id, attackTarget.pos.x + 1, attackTarget.pos.y);
            info.myCommandList.addCommand(Attack, attacker.id, attackTarget.pos.x + 1, attackTarget.pos.y - 1);
            info.myCommandList.addCommand(Attack, attacker.id, attackTarget.pos.x, attackTarget.pos.y - 1);
            info.myCommandList.addCommand(Attack, attacker.id, attackTarget.pos.x - 1, attackTarget.pos.y - 1);
        }
    }
    //乱打
    TPoint tem;
    for (int i = 0; i < 50; i++) {
        for (int j = 0; j < 50; j++) {
            tem.x = i;
            tem.y = j;
            if (GetDistance(attacker.lastpos, tem) <= attacker.range + 0.1) {
                info.myCommandList.addCommand(Attack, attacker.id, tem.x, tem.y);
            }
        }
    }
    //撤退
    {
        vector<TPoint> tar1;
        tar1.push_back(info.pointInfo[attacker.lastpos.x][attacker.lastpos.y]);
        for (int x = attacker.lastpos.x - attacker.move_left; x < attacker.lastpos.x + attacker.move_left + 1; x++) {
            for (int y = attacker.lastpos.y - attacker.move_left;
                 y < attacker.lastpos.y + attacker.move_left + 1; y++) {
                if (x >= 0 && x < 50 && y >= 0 && y < 50) {
                    TPoint temp = info.pointInfo[x][y];
                    if (GetDistance1(attacker.lastpos, temp) > attacker.move_left)continue;
                    if (temp.occupied)continue;
                    tar1.push_back(temp);
                }
            }
        }

        int *score = new int[tar1.size()];
        for (int i = 0; i < tar1.size(); i++) {
            score[i] = GetDistance1(tar1[i], attackTarget.pos) * 10;
            if (tar1[i].land == River)score[i] -= (attacker.move - 1) * 10;
            if (tar1[i].land == Forest) {
                if (attacker.type == LightArcher)score[i] += 5;
                if (attacker.type == HeavyArcher)score[i] += 5;
            }
            if (tar1[i].land == Dorm) {
                if (attacker.type == HeavyArcher)score[i] += 5;
            }
            bool hidden = true;
            for (int j = 0; j < target.size(); j++) {
                if (GetDistance3(tar1[i], target[j].pos) * 2 + 1 <= target[j].FieldOfVision) {
                    hidden = false;
                    break;
                }
            }
            if (hidden) {
                score[i] = 10000;
            }
            score[i] -= GetDistance1(tar1[i], attacker.lastpos);
        }

        TPoint tarr;
        int scorr = -10000;
        for (int i = 0; i < tar1.size(); i++) {
            if (score[i] > scorr) {
                scorr = score[i];
                tarr = tar1[i];
            }
        }
        if (scorr > -9000) {
            MoveToTarget(info, attacker, tarr);
        }
        delete[] score;
    }
    return 0;
}

void cal_indiv_priority(Info& info) {
    for(int i = 0; i < mysod.size(); i++) {
        mysod[i].individual_priority.clear();
        for(int j = 0; j < target.size(); j++) {
            priority_and_index temp;
            temp.index = j;
            temp.pri = target[j].attackpriority / 100.0 //因为是10000/r^2 归一化
                       + 200.0 / GetDistance(target[j].pos, mysod[i].lastpos) - target[j].blood + mysod[i].attack - target[j].armor;
            mysod[i].individual_priority.push_back(temp);
        }
    }
}