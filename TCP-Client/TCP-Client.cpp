// TCP-Client.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#undef UNICODE

#define WIN32_LEAN_AND_MEAN

//#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <random>
#include <conio.h>

// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
//#pragma comment (lib, "Mswsock.lib")
//#pragma comment (lib, "AdvApi32.lib")

#define DEFAULT_BUFLEN 256

const int minval = 100;
const int maxval = 1000000;

struct sSymbol
{
	std::string Symbol;
	float Bid = 0;
	float Ask = 0;
	float DailyChange = 0;

	sSymbol() : Bid(0), Ask(0), DailyChange(0) {}
	sSymbol(const std::string& _Symbol, float _Bid, float _Ask, float _DailyChange)
		: Symbol(_Symbol), Bid(_Bid), Ask(_Ask), DailyChange(_DailyChange) {}

	float getRandomValue(float min, float max) {
		std::random_device rd;
		std::default_random_engine engine(rd());
		std::uniform_real_distribution<float> distribution(min, max);
		return distribution(engine);
	}
};

struct SC_MARKET_DATA
{
	uint64_t compid = 0;
	int16_t Error = 0;
	sSymbol Sym[1] = {};
};

struct CS_MARKET_DATA
{
	uint64_t compid = 0;
};


int __cdecl main(int argc, char **argv)
{
	char key;
	bool viewdata = false;
	int receivedcount = 0;
	std::cout << "****************\n*    CLIENT    *\n****************\n\n";

	//Initialize Winsock
	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != NO_ERROR) {
		std::cout << "WSAStartup Failed with error: " << iResult << std::endl;
		return 1;
	}

	SOCKET ConnectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (ConnectSocket == INVALID_SOCKET) {
		std::cout << "Error at socket(): " << WSAGetLastError() << std::endl;
		WSACleanup();
		return 1;
	}

	// The sockaddr_in structure specifies the address family,
	// IP address, and port for the socket that is being bound.
	sockaddr_in addrServer;
	addrServer.sin_family = AF_INET;
	InetPton(AF_INET, "127.0.0.1", &addrServer.sin_addr.s_addr);
	//InetPton(AF_INET, "192.168.0.20", &addrServer.sin_addr.s_addr);
	addrServer.sin_port = htons(6666);
	memset(&(addrServer.sin_zero), '\0', 8);

	// Connect to server.
	std::cout << "Connecting..." << std::endl;
	iResult = connect(ConnectSocket, (SOCKADDR*)&addrServer, sizeof(addrServer));
	if (iResult == SOCKET_ERROR) {
		closesocket(ConnectSocket);
		std::cout << "Unable to connect to server: " << WSAGetLastError() << std::endl;
		WSACleanup();
		return 1;
	}

	char recvbuf[sizeof(SC_MARKET_DATA)];
	char sendbuf[sizeof(CS_MARKET_DATA)];
	int recvbuflen = sizeof(SC_MARKET_DATA);
	int sendbuflen = sizeof(CS_MARKET_DATA);

	CS_MARKET_DATA  p;	
	p.compid = 1;

	iResult = send(ConnectSocket, reinterpret_cast<const char*>(&p), sizeof(CS_MARKET_DATA), 0);
	if (iResult == SOCKET_ERROR) {
		std::cout << "Send failed with error: " << WSAGetLastError() << std::endl;
		closesocket(ConnectSocket);
		WSACleanup();
		return 1;
	}

	
	// Receive until the peer closes the connection
	do {

		if (_kbhit()) {
			key = _getch();
			if (key == 'v') {
				viewdata = !viewdata;
			}
		}

		iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
		const SC_MARKET_DATA* p = reinterpret_cast<const SC_MARKET_DATA*>(recvbuf);
		if (iResult > 0 && viewdata) {
			std::string result = "id " + std::to_string(p->compid) + " bid " + std::to_string(p->Sym[0].Bid) +
				" ask " + std::to_string(p->Sym[0].Ask) + " change " + std::to_string(p->Sym[0].DailyChange);
			std::cout << result << std::endl;
		}
		else if (iResult == 0)
			std::cout << "Connection closed\n" << std::endl;

		/*receivedcount++;
		if (receivedcount % 100 == 0)
			std::cout << "received: " << std::to_string(receivedcount) << std::endl;*/


	} while (iResult > 0);


	// shutdown the connection since no more data will be sent
	iResult = shutdown(ConnectSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed with error: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return 1;
	}


	// cleanup
	closesocket(ConnectSocket);
	WSACleanup();

	return 0;
}