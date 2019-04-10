#pragma once

#pragma comment (lib, "Ws2_32.lib") // Needed to link with Ws2_32.lib

#define MAX_PLAYERS 4
#define MAX_CLIENTS 2

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
	PLAYER_TRANSFORM playerTransforms[MAX_PLAYERS];
};

struct HazardSpawnPacket {
	glm::vec3 spawnPosition;
	glm::vec3 size;
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
	static const long long MAX_DISCONNECT_TIME_MS = 1000;

	NetworkManager(bool isServer, std::string serverIP);
	~NetworkManager();

	//========================================== Server Functions ==========================================//
	void SendToClients(ServerPacket packet);
	void SendOldHazardsToClient(CLIENT &client);
	void ReceiveFromClient(CLIENT &client);
	SOCKET InitializeListenSocket();
	int ListenForClients();
	//========================================== Client Functions ==========================================//

	void SendToServer();
	void ReceiveFromServer();
	SOCKET InitializeClientSocket(std::string serverIP);
	int ClientLoop(std::string SERVER_IP);
	std::vector<Hazard*>* hazards;
private:
	std::thread networkThread;

	static const size_t DEFAULT_BUFLEN = 512; //max buffer size of 512 bytes
	static const PCSTR DEFAULT_PORT;


	// threads for recieving client messages
	std::vector<CLIENT> clients = std::vector<CLIENT>(MAX_CLIENTS);

	// socket for communicating with server
	SOCKET clientSocket;
};
