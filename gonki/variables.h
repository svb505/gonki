#pragma once
#include <vector>
#include <string>

struct ChatContext {
	std::vector<std::string>& allMessages;
	std::string& myMes;
};

std::string userName = "";

const int TOTAL_LAPS = 3;
const int MIN_PLAYERS = 1;
bool readyToRace = false;

bool sendChat = false;

int currentCarsCount = 0;