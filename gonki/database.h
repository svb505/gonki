#pragma once
#include <sqlite3.h>
#include <vector>
#include <string>

struct PlayerInfo {
	float best, worst;
};
struct PlayerContext {
	std::vector<PlayerInfo> players;
};
extern sqlite3* db;

bool openDatabase();
int callbackPlayer(void* data, int argc, char** argv, char** colName);
void createDb(const std::string name = "playerInfo.db");
bool dbIsExists();
void saveDataForPlayer(float time);
PlayerInfo getDataForPlayer();
