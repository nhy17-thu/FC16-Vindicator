#ifndef DEFINITION_H
#define DEFINITION_H
#include <vector>
#include <string>
#include <stdexcept>
#include <map>
#include <set>
#include <iostream>

using namespace std;


typedef double TResourceD;  //double ����Դ���������ڲ�����
typedef int    TResourceI;  //int    ����Դ����������ʾ


typedef double TLength;
typedef double TXP;//����ֵ
typedef int    TRange;//���
typedef int    TMove;//�ƶ���
typedef int    TBlood;//����ֵ
typedef int    TAttack;//������
typedef int    TArmor;//����ֵ
typedef int    TPointID;//����ID
typedef int    TPlayerID;//ѡ��ID
typedef int    TTowerID;//����ID
typedef int    TSoldierID;//ʿ��ID



typedef int    TPosition;//����ֵ

typedef int    TTowerLevel;//���ĵȼ�
typedef int    TPopulation;//�˿�ֵ
typedef double TResourceT;//������Դ�ظ���d

typedef string TMapID;
typedef int    TMap;
typedef int    TLevel;  //�������Եȼ�
typedef int    TRound;  //�غ���




enum TSoldierType
{
	None=0
	,LightInfantry=1  //�Ჽ��
	,LightArcher=2   //�ṭ��
	,LightKnight=3   //�����
	,Mangonel=4      //Ͷʯ��
	,HeavyInfantry=5 //�ز���
	,HeavyArcher=6   //�ع���
	,HeavyKnight=7   //�����
	
};

enum TLandForm
{
	Road      //��·
	,Forest   //ɭ��
	,River    //����
	,Mountain //ɽ��
	,Dorm     //����
	,Classroom//����
	,TowerLand//��
};
enum TOccupiedType
{
	none      //û��ռ��
	,soldier  //ʿ��
	,tower    //��
	,unKnown  //��Ұ֮��
};
struct TPoint
{

	TPosition x;
	TPosition y;
	TLandForm land;

	bool occupied;
	bool visible;//�Ƿ�ɼ�
	TOccupiedType occupied_type;//��Ӧ��ռ������
	TSoldierID soldier;//��Ӧ��soldier
	TTowerID tower;//��Ӧ��tower

};

struct TSoldier
{
	TSoldierID id;
	TPlayerID owner;//ӵ����

	TRange range;//���
	TMove move_ability;//�ƶ���
	TAttack attack; //������
	TArmor armor;//����
	TBlood blood;//����ֵ
	TXP    experience;//����ֵ
	TLevel  level;//�ȼ�

	TSoldierType type;//ʿ������

	TPoint position;//���ڵ�
	TMove move_left;//���غ�ʣ���ƶ���

	TPosition x_position;//x����
	TPosition y_position;//y����

	bool attackable;//�Ƿ��ܹ�����

};
struct PlayerInfo
{
	TPlayerID id;

	bool live;//�Ƿ�����

    int rank;          //��ѡ������/������Դ/���������ϵ�/��1��ʼ
	double score;//����

	TResourceD resource;//��Դ��
	TPopulation max_population;//����˿�ֵ
	TPopulation population;//��ǰ�˿�ֵ
	int tower_num;//��������
	int soldier_num;//ʿ������
	int kill_num;//��ɱ��Ŀ
	int survival_round;//���غ���

	
};
struct TowerInfo
{
	TTowerID id;
	TPlayerID owner;

	TTowerLevel level;
	TPopulation population;//�ṩ���˿�
	TResourceT resource_income;//��Դ���� 
	int boost;//��������ֵ

	TPoint position;

	TPosition x_position;
	TPosition y_position;
	TBlood blood;
	TBlood max_blood;

	bool recruiting;//�Ƿ�������ļ
	int recruiting_round;//ʣ�����ļ�غ�,û����ļʱΪ-1��
	TSoldierType soldier;//����ļ��ʿ������
};
//��������
enum CommandType
{
	Attack       //����(ʿ��id��Ŀ���x��Ŀ���y)
	, Move       //�ƶ���ʿ��id�������ƶ����룩
	, Upgrade    //������ʳ��id��
	, Produce    //������ʳ��id��ʿ�����ͣ�
};
enum MoveDirection
{
	UP=0        
	,DOWN=1
	,LEFT=2
	,RIGHT=3
};
//�������������Ϣ
struct Command
{
	Command(CommandType _type, vector<int> _parameters) :
		type(_type), parameters(_parameters) {}
	Command(){}
	CommandType type;
	vector<int> parameters;  //����
};

//�����б�
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
			throw std::out_of_range("��������ʱԽ��");
		return m_commands[n];
	}
public:
	vector<Command> m_commands;
};
struct Info
{
	int playerSize;  //�������
	int playerAlive; //ʣ�������
	TPlayerID myID;  //ѡ��ID��
	
	
	TRound round;     //�غ���
	CommandList myCommandList;
	vector<PlayerInfo> playerInfo;   //������Ϣ
	vector<TowerInfo> towerInfo; //������Ϣ
	vector<vector<TPoint>> pointInfo;//������Ϣ
	vector<TSoldier> soldierInfo;//ʿ����Ϣ
	int towerNum;    //����������
	
};



#endif // DEFINITION_H
