#include <iostream>
#include <Winsock2.h>
#include <Ws2tcpip.h>
#include <Windows.h>
#include <conio.h>
#include <utility>
using namespace std;

#pragma comment(lib,"ws2_32.lib")

#define PORT 7777 // 포트번호
#define PACKET_SIZE 1024 // 패킷 사이즈
#define SERVER_IP "222.97.181.134" // ip

#define OMOKPAN_SIZE 15

int OMokPan[OMOKPAN_SIZE][OMOKPAN_SIZE] = {}; // 순서대로 가로 세로

int offset[8][2] = { {-1,0},{-1,1},{0,1},{1,1},{1,0},{1,-1},{0,-1},{-1,-1} }; // 바둑알이 연결되어 있는지 체크하는 8방향

HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
DWORD mode;

void DrawOMokPan();
void InputXY();
bool CheckCanMove(int value, bool IsRow);
bool CheckPlayerWin(int x, int y, int OMokEgg);
void gotoxy(int x, int y);

int main()
{
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData); // 생성

	SOCKET hSocket;
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
		InputXY(); // 입력 받기
		if (CheckPlayerWin(0, 0, 0))
		{
			gotoxy(19, 22);
			cout << "검은 돌 플레이어가 게임을 승리하였습니다!!!";
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

void InputXY() // 입력 받기
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
}

bool CheckCanMove(int value, bool IsRow) // 해당 방향으로 갈 수 있는지 체크
{
	if (IsRow == true)
	{
		return value >= 2 && value <= 16 ? true : false;
	}
	return value >= 0 && value <= 14 ? true : false;
}

bool CheckPlayerWin(int x, int y, int OMokEgg) // 현재 플레이어가 오목을 완성시켰는지 체크
{
	if (OMokEgg == 5) // 오목 완성
	{
		return true;
	}
	if (OMokPan[x][y] == 1) // 검은돌
	{
		for (int i = 0; i < 8; i++) // 8방향 체크
		{
			if (CheckCanMove(x + offset[i][0], false) && CheckCanMove(y + offset[i][1], true))
			{
				CheckPlayerWin(x + offset[i][0], y + offset[i][1], ++OMokEgg);
			}
		}
	}
	return false; // 오목 없음
}

void gotoxy(int x, int y) // 해당 위치로 위치를 옮김
{
	COORD pos = { x,y };
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}