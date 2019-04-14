#ifndef DEFINITION_H
#define DEFINITION_H
#include <vector>
#include <string>
#include <stdexcept>
#include <map>
#include <set>
#include <iostream>

using namespace std;


typedef double TResourceD;  //double 的资源数，用于内部操作
typedef int    TResourceI;  //int    的资源数，用于显示


typedef double TLength;
typedef double TXP;//经验值
typedef int    TRange;//射程
typedef int    TMove;//移动力
typedef int    TBlood;//生命值
typedef int    TAttack;//攻击力
typedef int    TArmor;//护甲值
typedef int    TPointID;//格子ID
typedef int    TPlayerID;//选手ID
typedef int    TTowerID;//塔的ID
typedef int    TSoldierID;//士兵ID



typedef int    TPosition;//坐标值

typedef int    TTowerLevel;//塔的等级
typedef int    TPopulation;//人口值
typedef double TResourceT;//塔的资源回复量d

typedef string TMapID;
typedef int    TMap;
typedef int    TLevel;  //各项属性等级
typedef int    TRound;  //回合数




enum TSoldierType
{
	None=0
	,LightInfantry=1  //轻步兵
	,LightArcher=2   //轻弓兵
	,LightKnight=3   //轻骑兵
	,Mangonel=4      //投石机
	,HeavyInfantry=5 //重步兵
	,HeavyArcher=6   //重弓兵
	,HeavyKnight=7   //重骑兵
	
};

enum TLandForm
{
	Road      //道路
	,Forest   //森林
	,River    //河流
	,Mountain //山地
	,Dorm     //宿舍
	,Classroom//教室
	,TowerLand//塔
};
enum TOccupiedType
{
	none      //没被占据
	,soldier  //士兵
	,tower    //塔
	,unKnown  //视野之外
};
struct TPoint
{

	TPosition x;
	TPosition y;
	TLandForm land;

	bool occupied;
	bool visible;//是否可见
	TOccupiedType occupied_type;//对应的占据类型
	TSoldierID soldier;//对应的soldier
	TTowerID tower;//对应的tower

};

struct TSoldier
{
	TSoldierID id;
	TPlayerID owner;//拥有者

	TRange range;//射程
	TMove move_ability;//移动力
	TAttack attack; //攻击力
	TArmor armor;//护甲
	TBlood blood;//生命值
	TXP    experience;//经验值
	TLevel  level;//等级

	TSoldierType type;//士兵类型

	TPoint position;//所在点
	TMove move_left;//本回合剩余移动力

	TPosition x_position;//x坐标
	TPosition y_position;//y坐标

	bool attackable;//是否能够攻击

};
struct PlayerInfo
{
	TPlayerID id;

	bool live;//是否灭亡

    int rank;          //该选手排名/按总资源/包括触手上的/从1开始
	double score;//积分

	TResourceD resource;//资源量
	TPopulation max_population;//最大人口值
	TPopulation population;//当前人口值
	int tower_num;//塔的数量
	int soldier_num;//士兵数量
	int kill_num;//击杀数目
	int survival_round;//存活回合数

	
};
struct TowerInfo
{
	TTowerID id;
	TPlayerID owner;

	TTowerLevel level;
	TPopulation population;//提供的人口
	TResourceT resource_income;//资源收入 
	int boost;//征兵加速值

	TPoint position;

	TPosition x_position;
	TPosition y_position;
	TBlood blood;
	TBlood max_blood;

	bool recruiting;//是否正在征募
	int recruiting_round;//剩余的征募回合,没有征募时为-1；
	TSoldierType soldier;//在征募的士兵类型
};
//命令种类
enum CommandType
{
	Attack       //攻击(士兵id，目标点x，目标点y)
	, Move       //移动（士兵id，方向，移动距离）
	, Upgrade    //升级（食堂id）
	, Produce    //生产（食堂id，士兵类型）
};
enum MoveDirection
{
	UP=0        
	,DOWN=1
	,LEFT=2
	,RIGHT=3
};
//保存命令相关信息
struct Command
{
	Command(CommandType _type, vector<int> _parameters) :
		type(_type), parameters(_parameters) {}
	Command(){}
	CommandType type;
	vector<int> parameters;  //参数
};

//命令列表
class CommandList
{
public:
	void addCommand(CommandType _type, vector<int> _parameters)
	{
		Command c;
		c.type = _type;
		c.parameters = _parameters;
		m_commands.push_back(c);
	}
	void addCommand(CommandType _type, int para1)
	{
		Command c;
		c.type = _type;
		c.parameters.push_back(para1);
		m_commands.push_back(c);
	}
	void addCommand(CommandType _type, int para1, int para2)
	{
		Command c;
		c.type = _type;
		c.parameters.push_back(para1);
		c.parameters.push_back(para2);
		m_commands.push_back(c);
	}
	void addCommand(CommandType _type, int para1, int para2, int para3)
	{
		Command c;
		c.type = _type;
		c.parameters.push_back(para1);
		c.parameters.push_back(para2);
		c.parameters.push_back(para3);
		m_commands.push_back(c);
	}
	void addCommand(CommandType _type,int para1,MoveDirection direction,int para3)
	{
		Command c;
		c.type = _type;
		int para2;
		switch (direction)
		{
		case UP:
			para2 = 0;
			break;
		case DOWN:
			para2 = 1;
			break;
		case LEFT:
			para2 = 2;
			break;
		case RIGHT:
			para2 = 3;
			break;
		default:
			break;
		}
		c.parameters.push_back(para1);
		c.parameters.push_back(para2);
		c.parameters.push_back(para3);
		m_commands.push_back(c);
	}
	void addCommand(CommandType _type,int para1,TSoldierType soldier_type)
	{
		Command c;
		c.type = _type;
		int para2;
		switch (soldier_type)
		{
		case LightInfantry:
			para2 = 1;
			break;
		case LightArcher:
			para2 = 2;
			break;
		case LightKnight:
			para2 = 3;
			break;
		case Mangonel:
			para2 = 4;
			break;
		case HeavyInfantry:
			para2 = 5;
			break;
		case HeavyArcher:
			para2 = 6;
			break;
		case HeavyKnight:
			para2 = 7;
			break;
		default:
			break;
		}
		c.parameters.push_back(para1);
		c.parameters.push_back(para2);
		m_commands.push_back(c);
	}
	void removeCommand(int n)
	{
		m_commands.erase(m_commands.begin() + n);
	}
	int  size() const { return int(m_commands.size()); }
	vector<Command>::iterator begin() { return m_commands.begin(); }
	vector<Command>::iterator end() { return m_commands.end(); }
	vector<Command>::const_iterator  begin()const { return m_commands.cbegin(); }
	vector<Command>::const_iterator end() const  { return m_commands.cend(); }
	Command& operator[](int n)
	{
		if (n < 0 || size() <= n)
			throw std::out_of_range("访问命令时越界");
		return m_commands[n];
	}
public:
	vector<Command> m_commands;
};
struct Info
{
	int playerSize;  //总玩家数
	int playerAlive; //剩余玩家数
	TPlayerID myID;  //选手ID号
	
	
	TRound round;     //回合数
	CommandList myCommandList;
	vector<PlayerInfo> playerInfo;   //势力信息
	vector<TowerInfo> towerInfo; //兵塔信息
	vector<vector<TPoint>> pointInfo;//格子信息
	vector<TSoldier> soldierInfo;//士兵信息
	int towerNum;    //兵塔总数量
	
};



#endif // DEFINITION_H
