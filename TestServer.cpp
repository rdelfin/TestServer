// TestServer.cpp : Defines the entry point for the console application.
//

#include <string>
#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iphlpapi.h>

#define DEFAULT_PORT "1337"
#define DEFAULT_BUFLEN 512

#pragma comment(lib, "Ws2_32.lib")

using namespace std;

int main(int argc, char* argv[])
{
	int status;

	// Init WSA
	WSADATA wsaData;
	if ((status = WSAStartup(MAKEWORD(2, 2), &wsaData)) != 0) {
		printf("WSAStartup failed: %d\n", status);
		return 42;
	}


	// Start Networking portion

	// Address information
	struct addrinfo *result = nullptr,
					*ptr = nullptr,
					hints;
	SOCKET listenSocket = INVALID_SOCKET;
	SOCKET clientSocket = INVALID_SOCKET;

	
	// Fill out address information
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	// Get information and store into result
	if ((status = getaddrinfo(nullptr, DEFAULT_PORT, &hints, &result)) != 0)
	{
		printf("getaddrinfo failed: %d\n", status);
		WSACleanup();
		return 1;
	}

	// Create listen socket from result data
	if ((listenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol)) == INVALID_SOCKET)
	{
		cout << "Error at socket(): " << WSAGetLastError() << endl;
		freeaddrinfo(result);
		WSACleanup();
		return 2;
	}

	// Bind socket to port and address
	if ((status = bind(listenSocket, result->ai_addr, (int)result->ai_addrlen)) == SOCKET_ERROR)
	{
		cout << "Bind failed with error " << WSAGetLastError() << endl;
		freeaddrinfo(result);
		closesocket(listenSocket);
		WSACleanup();
		return 3;
	}

	// Address information is no longer needed
	freeaddrinfo(result);

	if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR)
	{
		cout << "Listen failed with error: " << WSAGetLastError() << endl;
		closesocket(listenSocket);
		WSACleanup();
		return 4;
	}


	clientSocket = accept(listenSocket, nullptr, nullptr);
	if (clientSocket == INVALID_SOCKET) {
		printf("accept failed: %d\n", WSAGetLastError());
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}

	char recvbuf[DEFAULT_BUFLEN];
	int iResult, iSendResult;
	int recvbuflen = DEFAULT_BUFLEN;
	char message[] = "HTTP/1.1 200 OK\nServer: Wat/1.3\nContent-Type: text/html\nContent-Length: 12\nHello world!";

	do {

		iResult = recv(clientSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0) {
			printf("Bytes received: %d\n", iResult);

			// Echo the buffer back to the sender
			iSendResult = send(clientSocket, message, strlen(message), 0);
			if (iSendResult == SOCKET_ERROR) {
				printf("send failed: %d\n", WSAGetLastError());
				closesocket(clientSocket);
				WSACleanup();
				return 1;
			}

			iResult = shutdown(clientSocket, SD_SEND);
			if (iResult == SOCKET_ERROR) {
				printf("shutdown failed: %d\n", WSAGetLastError());
				closesocket(clientSocket);
				WSACleanup();
				return 1;
			}


			printf("Bytes sent: %d\n", iSendResult);

			cout << endl << "Value of bytes received: " << endl;
			cout << recvbuf << endl << endl;
			cout << "Value of bytes sent: " << endl;
			cout << message << endl;

			break;
		}
		else if (iResult == 0)
			printf("Connection closing...\n");
		else {
			printf("recv failed: %d\n", WSAGetLastError());
			closesocket(clientSocket);
			WSACleanup();
			return 1;
		}

	} while (iResult > 0);

	closesocket(clientSocket);
	WSACleanup();
	string s;
	cin >> s;
	return 0;
}

