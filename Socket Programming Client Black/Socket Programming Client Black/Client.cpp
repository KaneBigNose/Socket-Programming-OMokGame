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

#define PORT 7777 // ��Ʈ��ȣ
#define PACKET_SIZE 1024 // ��Ŷ ������
#define SERVER_IP "222.96.81.157" // ip

#define OMOKPAN_SIZE 15

int Player_turn = 1;
int OMok_winner = 0;

int OMokPan[OMOKPAN_SIZE][OMOKPAN_SIZE] = {}; // ������� ���� ����

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
	WSAStartup(MAKEWORD(2, 2), &wsaData); // ����

	hSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP); // IPv4�� ����ϰ� ������������ ������ ���� TCP ������ ����

	SOCKADDR_IN tAddr = {}; // ������ ������Ҹ� ���� ����ü ����
	tAddr.sin_family = AF_INET;
	tAddr.sin_port = htons(PORT);
	inet_pton(AF_INET, SERVER_IP, &tAddr.sin_addr);

	connect(hSocket, (SOCKADDR*)&tAddr, sizeof(tAddr)); // Ŭ�󿡼��� bind ��� connect ���

	GetConsoleMode(hStdin, &mode); // ���� �ܼ� ��� ��������

	DrawOMokPan(); // ������ �׸���

	while (true)
	{
		ReceiveGameStateFromClient(hSocket); // �����κ��� ������ ����
		DrawOMokPan(); // ������ ���ΰ�ħ

		if (Player_turn == 1)
		{
			gotoxy(10, 20);
			cout << "����� �����Դϴ�";
		}
		else
		{
			gotoxy(10, 20);
			cout << "������ ���� �ΰ� �ֽ��ϴ�" << endl;
		}

		pair<int, int> current_pos;

		if (Player_turn == 1)
		{
			current_pos = InputXY(); // �Է� �ޱ�
		}

		if (CheckPlayerWin(current_pos.first, current_pos.second - 2, 1))
		{
			gotoxy(19, 22);
			cout << "���� �� �÷��̾ ������ �¸��Ͽ����ϴ�!!!" << endl << endl;
			OMok_winner = 1;
			break;
		}
		else if (CheckPlayerWin(current_pos.first, current_pos.second - 2, 2))
		{
			gotoxy(19, 22);
			cout << "�� �� �÷��̾ ������ �¸��Ͽ����ϴ�!!!" << endl << endl;
			OMok_winner = 2;
			break;
		}
	}
	closesocket(hSocket);
	WSACleanup(); // �Ҹ�
	return 0;
}

void DrawOMokPan() // ������ �׸���
{
	system("cls");
	cout << "      202112206 �̻���" << endl;
	cout << "            ����" << endl;
	for (int i = 0; i < OMOKPAN_SIZE; i++)
	{
		for (int j = 0; j < OMOKPAN_SIZE; j++)
		{
			if (OMokPan[j][i] == 0)
			{
				cout << "��";
			}
			else if (OMokPan[j][i] == 1)
			{
				cout << "��";
			}
			else if (OMokPan[j][i] == 2)
			{
				cout << "��";
			}
		}
		cout << endl;
	}
	cout << endl << "W A S D�� ��ġ�� �����ϰ� Enter�� ���� ��������" << endl;
	gotoxy(0, 20);
	cout << "���� : ";
	gotoxy(0, 22);
	cout << "���� �ȳ� �޼��� : ";
}

