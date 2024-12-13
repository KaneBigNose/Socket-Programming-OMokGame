#include <iostream>
#include <Winsock2.h>
#include <Ws2tcpip.h>
#include <Windows.h>
#include <conio.h>
#include <utility>
#include <string>
#include <sstream>
using namespace std;

#pragma comment(lib,"ws2_32.lib")

#define PORT 7777 // 포트번호
#define PACKET_SIZE 1024 // 패킷 사이즈
#define SERVER_IP "222.96.81.157" // ip

#define OMOKPAN_SIZE 15

int Player_turn = 1;
int OMok_winner = 0;

int OMokPan[OMOKPAN_SIZE][OMOKPAN_SIZE] = {}; // 순서대로 가로 세로

HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
DWORD mode;

void DrawOMokPan();
pair<int, int> InputXY();
bool CheckCanMove(int value, bool IsRow);
bool CheckPlayerWin(int x, int y, int player);
void gotoxy(int x, int y);
void SendGameStateToServer(SOCKET hSocket);
void ReceiveGameStateFromClient(SOCKET hSocket);

SOCKET hSocket;

int main()
{
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData); // 생성

	hSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP); // IPv4를 사용하고 연결지향형의 성격을 가진 TCP 소켓을 생성

	SOCKADDR_IN tAddr = {}; // 소켓의 구성요소를 담을 구조체 생성
	tAddr.sin_family = AF_INET;
	tAddr.sin_port = htons(PORT);
	inet_pton(AF_INET, SERVER_IP, &tAddr.sin_addr);

	connect(hSocket, (SOCKADDR*)&tAddr, sizeof(tAddr)); // 클라에서는 bind 대신 connect 사용

	GetConsoleMode(hStdin, &mode); // 현재 콘솔 모드 가져오기

	DrawOMokPan(); // 오목판 그리기

	while (true)
	{
		ReceiveGameStateFromClient(hSocket); // 서버로부터 데이터 수신
		DrawOMokPan(); // 오목판 새로고침

		if (Player_turn == 1)
		{
			gotoxy(10, 20);
			cout << "당신의 차례입니다";
		}
		else
		{
			gotoxy(10, 20);
			cout << "상대방이 수를 두고 있습니다" << endl;
		}

		pair<int, int> current_pos;

		if (Player_turn == 1)
		{
			current_pos = InputXY(); // 입력 받기
		}

		if (CheckPlayerWin(current_pos.first, current_pos.second - 2, 1))
		{
			gotoxy(19, 22);
			cout << "검은 돌 플레이어가 게임을 승리하였습니다!!!" << endl << endl;
			OMok_winner = 1;
			break;
		}
		else if (CheckPlayerWin(current_pos.first, current_pos.second - 2, 2))
		{
			gotoxy(19, 22);
			cout << "흰 돌 플레이어가 게임을 승리하였습니다!!!" << endl << endl;
			OMok_winner = 2;
			break;
		}
	}
	closesocket(hSocket);
	WSACleanup(); // 소멸
	return 0;
}

void DrawOMokPan() // 오목판 그리기
{
	system("cls");
	cout << "      202112206 이상혁" << endl;
	cout << "            오목" << endl;
	for (int i = 0; i < OMOKPAN_SIZE; i++)
	{
		for (int j = 0; j < OMOKPAN_SIZE; j++)
		{
			if (OMokPan[j][i] == 0)
			{
				cout << "·";
			}
			else if (OMokPan[j][i] == 1)
			{
				cout << "○";
			}
			else if (OMokPan[j][i] == 2)
			{
				cout << "●";
			}
		}
		cout << endl;
	}
	cout << endl << "W A S D로 위치를 선정하고 Enter로 알을 놓으세요" << endl;
	gotoxy(0, 20);
	cout << "차례 : ";
	gotoxy(0, 22);
	cout << "게임 안내 메세지 : ";
}

