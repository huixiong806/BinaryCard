// playerA(Client).cpp : 定义控制台应用程序的入口点。

#include <iostream> 
#include <vector>
#include <string>
#include <sstream>
#include <thread>
#include <map>
#include <RakPeerInterface.h>   // RakNet Peer  
#include <RakNetTypes.h>          
#include <MessageIdentifiers.h> // RakNet自定义消息枚举定义处  
#include <BitStream.h>          // RakNet消息包的Bit数据流
#define INFINITE_LOOP_START while(1){
#define INFINITE_LOOP_END }
using namespace std;
using namespace RakNet;
typedef void(*msgGetCallBack)(Packet*, RakPeerInterface*);
map<unsigned char, msgGetCallBack> targetFunction;
RakPeerInterface* m_peer = RakPeerInterface::GetInstance();
// 自定义消息枚举值，消息ID
enum ChatMessagesDefine
{
	MSG_CHAT = ID_USER_PACKET_ENUM + 1,        // 消息ID从RakNet定义的最后一个枚举开始  	
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
		os << "[信息]本游戏由开发者orangebird改编自IBM Ponder This" << endl;
		os << "[信息]版权所有,使用MIT协议发布" << endl;
		//os << "衍生版开发者:your name" << endl;
		os << "[信息]游戏规则" << endl;
		os << "本局游戏将进行" << maxRound << "轮" << endl;
		os << "在每一轮中,A,B以及机器人将会轮流出示一个数字(只能是0或者1)并公开" << endl;
		os << "若三人出示数字相同，则A和B都得一分，否则不得分" << endl;
		os << "你的目标是与B配合，赢下" << maxRound << "轮中至少" << winNeed << "轮，这样即可获胜" << endl;
		os << "在第一轮开始时，B将得到一个由0和1组成的序列，表示机器人在所有" << maxRound << "轮" << endl;
		os << "中将会出示的数字，但你不会得到它。B唯一向你传递消息的机会只有你在每轮中所出示的数字" << endl;
		os << "第一轮开始前，你和B有无限制的时间可以商议你们的游戏策略。" << endl;
		os << "最后，为了减小运气带来的影响，游戏将以完全相同的规则进行3局，全部获胜才算最终胜利。" << endl;
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
		cout << "[游戏]新一局游戏已经开始" << endl;
		cout << "[游戏]现在进入策略商议时间,发送\"结束商议\"即可停止商议" << endl;
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
	cout << "[游戏]第" << game.nowRound << "轮" << endl;
	if (game.nowRound != 1)
	{
		cout << "[游戏]下面三行分别表示前" << game.nowRound - 1 << "轮的情报" << endl;
		cout << "A的出数序列:" << endl;
		for (int i = 0; i < game.movementsA.size(); ++i)
			cout << game.movementsA[i] << " ";
		cout << endl;
		cout << "B的出数序列:" << endl;
		for (int i = 0; i < game.movementsB.size(); ++i)
			cout << game.movementsB[i] << " ";
		cout << endl;
		cout << "Robot的序列:" << endl;
		for (int i = 0; i < game.sequence.size(); ++i)
			cout << game.sequence[i] << " ";
		cout << endl;
		cout << "A,B获胜序列:" << endl;
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
	cout << "[信息]即将断开和玩家B的连接" << endl;
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
				cout << "[游戏]恭喜你和玩家B获胜三局,获得了最后的胜利!" << endl;
				resetAll();
			}
			else
			{
				cout << "[游戏]恭喜你和玩家B获得了本局的胜利,将开始下一局!" << endl;
				++game.bigRound;
				game.newGame(game.winNeed, game.maxRound);
			}
		}
		else {
			cout << "[游戏]很遗憾,你在本局中失利了,游戏结束!" << endl;
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
	format_messege << game.movementsA[game.nowRound - 1];  //输入信息到字符串流，用空格隔开，此处为飞行器速度高度V, h为double型      
	bsOut.Write((MessageID)MSG_A_MOVEMENT);
	bsOut.Write(format_messege.str().c_str());
	m_peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, UNASSIGNED_SYSTEM_ADDRESS, true);
}
void stopChat(RakPeerInterface* peer)
{
	if (game.gameState == GAME_STATE_CHATING)
		cout << "[游戏]对方终止了商议，你还可以向对方发送最后一句话" << endl;
	cout << "[游戏]商议时间结束，第" << game.bigRound << "局游戏正式开始!" << endl;
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
	bsIn.Read(rs);  //提取字符串           
	istringstream input_str(rs.C_String()); //存入字符串流  
	input_str >> movementB>> movementC; //提取想要的变量 
	game.movementsB.push_back(movementB);
	game.sequence.push_back(movementC);
	cout << "[游戏]玩家B出示了数字:" << movementB << endl;
	cout << "[游戏]机器人出示了数字:" << movementC << endl;
	game.gameState = GAME_STATE_MAIN_THREAD_INPUT_LOCK;
	afterGettingAllMovements();
}
void serverLostConnection(Packet* packet, RakPeerInterface* peer)
{
	cout << "[连接]到玩家B的连接已经丢失!" << endl;
}
void chatMessage(Packet* packet, RakPeerInterface* peer)
{
	RakString rs;
	BitStream bsIn(packet->data, packet->length, false);
	bsIn.IgnoreBytes(sizeof(MessageID));
	bsIn.Read(rs);  //提取字符串           
	istringstream iss(rs.C_String()); //存入字符串流  
	string msg;
	iss >> msg; //提取想要的变量 
	if (game.gameState == GameState::GAME_STATE_CHATING)
		cout << "[玩家B]" << msg << endl;
	else cout << "[玩家B]" << "..." << endl;
	if (msg == "结束商议")
		stopChat(peer);
}