pair<int, int> InputXY() // �Է� �ޱ�
{
	pair<int, int> current_pos(7, 9); // �߾� ��ġ
	while (true)
	{
		gotoxy(current_pos.first * 2, current_pos.second);
		SetConsoleMode(hStdin, mode & ~ENABLE_ECHO_INPUT); // ���� ��Ȱ��ȭ
		char direction_input = _getch();
		SetConsoleMode(hStdin, mode); // ���� Ȱ��ȭ
		if (direction_input == '\r')
		{
			if (OMokPan[current_pos.first][current_pos.second - 2] != 0) // �̹� �ٸ� ���� ���� ���
			{
				gotoxy(19, 22);
				cout << "�ش� ��ġ���� ���� ��ġ�� �� �����ϴ�";
				continue;
			}
			cout << "��";
			OMokPan[current_pos.first][current_pos.second - 2] = 1; // ������
			gotoxy(19, 22);
			cout << "���� ���� ��ġ�Ͽ����ϴ�";
			SendGameStateToServer(hSocket);
			break;
		}
		else if (direction_input == 'w')
		{
			if (!CheckCanMove(current_pos.second - 1, true))
			{
				gotoxy(19, 22);
				cout << "�ش� �������δ� �̵��� �� �����ϴ�";
				continue;
			}
			current_pos.second--;
		}
		else if (direction_input == 'a')
		{
			if (!CheckCanMove(current_pos.first - 1, false))
			{
				gotoxy(19, 22);
				cout << "�ش� �������δ� �̵��� �� �����ϴ�";
				continue;
			}
			current_pos.first--;
		}
		else if (direction_input == 's')
		{
			if (!CheckCanMove(current_pos.second + 1, true))
			{
				gotoxy(19, 22);
				cout << "�ش� �������δ� �̵��� �� �����ϴ�";
				continue;
			}
			current_pos.second++;
		}
		else if (direction_input == 'd')
		{
			if (!CheckCanMove(current_pos.first + 1, false))
			{
				gotoxy(19, 22);
				cout << "�ش� �������δ� �̵��� �� �����ϴ�";
				continue;
			}
			current_pos.first++;
		}
		DrawOMokPan();
	}
	return current_pos;
}

bool CheckCanMove(int value, bool IsRow) // �ش� �������� �� �� �ִ��� üũ
{
	if (IsRow == true)
	{
		return value >= 2 && value <= 16 ? true : false;
	}
	return value >= 0 && value <= 14 ? true : false;
}

bool CheckPlayerWin(int x, int y, int player) // ���� �÷��̾ ������ �ϼ����״��� üũ
{
	int directions[8][2] = { {-1,0},{-1,1},{0,1},{1,1},{1,0},{1,-1},{0,-1},{-1,-1} }; // 8����

	for (int i = 0; i < 8; i++)
	{
		int count = 1; // ���� ���� �����ϹǷ� 1���� ����
		for (int j = 1; j < 5; j++) // ��������� Ȯ�� (5�� ���ӵ� ��)
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
		for (int j = 1; j < 5; j++) // �ݴ����
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
		if (count >= 5) // 5�� ���� ���� �������� ���� �ϼ�
		{
			return true;
		}
	}
	return false; // ���� ����
}

void gotoxy(int x, int y) // �ش� ��ġ�� ��ġ�� �ű�
{
	COORD pos = { x,y };
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}

void SendGameStateToServer(SOCKET hSocket) // ������ ������ ����, ���� ������ ������ ����
{
	string gameState = "";
	for (int i = 0; i < OMOKPAN_SIZE; i++)
	{
		for (int j = 0; j < OMOKPAN_SIZE; j++) // ������ �������� �������� ���� ���� string �������� ����
		{
			gameState += to_string(OMokPan[i][j]);
			gameState += " ";
		}
	}

	gameState += to_string(2);  // 2��° �÷��̾��� ���ʷ� ����

	gameState += " " + to_string(OMok_winner);  // 0�� ��� ����, 1�� ���� �� �¸�, 2�� �� �� �¸�

	int bytesSent = send(hSocket, gameState.c_str(), gameState.size(), 0); // ������ ����
}

void ReceiveGameStateFromClient(SOCKET hSocket) // �����κ��� ������ ����
{
	char buffer[4096];  // �ִ� ���� ũ�� ����
	int bytesReceived = recv(hSocket, buffer, sizeof(buffer), 0); // ������ ����

	string gameState(buffer, bytesReceived); // ������ ������ string���� ó��

	stringstream ss(gameState); // ��ūȭ

	string temp; // �����ϱ� ���ؼ� ���� �ӽ� ��

	for (int i = 0; i < OMOKPAN_SIZE; i++) // Ŭ���̾�Ʈ �����ǿ� �����κ��� �޾ƿ� �����͸� ����
	{
		for (int j = 0; j < OMOKPAN_SIZE; j++)
		{
			ss >> temp;
			OMokPan[i][j] = stoi(temp);
		}
	}

	ss >> temp;
	Player_turn = stoi(temp); // ���� ����

	ss >> temp;
	OMok_winner = stoi(temp); // ���� ����
}