pair<int, int> InputXY() // 입력 받기
{
	pair<int, int> current_pos(7, 9); // 중앙 위치
	while (true)
	{
		gotoxy(current_pos.first * 2, current_pos.second);
		SetConsoleMode(hStdin, mode & ~ENABLE_ECHO_INPUT); // 에코 비활성화
		char direction_input = _getch();
		SetConsoleMode(hStdin, mode); // 에코 활성화
		if (direction_input == '\r')
		{
			if (OMokPan[current_pos.first][current_pos.second - 2] != 0) // 이미 다른 돌이 있을 경우
			{
				gotoxy(19, 22);
				cout << "해당 위치에는 돌을 배치할 수 없습니다";
				continue;
			}
			cout << "○";
			OMokPan[current_pos.first][current_pos.second - 2] = 1; // 검은돌
			gotoxy(19, 22);
			cout << "검은 돌을 배치하였습니다";
			SendGameStateToServer(hSocket);
			break;
		}
		else if (direction_input == 'w')
		{
			if (!CheckCanMove(current_pos.second - 1, true))
			{
				gotoxy(19, 22);
				cout << "해당 방향으로는 이동할 수 없습니다";
				continue;
			}
			current_pos.second--;
		}
		else if (direction_input == 'a')
		{
			if (!CheckCanMove(current_pos.first - 1, false))
			{
				gotoxy(19, 22);
				cout << "해당 방향으로는 이동할 수 없습니다";
				continue;
			}
			current_pos.first--;
		}
		else if (direction_input == 's')
		{
			if (!CheckCanMove(current_pos.second + 1, true))
			{
				gotoxy(19, 22);
				cout << "해당 방향으로는 이동할 수 없습니다";
				continue;
			}
			current_pos.second++;
		}
		else if (direction_input == 'd')
		{
			if (!CheckCanMove(current_pos.first + 1, false))
			{
				gotoxy(19, 22);
				cout << "해당 방향으로는 이동할 수 없습니다";
				continue;
			}
			current_pos.first++;
		}
		DrawOMokPan();
	}
	return current_pos;
}

bool CheckCanMove(int value, bool IsRow) // 해당 방향으로 갈 수 있는지 체크
{
	if (IsRow == true)
	{
		return value >= 2 && value <= 16 ? true : false;
	}
	return value >= 0 && value <= 14 ? true : false;
}

bool CheckPlayerWin(int x, int y, int player) // 현재 플레이어가 오목을 완성시켰는지 체크
{
	int directions[8][2] = { {-1,0},{-1,1},{0,1},{1,1},{1,0},{1,-1},{0,-1},{-1,-1} }; // 8방향

	for (int i = 0; i < 8; i++)
	{
		int count = 1; // 현재 돌을 포함하므로 1부터 시작
		for (int j = 1; j < 5; j++) // 양방향으로 확인 (5개 연속된 돌)
		{
			int nextX = x + directions[i][0] * j;
			int nextY = y + directions[i][1] * j;
			if (CheckCanMove(nextX, false) && CheckCanMove(nextY, true) && OMokPan[nextX][nextY] == player)
			{
				count++;
			}
			else
			{
				break;

			}
		}
		for (int j = 1; j < 5; j++) // 반대방향
		{
			int nextX = x - directions[i][0] * j;
			int nextY = y - directions[i][1] * j;
			if (CheckCanMove(nextX, false) && CheckCanMove(nextY, true) && OMokPan[nextX][nextY] == player)
			{
				count++;
			}
			else
			{
				break;
			}
		}
		if (count >= 5) // 5개 연속 돌이 놓였으면 오목 완성
		{
			return true;
		}
	}
	return false; // 오목 없음
}

void gotoxy(int x, int y) // 해당 위치로 위치를 옮김
{
	COORD pos = { x,y };
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}

void SendGameStateToServer(SOCKET hSocket) // 오목판 정보와 차례, 승자 정보를 서버에 전달
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

	gameState += to_string(2);  // 2번째 플레이어의 차례로 전달

	gameState += " " + to_string(OMok_winner);  // 0은 계속 진행, 1은 검은 돌 승리, 2는 흰 돌 승리

	int bytesSent = send(hSocket, gameState.c_str(), gameState.size(), 0); // 데이터 전송
}

void ReceiveGameStateFromClient(SOCKET hSocket) // 서버로부터 데이터 수신
{
	char buffer[4096];  // 최대 수신 크기 설정
	int bytesReceived = recv(hSocket, buffer, sizeof(buffer), 0); // 데이터 수신

	string gameState(buffer, bytesReceived); // 수신한 데이터 string으로 처리

	stringstream ss(gameState); // 토큰화

	string temp; // 저장하기 위해서 쓰는 임시 값

	for (int i = 0; i < OMOKPAN_SIZE; i++) // 클라이언트 오목판에 서버로부터 받아온 데이터를 저장
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
}