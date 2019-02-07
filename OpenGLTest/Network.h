#pragma once
#undef UNICODE

#define SERVER_IP_ADDRESS "127.0.0.1"
#define DEFAULT_BUFLEN 512	//max buffer size oof 512 bytes
#define DEFAULT_PORT "5055"
#define MASTER_CLIENT_ID 0
#define MAX_CLIENTS 3

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <thread>  
#include <vector>
#include <algorithm>


// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")	 //server
#pragma comment (lib, "Mswsock.lib") //client
#pragma comment (lib, "AdvApi32.lib")//client


//NOTE - id = 0 is reserved for the player hosting the game, aka master client.
typedef struct client
{
	int id;
	SOCKET socket;

}CLIENT;


typedef struct player
{
	float x, z;
	float velocityX, velocityZ;
	int health;
	int id;

}PLAYER;


class PlayerPacket
{
public:
	PLAYER players[MAX_CLIENTS];
	PlayerPacket() {}
	~PlayerPacket() {}
};

std::vector<CLIENT> clients(MAX_CLIENTS);
std::thread my_thread[MAX_CLIENTS];
PlayerPacket serverPlayersData;

int num_clients = 0;
int temp_id = -1;


PlayerPacket initialializePlayerPacket(float x, float z, float velX, float velZ, int health) {

	PlayerPacket playerPacket;

	for (int i = 0;i < MAX_CLIENTS;i++) {
		playerPacket.players[i] = { x, z, velX, velZ, health, i }; //PLAYER player{0,0,0,100,true};
	}

	return playerPacket;
}

void printPlayerPacket(PlayerPacket packet) {

	for (int i = 0;i < MAX_CLIENTS;i++) printf("Player #%d - location(%d,%d), hp(%d)\n", packet.players[i].id, packet.players[i].x, packet.players[i].z, packet.players[i].health);
}

void print(char string[], int length) {
	for (int i = 0; i < length; i++) std::cout << string[i];
	std::cout << std::endl;
}




// --------------------------------------------------------------------------------------------------------------------------------------------------------------------
//
//    Server code
//
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------


void ProcessInput(char recvbuf[], int id) {
	//Handle reciving input logic here.
	switch (recvbuf[0]) {
	case 'w':
		serverPlayersData.players[id].z--;
		break;
	case 's':
		serverPlayersData.players[id].z++;
		break;
	case 'a':
		serverPlayersData.players[id].x--;
		break;
	case 'd':
		serverPlayersData.players[id].x++;
		break;
	case 'q':
		serverPlayersData.players[id].health++;
		break;
	case 'e':
		//serverPlayersData.players[new_client.id].y--;
		printf("e received\n");
		break;
	default:
		break;
	}
}

bool BroadCastAll() {

	char playerPacketBuffer[sizeof(PlayerPacket)];

	memcpy(playerPacketBuffer, (char*)&serverPlayersData, sizeof(PlayerPacket));

	int iResult = 0;

	for (int i = 0; i < MAX_CLIENTS;i++) {

		if (clients[i].socket != INVALID_SOCKET) 
			iResult = send(clients[i].socket, playerPacketBuffer, sizeof(playerPacketBuffer), 0);
	}

	return true;
}

int BroadCastAllLoop() {

	while (1) {
		BroadCastAll();
		Sleep(200);
	}
	return 0;
}


int ProcessClient(CLIENT &new_client, std::vector<CLIENT> &client_array, std::thread &thread) {

	char recvbuf[DEFAULT_BUFLEN];
	char playerPacketBuffer[sizeof(PlayerPacket)];
	int recvbuflen = DEFAULT_BUFLEN, iResult = 0, iSendResult = 0;

	while (1) {

		if (new_client.socket != 0) {

			iResult = recv(new_client.socket, recvbuf, recvbuflen, 0);

			if (iResult > 0) {

				printf("Client #%d: ", new_client.id);
				print(recvbuf, iResult);

				//Handle reciving input logic here.
				ProcessInput(recvbuf, new_client.id);

				memcpy(playerPacketBuffer, (char*)&serverPlayersData, sizeof(PlayerPacket));

				//Broadcast the message received by this client to all other clients connected to this server
				for (int i = 0; i < MAX_CLIENTS;i++) {
					if (clients[i].socket != INVALID_SOCKET) //if (new_client.id != i) - this line makes it so we don't send to the client sending the data.
						iResult = send(clients[i].socket, playerPacketBuffer, sizeof(playerPacketBuffer), 0);	//iResult = send(clients[i].socket, recvbuf, (int)strlen(recvbuf), 0);
				}

			}
			else {	//this client disconnected 

				switch (iResult) {
				case 0:
					printf("Client closing connection\n");
					break;
				case SOCKET_ERROR:
					printf("Client qutting - SOCKET_ERROR\n");
					break;
				default:
					printf("recv failed with error :%d\n", WSAGetLastError());
				}

				printf("Client %d has disconnected\n", new_client.id);

				closesocket(new_client.socket);
				closesocket(client_array[new_client.id].socket);
				client_array[new_client.id].socket = INVALID_SOCKET;

				//TODO: Broadcast the disconnection message to the other clients - for (int i = 0; i < MAX_CLIENTS; i++)

				break;
			}

		}


	}	//end while

	thread.detach();
	return 0;
}



