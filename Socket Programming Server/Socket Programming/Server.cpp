#include <iostream>
#include <WinSock2.h>
#include <Ws2tcpip.h>
#include <Windows.h>
using namespace std;

#pragma comment(lib, "ws2_32")

#define PORT 7777 // ��Ʈ��ȣ
#define	PACKET_SIZE 1024 // ��Ŷ ������
#define MAX_CLIENTS 2 // ���� ������ ��

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

		SOCKET hClient = accept(hListen, (SOCKADDR*)&tClntAddr, &iClntSize); // accept �Լ��� ������ ������

		// ���� �� Ŭ���̾�Ʈ ���� ���
		if (!WSAGetLastError()) {
			cout << connectedClients + 1 << "��° ����� ���� �Ϸ�!!!" << endl;
			char clientIP[INET_ADDRSTRLEN]; // IPv4 �ּ� ���ڿ� ����
			inet_ntop(AF_INET, &(tClntAddr.sin_addr), clientIP, INET_ADDRSTRLEN);
			cout << connectedClients + 1 << "��° Client IP : " << clientIP << endl;
			cout << connectedClients + 1 << "��° Port : " << ntohs(tClntAddr.sin_port) << endl;

			clientSockets[connectedClients] = hClient; // ���� ����� �߰�
			connectedClients++;
		}
	}

	// ���� �ݱ�
	closesocket(clientSockets[0]);
	closesocket(clientSockets[1]);
	closesocket(hListen);

	WSACleanup(); // �Ҹ�
	return 0;
}