#include "Network.h"



NetworkManager::NetworkManager() {

	//Initialize network
	networkThreadShouldDie = false;
	isConnectedToServer = false;
	for (int i = 0; i < MAX_CLIENTS + NUM_LOCAL; i++) {
		serverState.playerTransforms[i] = { glm::vec3(0), glm::vec3(0), 100 };
	}
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