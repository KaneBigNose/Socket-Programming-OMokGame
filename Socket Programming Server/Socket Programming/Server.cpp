#include <iostream>
#include <WinSock2.h>
#include <Ws2tcpip.h>
#include <Windows.h>
#include <string>
#include <sstream>
using namespace std;

#pragma comment(lib, "ws2_32")

#define PORT 7777 // ��Ʈ��ȣ
#define	PACKET_SIZE 1024 // ��Ŷ ������
#define MAX_CLIENTS 2 // ���� ������ ��

#define OMOKPAN_SIZE 15

int Player_turn = 1;
int OMok_winner = 0;

int OMokPan[OMOKPAN_SIZE][OMOKPAN_SIZE] = {};

int main()
{
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData); // ����

	SOCKET hListen;
	hListen = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP); // IPv4�� ����ϰ� ������������ ������ ���� TCP ������ ����

	SOCKADDR_IN tListenAddr = {}; // ������ ������Ҹ� ���� ����ü ����
	tListenAddr.sin_family = AF_INET;
	tListenAddr.sin_port = htons(PORT);
	tListenAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	bind(hListen, (SOCKADDR*)&tListenAddr, sizeof(tListenAddr)); // ������ ���ε�
	listen(hListen, SOMAXCONN); // ���� ���

	SOCKET clientSockets[MAX_CLIENTS]; // Ŭ���̾�Ʈ���� ������ ����
	int connectedClients = 0; // ���� ����� Ŭ���̾�Ʈ�� ��

	// Ŭ���̾�Ʈ �� ���� ���� �� ������ ���� ����ü ���� �� �� �Ҵ�, Ŭ���̾�Ʈ�� ���� ��û�ϸ� �������ִ� ����
	while (connectedClients < MAX_CLIENTS)
	{
		SOCKADDR_IN tClntAddr = {};
		int iClntSize = sizeof(tClntAddr);

		if (connectedClients < 2)
		{
			cout << "���� ���� ���� �� ������ ��ٸ�����... " << connectedClients << " / 2" << endl;
		}

		SOCKET hClient = accept(hListen, (SOCKADDR*)&tClntAddr, &iClntSize); // accept �Լ��� ������ ������

		// ���� �� Ŭ���̾�Ʈ ���� ���
		if (!WSAGetLastError()) {
			cout << connectedClients + 1 << "��° ����� ���� �Ϸ�!!!" << endl;
			char clientIP[INET_ADDRSTRLEN]; // IPv4 �ּ� ���ڿ� ����
			inet_ntop(AF_INET, &(tClntAddr.sin_addr), clientIP, INET_ADDRSTRLEN);
			cout << "�ش� Client IP & Port : " << clientIP << " / " << ntohs(tClntAddr.sin_port) << endl;

			clientSockets[connectedClients] = hClient; // ���� ����� �߰�
			connectedClients++;
		}
	}

	if (connectedClients == 2)
	{
		cout << "���� ���� �Ϸ�!!!" << endl;
	}

	while (true)
	{
		if (OMok_winner != 0) // ���ڰ� �߻����� ��� ����
		{
			if (OMok_winner == 1)
			{
				cout << "���� �� �¸�!!!" << endl;
			}
			else if (OMok_winner == 2)
			{
				cout << "�� �� �¸�!!!" << endl;
			}
			break;
		}

		if (Player_turn == 1) // ���ʿ� ���� ������ ����
		{
			ReceiveGameStateFromClient(clientSockets[0]);
		}
		else if (Player_turn == 2)
		{
			ReceiveGameStateFromClient(clientSockets[1]);
		}

		for (int i = 0; i < connectedClients; i++) // ���� ���� �� Ŭ���̾�Ʈ���� ����
		{
			SendGameStateToClient(clientSockets[i]);
		}
	}

	// ���� �ݱ�
	closesocket(clientSockets[0]);
	closesocket(clientSockets[1]);
	closesocket(hListen);

	WSACleanup(); // �Ҹ�
	return 0;
}

void ReceiveGameStateFromClient(SOCKET hSocket) // Ŭ���̾�Ʈ�κ��� ������ ����
{
	char buffer[2048];  // �ִ� ���� ũ�� ����
	int bytesReceived = recv(hSocket, buffer, sizeof(buffer), 0); // ������ ����

	if (bytesReceived == SOCKET_ERROR)
	{
		cout << "���� ���� ���� ����" << endl;
		return;
	}

	string gameState(buffer, bytesReceived); // ������ ������ string���� ó��

	stringstream ss(gameState); // ��ūȭ

	string temp; // �����ϱ� ���ؼ� ���� �ӽ� ��

	for (int i = 0; i < OMOKPAN_SIZE; i++) // ���� �����ǿ� �޾ƿ� �����͸� ����
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

	if (Player_turn == 1) // ���� ���� ǥ��
	{
		cout << "���� : ���� ��" << endl;
	}
	else if (Player_turn == 2)
	{
		cout << "���� : �� ��" << endl;
	}
}

void SendGameStateToClient(SOCKET hSocket) // ������ ������ ����, ���� ������ Ŭ���̾�Ʈ�鿡�� ����
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

	gameState += to_string(Player_turn);  // �÷��̾��� ���ʷ� ���� ����

	gameState += " " + to_string(OMok_winner);  // 0�� ��� ����, 1�� ���� �� �¸�, 2�� �� �� �¸�

	int bytesSent = send(hSocket, gameState.c_str(), gameState.size(), 0); // ������ ����
}