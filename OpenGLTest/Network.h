#pragma once

#pragma comment (lib, "Ws2_32.lib") // Needed to link with Ws2_32.lib

#define DEFAULT_BUFLEN 512	//max buffer size of 512 bytes
#define DEFAULT_PORT "5055"
#define MAX_CLIENTS 2
#define NUM_LOCAL 2

#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <chrono>
#include <string>
#include <thread>  
#include <vector>
#include <glm/vec3.hpp>

#include "InputManager.h"
#include "EventManager.h"
#include "Hazard.h"


struct CLIENT {
	int id;
	SOCKET socket;
};

struct PLAYER_TRANSFORM {
	glm::vec3 position;
	glm::vec3 rotation;
	int health;
	int stunFrames;
};

struct PlayerTransformPacket {
	PLAYER_TRANSFORM playerTransforms[MAX_CLIENTS + NUM_LOCAL];
};

struct HazardSpawnPacket {
	glm::vec3 spawnPosition;
	float fallSpeed;
};

struct ServerPacket {
	ServerPacketType type;
	union {
		PlayerTransformPacket playerTransformPacket;
		HazardSpawnPacket hazardSpawnPacket;
	};
};

typedef Input CLIENTPACKET;



class NetworkManager {

public:
	// state on or from the server
	PlayerTransformPacket serverState;
	bool networkThreadShouldDie;
	bool isConnectedToServer;
	const long long MAX_DISCONNECT_TIME_MS = 1000;

	NetworkManager();
	~NetworkManager();

	//========================================== Server Functions ==========================================//
	void SendToClients(ServerPacket packet);
	void ReceiveFromClient(CLIENT &client);
	SOCKET InitializeListenSocket();
	int ListenForClients();
	//========================================== Client Functions ==========================================//

	void SendToServer();
	void ReceiveFromServer();
	SOCKET InitializeClientSocket(std::string serverIP);
	int ClientLoop(std::string SERVER_IP);

private:

	// threads for recieving client messages
	std::thread threads[MAX_CLIENTS];
	std::vector<CLIENT> clients = std::vector<CLIENT>(MAX_CLIENTS);

	// socket for communicating with server
	SOCKET clientSocket;


};
