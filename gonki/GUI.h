#pragma once
#include "imgui.h"
#include <format>
#include <vector>
#include <imgui_stdlib.h>
#include "variables.h"

class GUI {
private:
	std::string chatMessage = "";
public:
	void setup(GLFWwindow* window) {
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;

		ImGui_ImplGlfw_InitForOpenGL(window, true);
		ImGui_ImplOpenGL3_Init("#version 330");

		ImGui::StyleColorsDark();
	}
	void render(bool ready, ENetPeer* server,float fps, CarState& myCar,int totLaps,RaceResult& rank,
		ChatContext& chatContext,int carCount) {
		
		int myPlace = rank.places[myCar.id];
		float lossPercent = server->packetLoss / 65535.0f * 100.0f;

		std::string hud = std::format("Lap: {} / {}\nPlace: {} / {}", myCar.lap, totLaps, myPlace, rank.allCars.size());
		std::string f_speed = std::format("Speed: {:.1f}", myCar.speed);
		std::string pingStr = std::format("Ping: {} ms | Jitter: {}",server->roundTripTime, server->roundTripTimeVariance);

		ImGui::Begin("Settings & Info");

		if (!ready) ImGui::Text("Waiting others players | Minimum 3 players");

		ImGui::InputText("Enter your name", &userName);
		ImGui::Dummy({ 0,10 });
		ImGui::InputText("Text", &chatMessage);

		if (ImGui::Button("Send")) {
			chatContext.myMes = chatMessage;

			sendChat = true;

			chatMessage.clear();
		}

		ImGui::BeginChild("ListRegion", ImVec2(0, 200), true);

		for (auto& m : chatContext.allMessages) ImGui::Text("%s", m.c_str());

		ImGui::EndChild();

		ImGui::Text("%s",pingStr.c_str());
		ImGui::Text("PL: %.1f", lossPercent);
		ImGui::Dummy({ 0,10 });

		ImGui::Text("Cars count: %d", carCount);
		ImGui::Dummy({ 0,10 });

		ImGui::Text("FPS: %.0f", fps);
		ImGui::Dummy({ 0,10 });

		ImGui::Text("%s", f_speed.c_str());
		ImGui::Text("%s", hud.c_str());

		ImGui::End();
	}
};
