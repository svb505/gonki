#include "database.h"
#include <sqlite3.h>
#include <filesystem>
#include <vector>
#include <string>
#include <iostream>

sqlite3* db;

int callbackPlayer(void* data, int argc, char** argv, char** colName) {
	PlayerContext* ctx = static_cast<PlayerContext*>(data);
	PlayerInfo p;

	p.best = std::stoi(argv[1]);
	p.worst = std::stoi(argv[2]);

	ctx->players.push_back(p);
}
bool dbIsExists() {
	return std::filesystem::exists("playerInfo.db");
}
bool openDatabase(){
	return sqlite3_open("playerInfo.db", &db) == SQLITE_OK;
}
void createDb(const std::string name) {
	std::string createQuery;
	std::string insertQuery;
	char* err;

	if (sqlite3_open(name.c_str(), &db) != SQLITE_OK) return;

	createQuery = "CREATE TABLE if NOT EXISTS player(id INTEGER PRIMARY KEY,"
		"best REAL, worst REAL)";

	insertQuery = "INSERT INTO player(id, best, worst) VALUES(0, 0, 0)";

	if (sqlite3_exec(db, createQuery.c_str(), nullptr, nullptr, &err) != SQLITE_OK) {
		sqlite3_free(err);
		return;
	}

	if (sqlite3_exec(db, insertQuery.c_str(), 0, 0, &err) != SQLITE_OK) {
		sqlite3_free(err);
		return;
	}
	sqlite3_close(db);
}
PlayerInfo getDataForPlayer() {
	PlayerContext ctx;

	char* err;

	sqlite3_exec(db, "SELECT * FROM player", callbackPlayer, &ctx, &err);

	return ctx.players[0];
}
void saveDataForPlayer(float time) {
	char* sql = nullptr;	

	PlayerInfo player = getDataForPlayer();

	if ((float)player.best == 0.0f && (float)player.worst == 0.0f) {
		sql = sqlite3_mprintf("UPDATE player SET best = %f, worst = %f WHERE id = 0;", time, time);
	}
	else if (time < player.best) sql = sqlite3_mprintf("UPDATE player SET best = %f WHERE id = 0;", time);
	else if (time > player.worst) sql = sqlite3_mprintf("UPDATE player SET worst = %f WHERE id = 0;", time);

	char* err = nullptr;

	if (sql != nullptr) sqlite3_exec(db, sql, nullptr, nullptr, &err);
	
	sqlite3_free(sql);
}





