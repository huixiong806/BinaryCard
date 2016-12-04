// playerA(Client).cpp : �������̨Ӧ�ó������ڵ㡣

#include <iostream> 
#include <vector>
#include <string>
#include <sstream>
#include <thread>
#include <map>
#include <RakPeerInterface.h>   // RakNet Peer  
#include <RakNetTypes.h>          
#include <MessageIdentifiers.h> // RakNet�Զ�����Ϣö�ٶ��崦  
#include <BitStream.h>          // RakNet��Ϣ����Bit������
#define INFINITE_LOOP_START while(1){
#define INFINITE_LOOP_END }
using namespace std;
using namespace RakNet;
typedef void(*msgGetCallBack)(Packet*, RakPeerInterface*);
map<unsigned char, msgGetCallBack> targetFunction;
RakPeerInterface* m_peer = RakPeerInterface::GetInstance();
// �Զ�����Ϣö��ֵ����ϢID
enum ChatMessagesDefine
{
	MSG_CHAT = ID_USER_PACKET_ENUM + 1,        // ��ϢID��RakNet��������һ��ö�ٿ�ʼ  	
											   //C TO S
	MSG_A_MOVEMENT,
	//S TO C
	MSG_BC_MOVEMENT,
	MSG_CHAT_START,
	MSG_MOVEMENT_REQUEST,
	MSG_GAME_DIFFICULTY,
	MSG_CUT_DOWN_CONNECTION
};
enum GameState
{
	GAME_STATE_MAIN_THREAD_INPUT_LOCK,
	GAME_STATE_CHATING,
	GAME_STATE_WATING_MOVMENT,
	GAME_STATE_WATING_OTHER,
};
class BinaryCardGameA
{
public:
	int maxRound, nowRound;
	int winNeed;
	int gameState;
	int bigRound;
	bool otherHasMove;
	vector<bool> sequence;
	vector<bool> winLoseArray;
	vector<bool> movementsA;
	vector<bool> movementsB;
	BinaryCardGameA()
	{
		nowRound = maxRound = 0;
		winNeed = 0;
		winLoseArray.clear();
		movementsA.clear();
		movementsB.clear();
		sequence.clear();
		gameState = GAME_STATE_MAIN_THREAD_INPUT_LOCK;
	}
	void printGameRule(ostream& os)
	{
		os << "[��Ϣ]����Ϸ�ɿ�����orangebird�ı���IBM Ponder This" << endl;
		os << "[��Ϣ]��Ȩ����,ʹ��MITЭ�鷢��" << endl;
		//os << "�����濪����:your name" << endl;
		os << "[��Ϣ]��Ϸ����" << endl;
		os << "������Ϸ������" << maxRound << "��" << endl;
		os << "��ÿһ����,A,B�Լ������˽���������ʾһ������(ֻ����0����1)������" << endl;
		os << "�����˳�ʾ������ͬ����A��B����һ�֣����򲻵÷�" << endl;
		os << "���Ŀ������B��ϣ�Ӯ��" << maxRound << "��������" << winNeed << "�֣��������ɻ�ʤ" << endl;
		os << "�ڵ�һ�ֿ�ʼʱ��B���õ�һ����0��1��ɵ����У���ʾ������������" << maxRound << "��" << endl;
		os << "�н����ʾ�����֣����㲻��õ�����BΨһ���㴫����Ϣ�Ļ���ֻ������ÿ��������ʾ������" << endl;
		os << "��һ�ֿ�ʼǰ�����B�������Ƶ�ʱ������������ǵ���Ϸ���ԡ�" << endl;
		os << "���Ϊ�˼�С����������Ӱ�죬��Ϸ������ȫ��ͬ�Ĺ������3�֣�ȫ����ʤ��������ʤ����" << endl;
	}
	void newGame(int winNeed_, int maxRound_)
	{
		nowRound = 1;
		winNeed = winNeed_;
		maxRound = maxRound_;
		sequence.clear();
		movementsA.clear();
		movementsB.clear();
		winLoseArray.clear();
		cout << "[��Ϸ]��һ����Ϸ�Ѿ���ʼ" << endl;
		cout << "[��Ϸ]���ڽ����������ʱ��,����\"��������\"����ֹͣ����" << endl;
		gameState = GAME_STATE_CHATING;
	}
	void generateSequence()
	{
		for (int i = 1; i <= maxRound; ++i)
			sequence.push_back(rand() % 2);
	}
	void judge()
	{
		bool result = movementsA[nowRound - 1] == movementsB[nowRound - 1];
		result &= movementsA[nowRound - 1] == sequence[nowRound - 1];
		winLoseArray.push_back(result);
	}
}game;
void nextRound(RakPeerInterface* peer)
{
	game.otherHasMove = false;
	cout << "[��Ϸ]��" << game.nowRound << "��" << endl;
	if (game.nowRound != 1)
	{
		cout << "[��Ϸ]�������зֱ��ʾǰ" << game.nowRound - 1 << "�ֵ��鱨" << endl;
		cout << "A�ĳ�������:" << endl;
		for (int i = 0; i < game.movementsA.size(); ++i)
			cout << game.movementsA[i] << " ";
		cout << endl;
		cout << "B�ĳ�������:" << endl;
		for (int i = 0; i < game.movementsB.size(); ++i)
			cout << game.movementsB[i] << " ";
		cout << endl;
		cout << "Robot������:" << endl;
		for (int i = 0; i < game.sequence.size(); ++i)
			cout << game.sequence[i] << " ";
		cout << endl;
		cout << "A,B��ʤ����:" << endl;
		for (int i = 0; i < game.winLoseArray.size(); ++i)
			cout << game.winLoseArray[i] << " ";
		cout << endl;
	}
}
void resetAll()
{
	game = BinaryCardGameA();
	BitStream bsOut;
	bsOut.Write((MessageID)MSG_CUT_DOWN_CONNECTION);
	m_peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, UNASSIGNED_SYSTEM_ADDRESS, true);
	//m_peer->Shutdown(300);
	cout << "[��Ϣ]�����Ͽ������B������" << endl;
}
void afterGettingAllMovements()
{
	game.judge();
	if (game.nowRound == game.maxRound)
	{
		int totalWin = 0;
		for (int i = 0; i < game.winLoseArray.size(); ++i)
			totalWin += game.winLoseArray[i];
		if (totalWin >= game.winNeed)
		{
			if (game.bigRound == 3)
			{
				cout << "[��Ϸ]��ϲ������B��ʤ����,���������ʤ��!" << endl;
				resetAll();
			}
			else
			{
				cout << "[��Ϸ]��ϲ������B����˱��ֵ�ʤ��,����ʼ��һ��!" << endl;
				++game.bigRound;
				game.newGame(game.winNeed, game.maxRound);
			}
		}
		else {
			cout << "[��Ϸ]���ź�,���ڱ�����ʧ����,��Ϸ����!" << endl;
			resetAll();
		}
	}
	else {
		++game.nowRound;
		nextRound(m_peer);
		game.gameState = GAME_STATE_MAIN_THREAD_INPUT_LOCK;
	}
}
void sendMovement()
{
	BitStream bsOut;
	ostringstream format_messege;
	format_messege << game.movementsA[game.nowRound - 1];  //������Ϣ���ַ��������ÿո�������˴�Ϊ�������ٶȸ߶�V, hΪdouble��      
	bsOut.Write((MessageID)MSG_A_MOVEMENT);
	bsOut.Write(format_messege.str().c_str());
	m_peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, UNASSIGNED_SYSTEM_ADDRESS, true);
}
void stopChat(RakPeerInterface* peer)
{
	if (game.gameState == GAME_STATE_CHATING)
		cout << "[��Ϸ]�Է���ֹ�����飬�㻹������Է��������һ�仰" << endl;
	cout << "[��Ϸ]����ʱ���������" << game.bigRound << "����Ϸ��ʽ��ʼ!" << endl;
	if(game.gameState == GAME_STATE_CHATING)
		game.gameState = GAME_STATE_MAIN_THREAD_INPUT_LOCK;
	nextRound(peer);
}
void movementGot(Packet* packet, RakPeerInterface* peer)
{
	bool movementB,movementC;
	RakString rs;
	BitStream bsIn(packet->data, packet->length, false);
	bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
	bsIn.Read(rs);  //��ȡ�ַ���           
	istringstream input_str(rs.C_String()); //�����ַ�����  
	input_str >> movementB>> movementC; //��ȡ��Ҫ�ı��� 
	game.movementsB.push_back(movementB);
	game.sequence.push_back(movementC);
	cout << "[��Ϸ]���B��ʾ������:" << movementB << endl;
	cout << "[��Ϸ]�����˳�ʾ������:" << movementC << endl;
	game.gameState = GAME_STATE_MAIN_THREAD_INPUT_LOCK;
	afterGettingAllMovements();
}
void serverLostConnection(Packet* packet, RakPeerInterface* peer)
{
	cout << "[����]�����B�������Ѿ���ʧ!" << endl;
}
void chatMessage(Packet* packet, RakPeerInterface* peer)
{
	RakString rs;
	BitStream bsIn(packet->data, packet->length, false);
	bsIn.IgnoreBytes(sizeof(MessageID));
	bsIn.Read(rs);  //��ȡ�ַ���           
	istringstream iss(rs.C_String()); //�����ַ�����  
	string msg;
	iss >> msg; //��ȡ��Ҫ�ı��� 
	if (game.gameState == GameState::GAME_STATE_CHATING)
		cout << "[���B]" << msg << endl;
	else cout << "[���B]" << "..." << endl;
	if (msg == "��������")
		stopChat(peer);
}