SOCKET StartServer() {

	WSADATA wsaData;
	SOCKET serverSocket = INVALID_SOCKET;
	struct addrinfo *result = NULL;
	struct addrinfo hints;
	int iResult;	//used for error checking

	printf("Initializing up server...\n");

	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData); // Initialize Winsock
	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
		return INVALID_SOCKET;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result); // Resolve the server address and port
	if (iResult != 0) {
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		return INVALID_SOCKET;
	}

	serverSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);	// Create a SOCKET for connecting to server
	if (serverSocket == INVALID_SOCKET) {
		printf("socket failed with error: %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return INVALID_SOCKET;
	}

	//Setup socket options
	//setsockopt(ListenSocket, SOL_SOCKET, SO_REUSEADDR, &OPTION_VALUE, sizeof(int)); //Make it possible to re-bind to a port that was used within the last 2 minutes
	//setsockopt(ListenSocket, IPPROTO_TCP, TCP_NODELAY, &OPTION_VALUE, sizeof(int)); //Used for interactive programs

	iResult = bind(serverSocket, result->ai_addr, (int)result->ai_addrlen); // Setup the TCP listening socket
	if (iResult == SOCKET_ERROR) {
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(serverSocket);
		WSACleanup();
		return INVALID_SOCKET;
	}

	freeaddrinfo(result);

	iResult = listen(serverSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR) {
		printf("listen failed with error: %d\n", WSAGetLastError());
		closesocket(serverSocket);
		WSACleanup();
		return INVALID_SOCKET;
	}

	printf("Listening...\n");

	return serverSocket;
}


void ServerLoop() {

	serverPlayersData = initialializePlayerPacket(0.0f, 0.0f, 0.0f, 0.0f, 0);

	SOCKET ListenSocket = StartServer();

	if (ListenSocket == INVALID_SOCKET) {
		return;
	}

	for (int i = 0;i < MAX_CLIENTS;i++) clients[i] = { -1, INVALID_SOCKET }; //Initialize the client list

	std::thread broadcast_thread(BroadCastAllLoop);
	broadcast_thread.detach();

	while (1) {

		//printf("server - waiting for a connection...\n");
		SOCKET incomingClientSocket = INVALID_SOCKET;
		incomingClientSocket = accept(ListenSocket, NULL, NULL);

		if (incomingClientSocket == INVALID_SOCKET) {
			printf("accept failed with error: %d\n", WSAGetLastError()); //closesocket(ListenSocket); WSACleanup();
			continue;
		}

		num_clients = -1; //reset number of clients
		temp_id = -1;	  //create temp id for the next client, with -1 being invalid

		for (int i = 1;i < MAX_CLIENTS;i++) {	//reserve id = 0  for the server (master client)

			if (clients[i].socket == INVALID_SOCKET && temp_id == -1) { //checks to see if there's an empty spot in the client array
				clients[i].socket = incomingClientSocket;
				clients[i].id = i;
				temp_id = i;
			}

			if (clients[i].socket != INVALID_SOCKET) num_clients++;

		}

		if (temp_id != -1) { //if we found space for the client

			printf("Client #%d accepted\n", clients[temp_id].id);
			std::string msg = std::to_string(clients[temp_id].id);
			send(clients[temp_id].socket, msg.c_str(), strlen(msg.c_str()), 0);

			//Create a thread process for that client
			my_thread[temp_id] = std::thread(ProcessClient, std::ref(clients[temp_id]), std::ref(clients), std::ref(my_thread[temp_id]));
		}
		else {
			std::string msg = "Server is full";
			send(incomingClientSocket, msg.c_str(), strlen(msg.c_str()), 0);
			std::cout << msg << std::endl;
		}
	}

	closesocket(ListenSocket); //Close listen socket

	for (int i = 0;i < MAX_CLIENTS;i++) {	//close client sockets
		my_thread[i].detach();
		closesocket(clients[i].socket);
	}

	WSACleanup();	//Clean up Winsock
	printf("Server has terminated successfully\n\n");

	return; //0
}




