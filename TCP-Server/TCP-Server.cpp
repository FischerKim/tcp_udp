// TCP-Server.cpp : Defines the entry point for the console application.
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
#include <fstream>
#include <random>
#include <unordered_map>
#include <conio.h>

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
// #pragma comment (lib, "Mswsock.lib")

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


int __cdecl main(void)
{
	int sentcount = 0;
	char key;

	std::cout << "****************\n*    SERVER    *\n****************\n\n";

	char str[INET_ADDRSTRLEN];
	std::unordered_map<SOCKET, uint64_t> _connected_clients;

	//Initialize Winsock
	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != NO_ERROR) {
		std::cout << "WSAStartup failed with error: " << iResult << std::endl;
		return 1;
	}

	//Create a SOCKET for listening for incoming connections request
	SOCKET ListenSocket, ClientSocket;
	ListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (ListenSocket == INVALID_SOCKET) {
		std::cout << "Socket failed with error: " << WSAGetLastError() << std::endl;
		WSACleanup();
		return 1;
	}

	//The sockaddr_in structure specifies the address family,
	//IP address, and port for the socket that is being bound
	sockaddr_in addrServer;
	addrServer.sin_family = AF_INET;
	InetPton(AF_INET, "127.0.0.1", &addrServer.sin_addr.s_addr);
	//InetPton(AF_INET, "192.168.0.20", &addrServer.sin_addr.s_addr);
	addrServer.sin_port = htons(6666);
	memset(&(addrServer.sin_zero), '\0', 8);

	//Bind socket
	if (bind(ListenSocket, (SOCKADDR *)& addrServer, sizeof(addrServer)) == SOCKET_ERROR) {
		std::cout << "Bind failed with error: " << WSAGetLastError() << std::endl;
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	//Listen for incomin connection requests on the created socket
	if (listen(ListenSocket, 5) == SOCKET_ERROR) {
		std::cout << "Listen failed with error: " << WSAGetLastError() << std::endl;
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	// Accept a client socket
	ClientSocket = accept(ListenSocket, NULL, NULL);
	if (ClientSocket == INVALID_SOCKET) {
		std::cout << "Accept failed with error: " << WSAGetLastError() << std::endl;
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	//closesocket(ListenSocket);

	//Variables for recieve
	int iSendResult;
	char recvbuf[sizeof(CS_MARKET_DATA)];
	char sendbuf[sizeof(SC_MARKET_DATA)];
	int recvbuflen = sizeof(CS_MARKET_DATA);
	int sendbuflen = sizeof(SC_MARKET_DATA);

	// Receive until the peer shuts down the connection
	iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
	do {
		const CS_MARKET_DATA* p = reinterpret_cast<const CS_MARKET_DATA*>(recvbuf);
		_connected_clients.insert(std::pair<SOCKET, uint64_t>(ClientSocket, p->compid));

		for (auto i = _connected_clients.begin(); i != _connected_clients.end(); i++) {
			SC_MARKET_DATA pk;
			pk.compid = i->second;
			pk.Error = 0;

			pk.Sym[0].Symbol = "FOREXSYM" + std::to_string(i->second);
			pk.Sym[0].Bid = pk.Sym[0].getRandomValue(minval, maxval);
			pk.Sym[0].Ask = pk.Sym[0].getRandomValue(minval - 10, maxval - 10);
			pk.Sym[0].DailyChange = pk.Sym[0].getRandomValue(0, 1);
			
			iSendResult = send(i->first, reinterpret_cast<const char*>(&pk), sendbuflen, 0);
			if (iSendResult == SOCKET_ERROR) {
				std::cout << "Send failed with error: " << WSAGetLastError() << std::endl;
				closesocket(i->first);
				WSACleanup();
				return 1;
			}
		}

		if (_kbhit()) {
			key = _getch();
			if (key == 'q') {
				std::cout << "'q' key pressed. Stopping the loop." << std::endl;
				break;
			}
		}
		/*sentcount++;
		if (sentcount % 10000 == 0) 
			std::cout << "sent: " << std::to_string(sentcount) << std::endl;*/
	} while (true);

	iResult = shutdown(ClientSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed with error: %d\n", WSAGetLastError());
		closesocket(ClientSocket);
		WSACleanup();
		return 1;
	}

	// cleanup
	closesocket(ClientSocket);
	WSACleanup();
	return 0;
}