void movementRequest(Packet* packet, RakPeerInterface* peer)
{
	cout << "[游戏]请输入你本轮要出的数字,一个整数,只能是1或者0" << endl;
	game.gameState = GAME_STATE_WATING_MOVMENT;
}
void connectionAccepted(Packet* packet, RakPeerInterface* peer)
{
	cout << "[连接]服务端同意了连接请求！" << std::endl;
	cout << "[信息]等待玩家B选择游戏难度" << std::endl;
}
void difficultyGot(Packet* packet, RakPeerInterface* peer)
{
	RakString rs;
	BitStream bsIn(packet->data, packet->length, false);
	bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
	bsIn.Read(rs);  //提取字符串           
	istringstream input_str(rs.C_String()); //存入字符串流  
	int n, m;
	input_str >> n>>m; //提取想要的变量 
	cout << "[游戏]玩家B选择了游戏的难度，你们需要在" << n<< "轮中赢至少" << m<< "轮" << endl;
	game.maxRound = n;
	game.winNeed = m;
	game.bigRound = 1;
	game.printGameRule(cout);
	game.newGame(m, n);
}
void cutDownConnection(Packet* packet, RakPeerInterface* peer)
{
	peer->Shutdown(300);
	cout << "[连接]已断开和服务器的连接，请关闭游戏" << std::endl;
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
	cout << "[信息]请输入服务器IP地址和端口，用空格隔开" << std::endl;
	cin >> SERVER_IP >> PORT;
	initMap();
	m_peer->Startup(1, &sd, 1);
	m_peer->Connect(SERVER_IP.c_str(), PORT, 0, 0);
	thread getMessage(getMsg);
	getMessage.detach();
	cout << "[连接]已经向服务器发送连接！" << std::endl;
	cout << "[信息]等待连接结果..." << endl;
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
				cout << "[信息]输入不合法，需要一个0~1之间的整数" << endl;
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
		//给主机发送数据  
		RakNet::BitStream bsOut;
		bsOut.Write((MessageID)MSG_CHAT);
		ostringstream format_messege;
		format_messege << msg;
		bsOut.Write(format_messege.str().c_str());
		m_peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true);
		if (msg == "结束商议"&&game.gameState == GAME_STATE_CHATING)
		{
			game.gameState = GAME_STATE_MAIN_THREAD_INPUT_LOCK;
			stopChat(m_peer);
		}
	}
	INFINITE_LOOP_END
	return 0;
}