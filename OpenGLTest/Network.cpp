#include "Network.h"

const PCSTR NetworkManager::DEFAULT_PORT = "5055";

NetworkManager::NetworkManager(bool isServer, std::string serverIP) {
	//Initialize network
	networkThreadShouldDie = false;
	isConnectedToServer = false;
	for (int i = 0; i < MAX_PLAYERS; i++) {
		serverState.playerTransforms[i] = { glm::vec3(0, -2, 0), glm::vec3(0), 100 };
	}

	if (isServer) {
		networkThread = std::thread(&NetworkManager::ListenForClients, this);
	}
	else {
		networkThread = std::thread(&NetworkManager::ClientLoop, this, serverIP);
	}
}


//destructor still needs to be properly implemented
NetworkManager::~NetworkManager() {
	networkThreadShouldDie = true;
	networkThread.join();
	std::cout << "NetworkManager destructor called\n";
}



void NetworkManager::SendToClients(ServerPacket packet) {

	int iResult = 0;

	for (int i = 0; i < MAX_CLIENTS; i++) {
		if (clients[i].socket != INVALID_SOCKET) {
			iResult = send(clients[i].socket, (char*)&packet, sizeof(ServerPacket), 0);
			// if send failed print reason
			if (iResult == 0) {
				std::cout << "Client #" << clients[i].id << " send failed, client shutdown connection" << std::endl;
			}
			else if (iResult == SOCKET_ERROR) {
				std::cout << "Client send failed, with error: " << WSAGetLastError() << std::endl;
			}
		}
	}
}

void NetworkManager::SendOldHazardsToClient(CLIENT & client) {
	for (int i = 0; i < hazards->size(); ++i) {
		ServerPacket packet;
		packet.type = PACKET_HAZARD_SPAWN;
		packet.hazardSpawnPacket.spawnPosition = (*hazards)[i]->getLocalPosition();
		packet.hazardSpawnPacket.size = (*hazards)[i]->getSize();
		packet.hazardSpawnPacket.fallSpeed = (*hazards)[i]->fallSpeed;
		send(client.socket, (char*)&packet, sizeof(ServerPacket), 0);
	}
}

void NetworkManager::ReceiveFromClient(CLIENT &client) {
	SendOldHazardsToClient(client);

	std::cout << "Recieve thread for Client #" << client.id << " started" << std::endl;

	int iResult = 0;

	while (client.socket != INVALID_SOCKET) {
		CLIENTPACKET packet;
		iResult = recv(client.socket, (char*)&packet, sizeof(CLIENTPACKET), 0);
		if (iResult == 0 || iResult == SOCKET_ERROR) {
			break;
		}
		// do something with client packet
		InputSourceEnum index = (InputSourceEnum)(INPUT_CLIENT1 + client.id);
		InputManager::setInput(packet, index);
	}

	// print client disconnect reason
	if (iResult > 0) {
		std::cout << "Client socket was set to INVALID_SOCKET" << std::endl;
	}
	else if (iResult == 0) {
		std::cout << "Client shutdown connection" << std::endl;
	}
	else if (iResult == SOCKET_ERROR) {
		std::cout << "Client recv failed with error " << WSAGetLastError() << std::endl;
	}

	if (client.socket != INVALID_SOCKET) {
		closesocket(client.socket);
		client.socket = INVALID_SOCKET;
	}

	std::cout << "Recieve thread for Client #" << client.id << " ended" << std::endl;
}

