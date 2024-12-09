#include <iostream>
#include <WinSock2.h>
#include <Ws2tcpip.h>
#include <Windows.h>
#include <string>
#include <sstream>
using namespace std;

#pragma comment(lib, "ws2_32")

#define PORT 7777 // 포트번호
#define	PACKET_SIZE 1024 // 패킷 사이즈
#define MAX_CLIENTS 2 // 게임 참가자 수

#define OMOKPAN_SIZE 15

int Player_turn = 1;
int OMok_winner = 0;

int OMokPan[OMOKPAN_SIZE][OMOKPAN_SIZE] = {};

int main()
{
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData); // 생성

	SOCKET hListen;
	hListen = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP); // IPv4를 사용하고 연결지향형의 성격을 가진 TCP 소켓을 생성

	SOCKADDR_IN tListenAddr = {}; // 소켓의 구성요소를 담을 구조체 생성
	tListenAddr.sin_family = AF_INET;
	tListenAddr.sin_port = htons(PORT);
	tListenAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	bind(hListen, (SOCKADDR*)&tListenAddr, sizeof(tListenAddr)); // 소켓을 바인드
	listen(hListen, SOMAXCONN); // 연결 대기

	SOCKET clientSockets[MAX_CLIENTS]; // 클라이언트들의 소켓을 저장
	int connectedClients = 0; // 현재 연결된 클라이언트의 수

	// 클라이언트 측 소켓 생성 및 정보를 담을 구조체 생성 및 값 할당, 클라이언트가 접속 요청하면 승인해주는 역할
	while (connectedClients < MAX_CLIENTS)
	{
		SOCKADDR_IN tClntAddr = {};
		int iClntSize = sizeof(tClntAddr);

		if (connectedClients < 2)
		{
			cout << "오목 선수 입장 목 빠지게 기다리는중... " << connectedClients << " / 2" << endl;
		}

		SOCKET hClient = accept(hListen, (SOCKADDR*)&tClntAddr, &iClntSize); // accept 함수로 접속을 승인함

		// 연결 시 클라이언트 정보 출력
		if (!WSAGetLastError()) {
			cout << connectedClients + 1 << "번째 사용자 연결 완료!!!" << endl;
			char clientIP[INET_ADDRSTRLEN]; // IPv4 주소 문자열 버퍼
			inet_ntop(AF_INET, &(tClntAddr.sin_addr), clientIP, INET_ADDRSTRLEN);
			cout << "해당 Client IP & Port : " << clientIP << " / " << ntohs(tClntAddr.sin_port) << endl;

			clientSockets[connectedClients] = hClient; // 현재 사용자 추가
			connectedClients++;
		}
	}

	if (connectedClients == 2)
	{
		cout << "선수 입장 완료!!!" << endl;
	}

	while (true)
	{
		if (OMok_winner != 0) // 승자가 발생했을 경우 종료
		{
			if (OMok_winner == 1)
			{
				cout << "검은 돌 승리!!!" << endl;
			}
			else if (OMok_winner == 2)
			{
				cout << "흰 돌 승리!!!" << endl;
			}
			break;
		}

		if (Player_turn == 1) // 차례에 따라 정보를 받음
		{
			ReceiveGameStateFromClient(clientSockets[0]);
		}
		else if (Player_turn == 2)
		{
			ReceiveGameStateFromClient(clientSockets[1]);
		}

		for (int i = 0; i < connectedClients; i++) // 게임 상태 각 클라이언트에게 전달
		{
			SendGameStateToClient(clientSockets[i]);
		}
	}

	// 소켓 닫기
	closesocket(clientSockets[0]);
	closesocket(clientSockets[1]);
	closesocket(hListen);

	WSACleanup(); // 소멸
	return 0;
}

void ReceiveGameStateFromClient(SOCKET hSocket) // 클라이언트로부터 데이터 수신
{
	char buffer[2048];  // 최대 수신 크기 설정
	int bytesReceived = recv(hSocket, buffer, sizeof(buffer), 0); // 데이터 수신

	if (bytesReceived == SOCKET_ERROR)
	{
		cout << "게임 상태 수신 실패" << endl;
		return;
	}

	string gameState(buffer, bytesReceived); // 수신한 데이터 string으로 처리

	stringstream ss(gameState); // 토큰화

	string temp; // 저장하기 위해서 쓰는 임시 값

	for (int i = 0; i < OMOKPAN_SIZE; i++) // 서버 오목판에 받아온 데이터를 저장
	{
		for (int j = 0; j < OMOKPAN_SIZE; j++)
		{
			ss >> temp;
			OMokPan[i][j] = stoi(temp);
		}
	}

	ss >> temp;
	Player_turn = stoi(temp); // 차례 정보

	ss >> temp;
	OMok_winner = stoi(temp); // 승자 정보

	if (Player_turn == 1) // 차례 정보 표기
	{
		cout << "차례 : 검은 돌" << endl;
	}
	else if (Player_turn == 2)
	{
		cout << "차례 : 흰 돌" << endl;
	}
}

void SendGameStateToClient(SOCKET hSocket) // 오목판 정보와 차례, 승자 정보를 클라이언트들에게 전달
{
	string gameState = "";

	for (int i = 0; i < OMOKPAN_SIZE; i++)
	{
		for (int j = 0; j < OMOKPAN_SIZE; j++) // 공백을 기준으로 오목판의 현재 값을 string 형식으로 전달
		{
			gameState += to_string(OMokPan[i][j]);
			gameState += " ";
		}
	}

	gameState += to_string(Player_turn);  // 플레이어의 차례로 정보 전달

	gameState += " " + to_string(OMok_winner);  // 0은 계속 진행, 1은 검은 돌 승리, 2는 흰 돌 승리

	int bytesSent = send(hSocket, gameState.c_str(), gameState.size(), 0); // 데이터 전송
}