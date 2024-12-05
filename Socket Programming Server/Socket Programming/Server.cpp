#include <iostream>
#include <WinSock2.h>
#include <Ws2tcpip.h>
#include <Windows.h>
using namespace std;

#pragma comment(lib, "ws2_32")

#define PORT 7777 // 포트번호
#define	PACKET_SIZE 1024 // 패킷 사이즈
#define MAX_CLIENTS 2 // 게임 참가자 수

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

		SOCKET hClient = accept(hListen, (SOCKADDR*)&tClntAddr, &iClntSize); // accept 함수로 접속을 승인함

		// 연결 시 클라이언트 정보 출력
		if (!WSAGetLastError()) {
			cout << connectedClients + 1 << "번째 사용자 연결 완료!!!" << endl;
			char clientIP[INET_ADDRSTRLEN]; // IPv4 주소 문자열 버퍼
			inet_ntop(AF_INET, &(tClntAddr.sin_addr), clientIP, INET_ADDRSTRLEN);
			cout << connectedClients + 1 << "번째 Client IP : " << clientIP << endl;
			cout << connectedClients + 1 << "번째 Port : " << ntohs(tClntAddr.sin_port) << endl;

			clientSockets[connectedClients] = hClient; // 현재 사용자 추가
			connectedClients++;
		}
	}

	// 소켓 닫기
	closesocket(clientSockets[0]);
	closesocket(clientSockets[1]);
	closesocket(hListen);

	WSACleanup(); // 소멸
	return 0;
}