SOCKET NetworkManager::InitializeListenSocket() {
	std::cout << "Initializing listen socket" << std::endl;

	int iResult;

	// Initialize Winsock
	WSADATA wsaData = {};
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		std::cout << "WSAStartup failed with error: " << iResult << std::endl;
		return INVALID_SOCKET;
	}

	addrinfo hints = {};
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	// Resolve server address and port
	addrinfo *addressInfo = NULL;
	iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &addressInfo);
	if (iResult != 0) {
		std::cout << "getaddrinfo failed with error: " << iResult << std::endl;
		return INVALID_SOCKET;
	}

	SOCKET serverSocket = socket(addressInfo->ai_family, addressInfo->ai_socktype, addressInfo->ai_protocol);
	if (serverSocket == INVALID_SOCKET) {
		std::cout << "Constructing socket failed with error: " << WSAGetLastError() << std::endl;
		freeaddrinfo(addressInfo);
		return INVALID_SOCKET;
	}

	iResult = bind(serverSocket, addressInfo->ai_addr, (int)addressInfo->ai_addrlen);
	freeaddrinfo(addressInfo);
	if (iResult == SOCKET_ERROR) {
		std::cout << "Binding socket failed with error: " << WSAGetLastError() << std::endl;
		closesocket(serverSocket);
		return INVALID_SOCKET;
	}

	iResult = listen(serverSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR) {
		std::cout << "Listening on socket failed with error: " << WSAGetLastError() << std::endl;
		closesocket(serverSocket);
		return INVALID_SOCKET;
	}

	std::cout << "Listening on socket..." << std::endl;

	return serverSocket;
}

int NetworkManager::ListenForClients() {

	SOCKET listenSocket = InitializeListenSocket();
	if (listenSocket == INVALID_SOCKET) {
		WSACleanup();
		return EXIT_FAILURE;
	}

	for (int i = 0; i < MAX_CLIENTS; i++) {
		clients[i] = { i, INVALID_SOCKET };
	}

	std::thread threads[MAX_CLIENTS];

	while (!networkThreadShouldDie) {
		timeval timeout;
		timeout.tv_sec = 1;
		timeout.tv_usec = 0;
		fd_set readSet;
		FD_ZERO(&readSet);
		FD_SET(listenSocket, &readSet);
		// if there's a client trying to connect
		if (select(listenSocket, &readSet, NULL, NULL, &timeout) == 1) {
			SOCKET incomingClientSocket = accept(listenSocket, NULL, NULL);

			if (incomingClientSocket != INVALID_SOCKET) {
				// check for empty spot
				int empty_index = -1;
				for (int i = 0; i < MAX_CLIENTS; i++) {
					if (clients[i].socket == INVALID_SOCKET) {
						empty_index = i;
						break;
					}
				}

				// if empty spot found
				if (empty_index != -1) {
					clients[empty_index].socket = incomingClientSocket;
					std::cout << "Client #" << empty_index << " accepted" << std::endl;
					std::string msg = std::to_string(clients[empty_index].id);
					send(clients[empty_index].socket, msg.c_str(), strlen(msg.c_str()), 0);
					// if possible join existing thread
					if (threads[empty_index].joinable()) {
						threads[empty_index].join();
						std::cout << "Recieve thread for Client #" << empty_index << " joined" << std::endl;
					}
					//Create a thread process for that client
					threads[empty_index] = std::thread(&NetworkManager::ReceiveFromClient, this, std::ref(clients[empty_index]));
				}
				else {
					std::string msg = "Server is full";
					send(incomingClientSocket, msg.c_str(), strlen(msg.c_str()), 0);
					std::cout << "Server is full" << std::endl;
				}
			}
		}
	}

	// close listen socket
	closesocket(listenSocket);
	listenSocket = INVALID_SOCKET;

	// close client sockets and threads
	for (int i = 0; i < MAX_CLIENTS; i++) {
		if (threads[i].joinable()) {
			closesocket(clients[i].socket);
			clients[i].socket = INVALID_SOCKET;
			threads[i].join();
			std::cout << "Recieve thread for Client #" << i << " joined" << std::endl;
		}
	}

	WSACleanup();

	std::cout << "Server has terminated successfully" << std::endl;
	return EXIT_SUCCESS;
}


void NetworkManager::SendToServer() {
	Input input = InputManager::getInput(INPUT_LOCAL1);
	int iResult = send(clientSocket, (const char*)&input, sizeof(CLIENTPACKET), 0);

	// if send failed print reason
	if (iResult == 0) {
		std::cout << "Server send failed, server shutdown connection" << std::endl;
	}
	else if (iResult == SOCKET_ERROR) {
		std::cout << "Server send failed, with error: " << WSAGetLastError() << std::endl;
	}
}

