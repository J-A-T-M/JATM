#pragma once

#pragma comment (lib, "Ws2_32.lib") // Needed to link with Ws2_32.lib

#define SERVER_IP_ADDRESS "127.0.0.1"
#define DEFAULT_BUFLEN 512	//max buffer size oof 512 bytes
#define DEFAULT_PORT "5055"
#define MASTER_CLIENT_ID 0
#define MAX_CLIENTS 3

#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>
#include <thread>  
#include <vector>

#include "InputManager.h"

struct CLIENT {
	int id;
	SOCKET socket;
};

struct PLAYER {
	float x;
	float z;
	float velocityX;
	float velocityZ;
	int health;
};

struct SERVERPACKET {
	PLAYER players[MAX_CLIENTS];
};

typedef Input CLIENTPACKET;

// state on or from the server
SERVERPACKET serverState;

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------
//
//    Server code
//
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------

// threads for recieving client messages
std::thread threads[MAX_CLIENTS];
bool threadsShouldDie = false;
std::vector<CLIENT> clients(MAX_CLIENTS);

void sendToClients() {
	int iResult = 0;
	for (int i = 0; i < MAX_CLIENTS; i++) {
		if (clients[i].socket != INVALID_SOCKET) {
			iResult = send(clients[i].socket, (char*)&serverState, sizeof(SERVERPACKET), 0);
		}
	}
}

void recieveFromClient(CLIENT &client) {
	int iResult = 0;
	CLIENTPACKET packet;

	while (client.socket != INVALID_SOCKET && !threadsShouldDie) {
		iResult = recv(client.socket, (char*)&packet, sizeof(CLIENTPACKET), 0);
		if (iResult <= 0) {
			break;
		}
		// handle input from client
		InputSourceEnum index = (InputSourceEnum)(INPUT_CLIENT1 + client.id);
		InputManager::setInput(packet, index);
	}
	
	// if client disconnected print reason
	if (!threadsShouldDie) {
		switch (iResult) {
			case 0:
				std::cout << "Client closed connection" << std::endl;
				break;
			case SOCKET_ERROR:
				std::cout << "Client SOCKET_ERROR" << std::endl;
				break;
			default:
				std::cout << "Client recv failed with error: " << WSAGetLastError() << std::endl;
				break;
		}
	}

	closesocket(client.socket);
	client.socket = INVALID_SOCKET;
	std::cout << "Client #" << client.id << " has disconnected" << std::endl;
}

SOCKET initializeListenSocket() {
	int iResult;

	std::cout << "Initializing up server..." << std::endl;

	WSADATA wsaData;
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData); // Initialize Winsock
	if (iResult != 0) {
		std::cout << "WSAStartup failed with error: " << iResult << std::endl;
		return INVALID_SOCKET;
	}

	addrinfo hints = {};
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	addrinfo *result = NULL;
	iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result); // Resolve the server address and port
	if (iResult != 0) {
		std::cout << "getaddrinfo failed with error: " << iResult << std::endl;
		WSACleanup();
		return INVALID_SOCKET;
	}

	SOCKET serverSocket = INVALID_SOCKET;
	serverSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);	// Create a SOCKET for connecting to server
	if (serverSocket == INVALID_SOCKET) {
		std::cout << "socket failed with error: " << WSAGetLastError() << std::endl;
		freeaddrinfo(result);
		WSACleanup();
		return INVALID_SOCKET;
	}

	//Setup socket options
	//setsockopt(ListenSocket, SOL_SOCKET, SO_REUSEADDR, &OPTION_VALUE, sizeof(int)); //Make it possible to re-bind to a port that was used within the last 2 minutes
	//setsockopt(ListenSocket, IPPROTO_TCP, TCP_NODELAY, &OPTION_VALUE, sizeof(int)); //Used for interactive programs

	iResult = bind(serverSocket, result->ai_addr, (int)result->ai_addrlen); // Setup the TCP listening socket
	if (iResult == SOCKET_ERROR) {
		std::cout << "bind failed with error: " << WSAGetLastError() << std::endl;
		freeaddrinfo(result);
		closesocket(serverSocket);
		WSACleanup();
		return INVALID_SOCKET;
	}
	freeaddrinfo(result);

	iResult = listen(serverSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR) {
		std::cout << "listen failed with error: " << WSAGetLastError() << std::endl;
		closesocket(serverSocket);
		WSACleanup();
		return INVALID_SOCKET;
	}

	std::cout << "Listening..." << std::endl;

	return serverSocket;
}