// --------------------------------------------------------------------------------------------------------------------------------------------------------------------
//
//    Client code
//
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------

CLIENT my_client;
SOCKET ClientSocket;

int ProcessServerSocket(CLIENT &new_client) {

	char recvbuf[DEFAULT_BUFLEN], playerPacketBuffer[sizeof(PlayerPacket)];
	int recvbuflen = DEFAULT_BUFLEN, iResult = 0;
	PlayerPacket playerPacket;

	while (1) {	//keep on receiving message from the server

		//memset(recvbuf, 0, DEFAULT_BUFLEN); if we wish to remove characters from previous messages

		if (new_client.socket != 0) {	//check if socket is valid

			iResult = recv(new_client.socket, playerPacketBuffer, sizeof(playerPacketBuffer), 0);	//iResult = recv(new_client.socket, recvbuf, recvbuflen, 0);

			if (iResult != SOCKET_ERROR) {

				memcpy((char *)&playerPacket, playerPacketBuffer, sizeof(PlayerPacket));

				serverPlayersData = playerPacket;//memcpy(playerPacketBuffer, (char*)&serverPlayersData, sizeof(PlayerPacket));

				//printPlayerPacket(playerPacket);
				//printf("\n");

			}
			else {
				printf("recv() failed: %d\n", WSAGetLastError());
				break;
			}
		}
	}

	if (WSAGetLastError() == WSAECONNRESET)	printf("The server has disconnected\n");

	return 0;
}


SOCKET InitializeClient() {

	WSADATA wsaData;
	SOCKET ClientSocket = INVALID_SOCKET;
	struct addrinfo *result = NULL, *ptr = NULL, hints;
	int iResult;

	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData); // Initialize Winsock
	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
		return 1;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	printf("Connecting...\n");

	iResult = getaddrinfo(static_cast<LPCTSTR>(SERVER_IP_ADDRESS), DEFAULT_PORT, &hints, &result); // Resolve the server address and port
	if (iResult != 0) {
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		return 1;
	}

	// Attempt to connect to an address until one succeeds
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

		// Create a SOCKET for connecting to server
		ClientSocket = socket(ptr->ai_family, ptr->ai_socktype,
			ptr->ai_protocol);
		if (ClientSocket == INVALID_SOCKET) {
			printf("socket failed with error: %ld\n", WSAGetLastError());
			WSACleanup();
			system("pause");
			return 1;
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

		printf("Unable to connect to server!\n");
		WSACleanup();
		system("pause");
		return 1;
	}

	printf("Successfully Connected\n");

	return ClientSocket;
}


void ClientSendMessage(char msgBuffer[]) {

	int iResult = send(ClientSocket, msgBuffer, (int)strlen(msgBuffer), 0);

	if (iResult <= 0)
	{
		printf("Perhaps a packet was dropped, send() failed: %d\n", WSAGetLastError());

		if (iResult == SOCKET_ERROR) {
			//ShutDownClient(ClientSocket);
			printf("SOCKET_ERROR\n");
		}
		
	}
}

void ClientLoop() {

	ClientSocket = InitializeClient();

	char msgBuffer[DEFAULT_BUFLEN];
	int iResult = 0;
	int size = recv(ClientSocket, msgBuffer, DEFAULT_BUFLEN, 0);

	my_client = { -1, ClientSocket };

	if (size != SOCKET_ERROR) {

		std::string message = msgBuffer;
		message.resize(size);


		if (message != "Server is full") {
			int clientID = atoi(msgBuffer);	//this is the clientt id
			printf("Server to this client - your client id is: %d\n", clientID);

			std::thread client_thread(ProcessServerSocket, std::ref(my_client));	//client receives data here
			client_thread.detach();
		}
		else {
			std::cout << message.c_str() << std::endl;
		}
	}

	return; //0
}

int ShutDownClient(SOCKET ConnectSocket) {

	printf("Exiting game...\n");

	int iResult = shutdown(ConnectSocket, SD_SEND);

	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed with error: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return -1;
	}

	// cleanup
	closesocket(ConnectSocket);
	WSACleanup();

	return 0;
}