void NetworkManager::ReceiveFromServer() {
	std::cout << "Recieve thread for server started" << std::endl;

	int iResult = 0;
	bool hasDisconnected = false;
	auto t1 = std::chrono::high_resolution_clock::now();
	auto t2 = t1;


	while (!networkThreadShouldDie && clientSocket != INVALID_SOCKET) {
		ServerPacket packet;
		iResult = recv(clientSocket, (char *)&packet, sizeof(ServerPacket), 0);
		if (iResult == 0 || iResult == SOCKET_ERROR) {

			if (!hasDisconnected) {
				t1 = std::chrono::high_resolution_clock::now();
				hasDisconnected = true;
			}
			else {

				t2 = std::chrono::high_resolution_clock::now();

				if (std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count() > MAX_DISCONNECT_TIME_MS) {
					isConnectedToServer = false;
					break;
				}
			}
		}
		switch (packet.type) {
		case PACKET_PLAYER_TRANSFORM: {
			serverState = packet.playerTransformPacket;
			break;
		}
		case PACKET_HAZARD_SPAWN: {
			Hazard* hazard = new Hazard(
				packet.hazardSpawnPacket.spawnPosition,
				packet.hazardSpawnPacket.size,
				packet.hazardSpawnPacket.fallSpeed
			);
			EventManager::notify(SPAWN_HAZARD, &TypeParam<Hazard*>(hazard));
			break;
		}
		}
	}

	// print server disconnect reason
	if (iResult > 0) {
		std::cout << "Server socket was set to INVALID_SOCKET" << std::endl;
	}
	else if (iResult == 0) {
		std::cout << "Server shutdown connection" << std::endl;
	}
	else if (iResult == SOCKET_ERROR) {
		std::cout << "Server recv failed with error " << WSAGetLastError() << std::endl;
	}

	std::cout << "Recieve thread for server ended" << std::endl;
}


SOCKET NetworkManager::InitializeClientSocket(std::string serverIP) {

	std::cout << "Initializing client socket" << std::endl;

	int iResult;

	// Initialize Winsock
	WSADATA wsaData = {};
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		std::cout << "WSAStartup failed with error: " << iResult << std::endl;
		return INVALID_SOCKET;
	}

	addrinfo hints = {};
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// Resolve client address and port
	addrinfo *addressInfo = NULL;
	iResult = getaddrinfo(serverIP.c_str(), DEFAULT_PORT, &hints, &addressInfo); // Resolve the server address and port  //static_cast<PCSTR>(SERVER_IP_ADDRESS)
	if (iResult != 0) {
		std::cout << "getaddrinfo failed with error: " << iResult << std::endl;
		return INVALID_SOCKET;
	}

	// Attempt to connect to an address until one succeeds
	SOCKET clientSocket = INVALID_SOCKET;
	for (addrinfo* ptr = addressInfo; ptr != NULL; ptr = ptr->ai_next) {
		// Create a SOCKET for connecting to server
		clientSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
		if (clientSocket == INVALID_SOCKET) {
			std::cout << "socket failed with error: " << WSAGetLastError() << std::endl;
			freeaddrinfo(addressInfo);
			return INVALID_SOCKET;
		}

		// Connect to server.
		iResult = connect(clientSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			closesocket(clientSocket);
			clientSocket = INVALID_SOCKET;
			continue;
		}
		break;
	}

	freeaddrinfo(addressInfo);

	if (clientSocket == INVALID_SOCKET) {
		std::cout << "Unable to connect to server!" << std::endl;
		return INVALID_SOCKET;
	}

	isConnectedToServer = true;
	std::cout << "Connected to server" << std::endl;

	return clientSocket;
}

int NetworkManager::ClientLoop(std::string SERVER_IP) {

	clientSocket = InitializeClientSocket(SERVER_IP);
	if (clientSocket == INVALID_SOCKET) {
		WSACleanup();
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
	std::cout << "Client id is: " << clientID << std::endl;

	// this currently loops forever
	ReceiveFromServer();

	return EXIT_SUCCESS;

}