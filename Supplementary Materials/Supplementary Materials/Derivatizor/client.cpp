// Noah Rubin

/*
	Since I was trying to sell this program, I had to ensure that it could not
	simply be copy and pasted and sent to someone else who had not bought it. This
	system prevents that by sending the machine GUID to a central server (at my house).
	The server keeps a list of all the GUIDs it has seen and whether or not it has
	entered a correct key, which can only be created and given by me. If the server
	recognizes the machine as having entered a key before, it allows the client to run.
	However if it doesn't, then it prompts the user for this key before continuing. When
	the user enters a key the server checks it against a list of available keys that have
	not been used before.

	This code is disabled in the version I am sending to the admissions office.
*/

#if 0	// disable

#include <iostream>
#include <string>

#include <WinSock2.h>
#include <WS2tcpip.h>

#include "ClientServerDefs.h"
#include "client.h"


int GetMachineGuid(char* buf, DWORD size);
SOCKET InitializeSocket();
void HandleGuidNotRecognized(SOCKET socket);
bool SendServerMessage(SOCKET socket, char* buf, int len);


bool Connect()
{
	SOCKET theSocket = InitializeSocket();
	if (theSocket == INVALID_SOCKET) {
		std::cerr << "Error in initializing socket\nClosing...\n";
		std::getchar();
		WSACleanup();
		return false;
	}
	char buf[BUFFER_LEN];
	buf[0] = CLIENT_HEADER_GUID;
	int size = GetMachineGuid(buf + 1, BUFFER_LEN - 1);
	int bytesSent = send(theSocket, buf, size + 1, 0);
	if (bytesSent == SOCKET_ERROR) {
		std::cerr << "Error in send to server: Server or internet connection might be down\n";
		std::getchar();
		closesocket(theSocket);
		WSACleanup();
		return false;
	}
	int bytesRead = recv(theSocket, buf, BUFFER_LEN, 0);
	if (bytesRead > 0) {
		switch (buf[0])
		{
		case SERVER_GUID_RECOGNIZED_RESPONSE:
			break;
		case SERVER_GUID_NOT_RECOGNIZED_RESPONSE:
			HandleGuidNotRecognized(theSocket);
			break;
		default:
			std::cout << "A network error was encountered. Try restarting\n";
			std::getchar();
			return false;
		}
	}
	else if (bytesRead == 0) {
		std::cout << "Connection closed\n";
	}
	else {
		std::cerr << "recv failed\n";
	}
	closesocket(theSocket);
	WSACleanup();
	return true;
}


bool SendServerMessage(SOCKET client, char* buf, int len)
{
	int ret = send(client, buf, len, 0);
	if (ret == SOCKET_ERROR) {
		std::cerr << "Error in send to server: Server or internet connection might be down\n";
		std::getchar();
		closesocket(client);
		return false;
	}
	return true;
}


void HandleGuidNotRecognized(SOCKET socket)
{
	char buf[BUFFER_LEN];
	std::cout << "Enter the product key.\n>";
	std::string input;
	do {
		std::getline(std::cin, input);
		if (input.size() > BUFFER_LEN - 1) {
			std::cout << "Key not recognized. Try again\n";
			continue;
		}
		buf[0] = CLIENT_HEADER_KEY;
		std::memcpy(buf + 1, input.data(), BUFFER_LEN - 1);
		if (!SendServerMessage(socket, buf, input.size() + 1)) {
			return;
		}
		int bytesRead = recv(socket, buf, BUFFER_LEN, 0);
		if (bytesRead > 0) {
			switch (buf[0])
			{
			case SERVER_KEY_RECOGNIZED_RESPONSE:
				std::cout << "Key successfuly redeemed\n";
				return;
			case SERVER_KEY_NOT_RECOGNIZED_RESPONSE:
				std::cout << "Key not recognized. Try again\n";
				break;
			default:
				std::cout << "A network error was encountered. Trying again ...\n";
				break;
			}
		}
	} while (true);
}


int GetMachineGuid(char* buf, DWORD size)
{
	HKEY key1;
	HKEY key2;
	long retVal = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE", 0, KEY_READ | KEY_WOW64_64KEY, &key1);
	retVal = RegOpenKeyEx(key1, "Microsoft", 0, KEY_READ | KEY_WOW64_64KEY, &key2);
	DWORD valType = REG_SZ;
	RegGetValue(key2, "Cryptography", "MachineGuid", RRF_RT_REG_SZ, 0, buf, &size);
	RegCloseKey(key2);
	RegCloseKey(key1);
	return size;
}


SOCKET InitializeSocket()
{
	WSAData wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	addrinfo* result;
	addrinfo hint;
	ZeroMemory(&hint, sizeof(addrinfo));
	hint.ai_family = AF_INET;
	hint.ai_socktype = SOCK_STREAM;
	hint.ai_protocol = IPPROTO_TCP;

	if (getaddrinfo(SERVER_ADDRESS, PORT_STRING, &hint, &result)) {
		std::cerr << "Error initializing socket\n";
		std::cerr << "Internet connection might be down\n";
		return INVALID_SOCKET;
	}

	SOCKET theSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (theSocket == INVALID_SOCKET) {
		std::cerr << "Error initializing socket\n";
		std::cerr << "Internet connection might be down\n";
		freeaddrinfo(result);
		return INVALID_SOCKET;
	}
	bool success = false;
	do {
		if (connect(theSocket, result->ai_addr, result->ai_addrlen) != SOCKET_ERROR) {
			success = true;
		}
		result = result->ai_next;
	} while (!success && result);
	freeaddrinfo(result);
	if (!success) {
		closesocket(theSocket);
		std::cerr << "Error connecting to server\n";
		std::cerr << "Server might be down\n";
		return INVALID_SOCKET;
	}
	return theSocket;
}

#endif