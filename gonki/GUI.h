#pragma once
#include "imgui.h"
#include <format>

class GUI {
public:
	void render(bool ready, ENetPeer* server,float fps) {
		float lossPercent = server->packetLoss / 65535.0f * 100.0f;
		std::string pingStr = std::format("Ping: {} ms | Jitter: {}",server->roundTripTime, server->roundTripTimeVariance);

		ImGui::Begin("Settings & Info");

		if (!ready) ImGui::Text("Waiting others players | Minimum 3 players");

		ImGui::Text("%s",pingStr.c_str());
		ImGui::Text("PL: %.1f", lossPercent);
		ImGui::Text("FPS: %.0f", fps);

		ImGui::End();
	}
};
