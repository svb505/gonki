#pragma once



void SendChat(ENetPeer* peer,std::string& myMes) {
    ChatPacket p{};
    p.type = PacketType::Chat;

    strncpy_s(p.msg, sizeof(p.msg), myMes.c_str(), _TRUNCATE);

    ENetPacket* packet = enet_packet_create(&p, sizeof(p), ENET_PACKET_FLAG_RELIABLE);
    enet_peer_send(peer, 1, packet);

    myMes.clear();
}
void SendState(ENetPeer* peer,CarState& myCar) {
    ClientStatePacket packet{};

    packet.type = PacketType::ClientState;
    packet.state = myCar;

    ENetPacket* p = enet_packet_create(&packet, sizeof(packet), ENET_PACKET_FLAG_UNSEQUENCED);
    enet_peer_send(peer, 0, p);
}