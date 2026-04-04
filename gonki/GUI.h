#pragma once
#include "imgui.h"
#include <format>

class GUI {
public:
	void render(bool ready, ENetPeer* server,float fps, CarState& myCar,int totLaps,RaceResult& rank) {
		int myPlace = rank.places[myCar.id];
		std::string hud = std::format("Lap: {} / {}\nPlace: {} / {}", myCar.lap + 1, totLaps, myPlace, rank.allCars.size());
		std::string f_speed = std::format("Speed: {:.1f}", myCar.speed);

		float lossPercent = server->packetLoss / 65535.0f * 100.0f;
		std::string pingStr = std::format("Ping: {} ms | Jitter: {}",server->roundTripTime, server->roundTripTimeVariance);

		ImGui::Begin("Settings & Info");

		if (!ready) ImGui::Text("Waiting others players | Minimum 3 players");

		ImGui::Text("%s",pingStr.c_str());
		ImGui::Text("PL: %.1f", lossPercent);
		ImGui::Dummy({ 0,10 });
		ImGui::Text("FPS: %.0f", fps);
		ImGui::Dummy({ 0,10 });
		ImGui::Text("%s", f_speed.c_str());
		ImGui::Text("%s", hud.c_str());
		ImGui::End();
	}
};
