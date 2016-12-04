// playerB(Host).cpp : �������̨Ӧ�ó������ڵ㡣

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
class BinaryCardGameB
{
public:
	int maxRound, nowRound;
	int winNeed;
	int gameState;
	int bigRound;
	vector<bool> sequence;
	vector<bool> winLoseArray;
	vector<bool> movementsA;
	vector<bool> movementsB;
	BinaryCardGameB()
	{
		nowRound=maxRound = 0;
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
		os << "������Ϸ������"<< maxRound <<"��" << endl;
		os << "��ÿһ����,A,B�Լ������˽���������ʾһ������(ֻ����0����1)������" << endl;
		os << "�����˳�ʾ������ͬ����A��B����һ�֣����򲻵÷�" << endl;
		os << "���Ŀ������A��ϣ�Ӯ��"<< maxRound << "��������"<< winNeed <<"�֣��������ɻ�ʤ" << endl;
		os << "�ڵ�һ�ֿ�ʼʱ���㽫�õ�һ����0��1��ɵ����У���ʾ������������" << maxRound << "��"<<endl;
		os << "�н����ʾ�����֡���A����õ���������Ψһ��A������Ϣ�Ļ���ֻ������ÿ��������ʾ������"<<endl;
		os << "��һ�ֿ�ʼǰ�����A�������Ƶ�ʱ������������ǵ���Ϸ���ԡ�" << endl;
		os << "���Ϊ�˼�С����������Ӱ�죬��Ϸ������ȫ��ͬ�Ĺ������3�֣�ȫ����ʤ��������ʤ����" << endl;
	}
	void newGame(int winNeed_,int maxRound_)
	{
		nowRound = 1;
		winNeed = winNeed_;
		maxRound = maxRound_;
		sequence.clear();
		movementsA.clear();
		movementsB.clear();
		winLoseArray.clear();
		generateSequence();
		cout << "[��Ϸ]��һ����Ϸ�Ѿ���ʼ" << endl;
		cout << "[��Ϸ]���ڽ����������ʱ��,����\"��������\"����ֹͣ����" << endl;
		gameState = GAME_STATE_CHATING;
		BitStream bsOut;
		bsOut.Write((MessageID)MSG_CHAT_START);
		m_peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, UNASSIGNED_SYSTEM_ADDRESS, true);
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
void startNewGame()
{
	cout << "[��Ϣ]��������Ϸ�Ѷ�,��ΧΪ0~3֮�������" << endl;
	int difficulty;
	while (cin >> difficulty)
	{
		if (cin.fail() || difficulty < 0 || difficulty>3)
		{
			cout << "[��Ϣ]���벻�Ϸ�����Ҫһ��0~3֮�������" << endl;
			cin.clear();
			cin.sync();
		}
		else break;
	}
	int m[4] = { 4,6,9,13 };
	int n[4] = { 7,9,13,18 };
	cout<< "[��Ϸ]��ѡ�����Ѷ�"<<difficulty<<",������A��Ҫ��" << n[difficulty] << "����Ӯ����" << m[difficulty] << "��" << endl;
	BitStream bsOut;
	ostringstream format_messege;
	format_messege <<n[difficulty]<<" "<<m[difficulty] << endl;
	bsOut.Write((MessageID)MSG_GAME_DIFFICULTY);
	bsOut.Write(format_messege.str().c_str());
	m_peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, UNASSIGNED_SYSTEM_ADDRESS, true);
	game.bigRound = 1;
	game.maxRound = n[difficulty];
	game.winNeed = m[difficulty];
	game.printGameRule(cout);
	game.newGame(m[difficulty], n[difficulty]);
}
void nextRound(RakPeerInterface* peer)
{
	cout << "[��Ϸ]��"<<game.nowRound <<"��" << endl;
	if (game.nowRound != 1)
	{
		cout << "[��Ϸ]�������зֱ��ʾǰ"<< game.nowRound-1<<"�ֵ��鱨��Ϊ�˷�����䣬���ٴ���ʾ�����˳����ֵ�����" << endl;
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
	BitStream bsOut;
	bsOut.Write((MessageID)MSG_MOVEMENT_REQUEST);
	peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, UNASSIGNED_SYSTEM_ADDRESS, true);
	cout << "[��Ϸ]�ȴ����A��ʾ����" << endl;
	game.gameState = GAME_STATE_WATING_OTHER;
}
void resetAll()
{
	game = BinaryCardGameB();
	BitStream bsOut;
	bsOut.Write((MessageID)MSG_CUT_DOWN_CONNECTION);
	m_peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, UNASSIGNED_SYSTEM_ADDRESS, true);
	cout << "[��Ϣ]���A�����Ͽ��ͱ���������" << endl;
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
				cout << "[��Ϸ]��ϲ������A��ʤ����,���������ʤ��!" << endl;
				resetAll();
			}
			else
			{
				cout << "[��Ϸ]��ϲ������A����˱��ֵ�ʤ��,����ʼ��һ��!" << endl;
				++game.bigRound;
				game.newGame(game.winNeed , game.maxRound);
			}
		}else{
			cout << "[��Ϸ]���ź�,���ڱ�����ʧ����,��Ϸ����!" << endl;
			resetAll();
		}
	}else{
		++game.nowRound;
		nextRound(m_peer);
		game.gameState = GAME_STATE_MAIN_THREAD_INPUT_LOCK;
	}
}
void sendMovement()
{
	BitStream bsOut;
	ostringstream format_messege;
	format_messege <<game.movementsB[game.nowRound-1]<<" "<<game.sequence[game.nowRound-1];  //������Ϣ���ַ��������ÿո�������˴�Ϊ�������ٶȸ߶�V, hΪdouble��      
	bsOut.Write((MessageID)MSG_BC_MOVEMENT);
	bsOut.Write(format_messege.str().c_str());
	m_peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, UNASSIGNED_SYSTEM_ADDRESS, true);
}
void stopChat(RakPeerInterface* peer)
{
	if (game.gameState == GAME_STATE_CHATING)
		cout << "[��Ϸ]�Է���ֹ�����飬�㻹������Է��������һ�仰" << endl;
	if (game.gameState == GAME_STATE_CHATING)
		game.gameState = GAME_STATE_MAIN_THREAD_INPUT_LOCK;
	cout << "[��Ϸ]����ʱ���������"<<game.bigRound<<"����Ϸ��ʽ��ʼ!" << endl;
	cout << "[��Ϸ]��õ���һ��01����:";
	for (int i = 0; i < game.sequence.size(); ++i)
		cout << game.sequence[i] << " ";
	cout << endl;
	nextRound(peer);
}
void movementGot(Packet* packet, RakPeerInterface* peer)
{
	bool movement;
	RakString rs;
	BitStream bsIn(packet->data, packet->length, false);
	bsIn.IgnoreBytes(sizeof(MessageID));
	bsIn.Read(rs);  //��ȡ�ַ���           
	istringstream input_str(rs.C_String()); //�����ַ�����  
	input_str >> movement; //��ȡ��Ҫ�ı��� 
	game.movementsA.push_back(movement);
	cout << "[��Ϸ]���A��ʾ������:" << movement << endl;
	cout << "[��Ϸ]�������㱾��Ҫ��������,һ������,ֻ����1����0" << endl;
	game.gameState = GAME_STATE_WATING_MOVMENT;
}
void clientLostConnection(Packet* packet, RakPeerInterface* peer)
{
	cout << "[����]�����A�������Ѿ���ʧ!" << endl;
}
void newIncomingConnection(Packet* packet, RakPeerInterface* peer)
{
	cout << "[����]���A�ɹ����ӵ��˱���!" << endl;
	startNewGame();
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
	if(game.gameState ==GameState::GAME_STATE_CHATING)
		cout << "[���A]" << msg << endl;
	else cout << "[���A]" << "..." << endl;
	if (msg == "��������")
		stopChat(peer);
}
void initMap()
{
	targetFunction[ID_DISCONNECTION_NOTIFICATION] = clientLostConnection;
	targetFunction[ID_CONNECTION_LOST] = clientLostConnection;
	targetFunction[ID_NEW_INCOMING_CONNECTION] = newIncomingConnection;
	targetFunction[MSG_CHAT] = chatMessage;
	targetFunction[MSG_A_MOVEMENT] = movementGot;
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
	string SERVER_IP;
	unsigned short PORT;
	cout << "[��Ϣ]�����뱾��IP��ַ�Ͷ˿ڣ��ÿո����" << std::endl;
	cin >> SERVER_IP >> PORT;
	cout << "[��Ϣ]��������������" << std::endl;
	//����������  
	m_peer->Startup(1, &SocketDescriptor(PORT, SERVER_IP.c_str()), 1);
	//�������������  
	m_peer->SetMaximumIncomingConnections(1);
	cout << "[��Ϣ]�����������ɹ���" << std::endl;
	cout << "[��Ϣ]�ȴ����A���ӵ�����" << endl;
	thread getMessage(getMsg);
	getMessage.detach();
	initMap();
	INFINITE_LOOP_START
	if (game.gameState == GAME_STATE_MAIN_THREAD_INPUT_LOCK|| game.gameState == GAME_STATE_WATING_OTHER)
	{
		Sleep(100);
		continue;
	}
	if (game.gameState == GAME_STATE_WATING_MOVMENT)
	{
		bool movement;
		while (cin>>movement)
		{
			if (cin.fail())
			{
				cout << "[��Ϣ]���벻�Ϸ�����Ҫһ��0~1֮�������" << endl;
				cin.clear();
				cin.sync();
			}
			else break;
		}
		game.movementsB.push_back(movement);
		game.gameState = GAME_STATE_MAIN_THREAD_INPUT_LOCK;
		sendMovement();
		afterGettingAllMovements();
	}
	if (game.gameState == GAME_STATE_CHATING)
	{
		string msg;
		cin >> msg;
		BitStream bsOut;
		bsOut.Write((MessageID)MSG_CHAT);
		ostringstream format_messege;
		format_messege <<msg;
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