int listenForClients() {
	SOCKET ListenSocket = initializeListenSocket();
	if (ListenSocket == INVALID_SOCKET) {
		return -1;
	}

	for (int i = 0; i < MAX_CLIENTS; i++) {
		clients[i] = { -1, INVALID_SOCKET };
	}

	for (int i = 0; i < MAX_CLIENTS; i++) {
		serverState.players[i] = { 0.0f, 0.0f, 0.0f, 0.0f, 100 };
	}

	while (true) {
		SOCKET incomingClientSocket = INVALID_SOCKET;
		incomingClientSocket = accept(ListenSocket, NULL, NULL);

		if (incomingClientSocket != INVALID_SOCKET) {
			// check for empty spot
			int temp_id = -1;
			for (int i = 0; i < MAX_CLIENTS; i++) {
				if (clients[i].socket == INVALID_SOCKET) {
					temp_id = i;
					break;
				}
			}

			// if empty spot found
			if (temp_id != -1) {
				clients[temp_id].socket = incomingClientSocket;
				clients[temp_id].id = temp_id;
				std::cout << "Client #" << temp_id << " accepted" << std::endl;
				std::string msg = std::to_string(clients[temp_id].id);
				send(clients[temp_id].socket, msg.c_str(), strlen(msg.c_str()), 0);
				//Create a thread process for that client
				threads[temp_id] = std::thread(recieveFromClient, std::ref(clients[temp_id]));
			} else {
				std::string msg = "Server is full";
				send(incomingClientSocket, msg.c_str(), strlen(msg.c_str()), 0);
				std::cout << "Server is full" << std::endl;
			}
		}
	}

	//Close listen socket
	closesocket(ListenSocket); 

	threadsShouldDie = true;
	for (int i = 0; i < MAX_CLIENTS; i++) {	
		// close client sockets
		closesocket(clients[i].socket);
		// join threads
		threads[i].join();
	}

	//Clean up Winsock
	WSACleanup();
	std::cout << "Server has terminated successfully" << std::endl;

	return 0;
}



// --------------------------------------------------------------------------------------------------------------------------------------------------------------------
//
//    Client code
//
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------

// socket for communicating with server
SOCKET clientSocket;

void sendToServer() {
	Input input = InputManager::getInput(INPUT_LOCAL1);
	int iResult = send(clientSocket, (const char*)&input, sizeof(CLIENTPACKET), 0);

	if (iResult <= 0) {
		std::cout << "Server send failed with error: " << WSAGetLastError() << std::endl;
		if (iResult == SOCKET_ERROR) {
			std::cout << "SOCKET_ERROR" << std::endl;
		}
	}
}

void recieveFromServer() {
	int iResult = 0;
	SERVERPACKET packet;

	while (clientSocket != INVALID_SOCKET) {
		iResult = recv(clientSocket, (char *)&packet, sizeof(SERVERPACKET), 0);
		if (iResult == SOCKET_ERROR) {
			std::cout << "Server recv failed with error: " << WSAGetLastError() << std::endl;
			break;
		}
		serverState = packet;
	}

	if (WSAGetLastError() == WSAECONNRESET) {
		std::cout << "Connection with server has been reset" << std::endl;
	}
}

SOCKET initializeClientSocket() {
	int iResult;

	WSADATA wsaData;
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData); // Initialize Winsock
	if (iResult != 0) {
		std::cout << "WSAStartup failed with error: " << iResult << std::endl;
		return INVALID_SOCKET;
	}

	addrinfo hints = {};
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	std::cout << "Connecting..." << std::endl;

	addrinfo *result = NULL;
	iResult = getaddrinfo(static_cast<PCSTR>(SERVER_IP_ADDRESS), DEFAULT_PORT, &hints, &result); // Resolve the server address and port
	if (iResult != 0) {
		std::cout << "getaddrinfo failed with error: " << iResult << std::endl;
		WSACleanup();
		return INVALID_SOCKET;
	}

	// Attempt to connect to an address until one succeeds
	SOCKET ClientSocket = INVALID_SOCKET;
	for (addrinfo* ptr = result; ptr != NULL; ptr = ptr->ai_next) {
		// Create a SOCKET for connecting to server
		ClientSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
		if (ClientSocket == INVALID_SOCKET) {
			std::cout << "socket failed with error: " << WSAGetLastError() << std::endl;
			WSACleanup();
			return INVALID_SOCKET;
		}

		// Connect to server.
		iResult = connect(ClientSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			closesocket(ClientSocket);
			ClientSocket = INVALID_SOCKET;
			continue;
		}
		break;
	}

	freeaddrinfo(result);

	if (ClientSocket == INVALID_SOCKET) {
		std::cout << "Unable to connect to server!" << std::endl;
		WSACleanup();
		return INVALID_SOCKET;
	}

	std::cout << "Successfully Connected" << std::endl;

	return ClientSocket;
}

int ClientLoop() {
	clientSocket = initializeClientSocket();
	if (clientSocket == INVALID_SOCKET) {
		return EXIT_FAILURE;
	}

	char msgBuffer[DEFAULT_BUFLEN];
	int iResult = recv(clientSocket, msgBuffer, DEFAULT_BUFLEN, 0);
	if (iResult == SOCKET_ERROR) {
		return EXIT_FAILURE;
	}

	std::string string = msgBuffer;
	string.resize(iResult);
	if (string == "Server is full") {
		std::cout << "Server is full" << std::endl;
		return EXIT_FAILURE;
	}

	int clientID = atoi(msgBuffer);
	std::cout << "Server to this client - your client id is: " << clientID << std::endl;

	// this currently loops forever
	recieveFromServer();

	return EXIT_SUCCESS;
}