void movementRequest(Packet* packet, RakPeerInterface* peer)
{
	cout << "[��Ϸ]�������㱾��Ҫ��������,һ������,ֻ����1����0" << endl;
	game.gameState = GAME_STATE_WATING_MOVMENT;
}
void connectionAccepted(Packet* packet, RakPeerInterface* peer)
{
	cout << "[����]�����ͬ������������" << std::endl;
	cout << "[��Ϣ]�ȴ����Bѡ����Ϸ�Ѷ�" << std::endl;
}
void difficultyGot(Packet* packet, RakPeerInterface* peer)
{
	RakString rs;
	BitStream bsIn(packet->data, packet->length, false);
	bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
	bsIn.Read(rs);  //��ȡ�ַ���           
	istringstream input_str(rs.C_String()); //�����ַ�����  
	int n, m;
	input_str >> n>>m; //��ȡ��Ҫ�ı��� 
	cout << "[��Ϸ]���Bѡ������Ϸ���Ѷȣ�������Ҫ��" << n<< "����Ӯ����" << m<< "��" << endl;
	game.maxRound = n;
	game.winNeed = m;
	game.bigRound = 1;
	game.printGameRule(cout);
	game.newGame(m, n);
}
void cutDownConnection(Packet* packet, RakPeerInterface* peer)
{
	peer->Shutdown(300);
	cout << "[����]�ѶϿ��ͷ����������ӣ���ر���Ϸ" << std::endl;
}
void initMap()
{
	targetFunction[ID_CONNECTION_REQUEST_ACCEPTED] = connectionAccepted;
	targetFunction[ID_DISCONNECTION_NOTIFICATION] = serverLostConnection;
	targetFunction[ID_CONNECTION_LOST] = serverLostConnection;
	targetFunction[MSG_CHAT] = chatMessage;
	targetFunction[MSG_BC_MOVEMENT] = movementGot;
	targetFunction[MSG_MOVEMENT_REQUEST] = movementRequest;
	targetFunction[MSG_GAME_DIFFICULTY] = difficultyGot;
	targetFunction[MSG_CUT_DOWN_CONNECTION] = cutDownConnection;
}
void getMsg()
{
	Packet* m_packet;
	INFINITE_LOOP_START
		for (m_packet = m_peer->Receive(); m_packet; m_peer->DeallocatePacket(m_packet), m_packet = m_peer->Receive())
		{
			if (targetFunction.find(m_packet->data[0]) == targetFunction.end())
				break;
			targetFunction[m_packet->data[0]](m_packet, m_peer);
		}
	INFINITE_LOOP_END
}
int main()
{
	SocketDescriptor sd;
	string SERVER_IP;
	unsigned short PORT;
	cout << "[��Ϣ]�����������IP��ַ�Ͷ˿ڣ��ÿո����" << std::endl;
	cin >> SERVER_IP >> PORT;
	initMap();
	m_peer->Startup(1, &sd, 1);
	m_peer->Connect(SERVER_IP.c_str(), PORT, 0, 0);
	thread getMessage(getMsg);
	getMessage.detach();
	cout << "[����]�Ѿ���������������ӣ�" << std::endl;
	cout << "[��Ϣ]�ȴ����ӽ��..." << endl;
	INFINITE_LOOP_START
	if (game.gameState == GAME_STATE_MAIN_THREAD_INPUT_LOCK || game.gameState == GAME_STATE_WATING_OTHER)
	{
		Sleep(100);
		continue;
	}
	if (game.gameState == GAME_STATE_WATING_MOVMENT)
	{
		bool movement;
		while (cin >> movement)
		{
			if (cin.fail())
			{
				cout << "[��Ϣ]���벻�Ϸ�����Ҫһ��0~1֮�������" << endl;
				cin.clear();
				cin.sync();
			}
			else break;
		}
		game.movementsA.push_back(movement);
		game.gameState = GAME_STATE_WATING_OTHER;
		sendMovement();
	}
	else if (game.gameState == GAME_STATE_CHATING)
	{
		bool movement;
		string msg;
		cin >> msg;
		//��������������  
		RakNet::BitStream bsOut;
		bsOut.Write((MessageID)MSG_CHAT);
		ostringstream format_messege;
		format_messege << msg;
		bsOut.Write(format_messege.str().c_str());
		m_peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true);
		if (msg == "��������"&&game.gameState == GAME_STATE_CHATING)
		{
			game.gameState = GAME_STATE_MAIN_THREAD_INPUT_LOCK;
			stopChat(m_peer);
		}
	}
	INFINITE_LOOP_END
	return 0;
}