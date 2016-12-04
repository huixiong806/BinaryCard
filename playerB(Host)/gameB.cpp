// playerB(Host).cpp : 定义控制台应用程序的入口点。

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
		os << "[信息]本游戏由开发者orangebird改编自IBM Ponder This" << endl;
		os << "[信息]版权所有,使用MIT协议发布" << endl;
		//os << "衍生版开发者:your name" << endl;
		os << "[信息]游戏规则" << endl;
		os << "本局游戏将进行"<< maxRound <<"轮" << endl;
		os << "在每一轮中,A,B以及机器人将会轮流出示一个数字(只能是0或者1)并公开" << endl;
		os << "若三人出示数字相同，则A和B都得一分，否则不得分" << endl;
		os << "你的目标是与A配合，赢下"<< maxRound << "轮中至少"<< winNeed <<"轮，这样即可获胜" << endl;
		os << "在第一轮开始时，你将得到一个由0和1组成的序列，表示机器人在所有" << maxRound << "轮"<<endl;
		os << "中将会出示的数字。但A不会得到它，并且唯一向A传递消息的机会只有你在每轮中所出示的数字"<<endl;
		os << "第一轮开始前，你和A有无限制的时间可以商议你们的游戏策略。" << endl;
		os << "最后，为了减小运气带来的影响，游戏将以完全相同的规则进行3局，全部获胜才算最终胜利。" << endl;
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
		cout << "[游戏]新一局游戏已经开始" << endl;
		cout << "[游戏]现在进入策略商议时间,发送\"结束商议\"即可停止商议" << endl;
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
	cout << "[信息]请输入游戏难度,范围为0~3之间的整数" << endl;
	int difficulty;
	while (cin >> difficulty)
	{
		if (cin.fail() || difficulty < 0 || difficulty>3)
		{
			cout << "[信息]输入不合法，需要一个0~3之间的整数" << endl;
			cin.clear();
			cin.sync();
		}
		else break;
	}
	int m[4] = { 4,6,9,13 };
	int n[4] = { 7,9,13,18 };
	cout<< "[游戏]你选择了难度"<<difficulty<<",你和玩家A需要在" << n[difficulty] << "轮中赢至少" << m[difficulty] << "轮" << endl;
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
	cout << "[游戏]第"<<game.nowRound <<"轮" << endl;
	if (game.nowRound != 1)
	{
		cout << "[游戏]下面三行分别表示前"<< game.nowRound-1<<"轮的情报，为了方便记忆，将再次提示机器人出数字的序列" << endl;
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
	BitStream bsOut;
	bsOut.Write((MessageID)MSG_MOVEMENT_REQUEST);
	peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, UNASSIGNED_SYSTEM_ADDRESS, true);
	cout << "[游戏]等待玩家A出示数字" << endl;
	game.gameState = GAME_STATE_WATING_OTHER;
}
void resetAll()
{
	game = BinaryCardGameB();
	BitStream bsOut;
	bsOut.Write((MessageID)MSG_CUT_DOWN_CONNECTION);
	m_peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, UNASSIGNED_SYSTEM_ADDRESS, true);
	cout << "[信息]玩家A即将断开和本机的连接" << endl;
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
				cout << "[游戏]恭喜你和玩家A获胜三局,获得了最后的胜利!" << endl;
				resetAll();
			}
			else
			{
				cout << "[游戏]恭喜你和玩家A获得了本局的胜利,将开始下一局!" << endl;
				++game.bigRound;
				game.newGame(game.winNeed , game.maxRound);
			}
		}else{
			cout << "[游戏]很遗憾,你在本局中失利了,游戏结束!" << endl;
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
	format_messege <<game.movementsB[game.nowRound-1]<<" "<<game.sequence[game.nowRound-1];  //输入信息到字符串流，用空格隔开，此处为飞行器速度高度V, h为double型      
	bsOut.Write((MessageID)MSG_BC_MOVEMENT);
	bsOut.Write(format_messege.str().c_str());
	m_peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, UNASSIGNED_SYSTEM_ADDRESS, true);
}
void stopChat(RakPeerInterface* peer)
{
	if (game.gameState == GAME_STATE_CHATING)
		cout << "[游戏]对方终止了商议，你还可以向对方发送最后一句话" << endl;
	if (game.gameState == GAME_STATE_CHATING)
		game.gameState = GAME_STATE_MAIN_THREAD_INPUT_LOCK;
	cout << "[游戏]商议时间结束，第"<<game.bigRound<<"局游戏正式开始!" << endl;
	cout << "[游戏]你得到了一个01序列:";
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
	bsIn.Read(rs);  //提取字符串           
	istringstream input_str(rs.C_String()); //存入字符串流  
	input_str >> movement; //提取想要的变量 
	game.movementsA.push_back(movement);
	cout << "[游戏]玩家A出示了数字:" << movement << endl;
	cout << "[游戏]请输入你本轮要出的数字,一个整数,只能是1或者0" << endl;
	game.gameState = GAME_STATE_WATING_MOVMENT;
}
void clientLostConnection(Packet* packet, RakPeerInterface* peer)
{
	cout << "[连接]到玩家A的连接已经丢失!" << endl;
}
void newIncomingConnection(Packet* packet, RakPeerInterface* peer)
{
	cout << "[连接]玩家A成功连接到了本机!" << endl;
	startNewGame();
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
	if(game.gameState ==GameState::GAME_STATE_CHATING)
		cout << "[玩家A]" << msg << endl;
	else cout << "[玩家A]" << "..." << endl;
	if (msg == "结束商议")
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
	cout << "[信息]请输入本机IP地址和端口，用空格隔开" << std::endl;
	cin >> SERVER_IP >> PORT;
	cout << "[信息]正在启动服务器" << std::endl;
	//启动服务器  
	m_peer->Startup(1, &SocketDescriptor(PORT, SERVER_IP.c_str()), 1);
	//设置最大链接数  
	m_peer->SetMaximumIncomingConnections(1);
	cout << "[信息]服务器启动成功！" << std::endl;
	cout << "[信息]等待玩家A连接到本机" << endl;
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
				cout << "[信息]输入不合法，需要一个0~1之间的整数" << endl;
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
		if (msg == "结束商议"&&game.gameState == GAME_STATE_CHATING)
		{
			game.gameState = GAME_STATE_MAIN_THREAD_INPUT_LOCK;
			stopChat(m_peer);
		}
	}
	INFINITE_LOOP_END
	return 0;
}