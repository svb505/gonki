#include "database.h"
#include <sqlite3.h>
#include <filesystem>
#include <vector>
#include <string>

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

	if (sqlite3_open("playerInfo.db", &db) != SQLITE_OK) return ctx.players[0]; 
	else {
		char* err;

		sqlite3_exec(db, "SELECT * FROM player", callbackPlayer, &ctx, &err);
		sqlite3_close(db);

		return ctx.players[0];
	}
}
void saveDataForPlayer(float time) {
	if (sqlite3_open("playerInfo.db", &db) != SQLITE_OK) return;

	char* sql = nullptr;

	PlayerInfo player = getDataForPlayer();

	if (player.best == 0.0f && player.worst == 0.0f) {
		sql = sqlite3_mprintf("UPDATE player SET best = %f, worst = %f WHERE id = 0;", time, time);
	}
	else if (time < player.best) sql = sqlite3_mprintf("UPDATE player SET best = %f WHERE id = 0;", time);
	else if (time > player.worst) sql = sqlite3_mprintf("UPDATE player SET worst = %f WHERE id = 0;", time);

	char* err = nullptr;

	sqlite3_exec(db, sql, nullptr, nullptr, &err);
	sqlite3_free(sql);

	sqlite3_close(db);
}





