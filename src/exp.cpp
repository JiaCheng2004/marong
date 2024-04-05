#include <marong/exp.h>

void addExp(nlohmann::json& users, dpp::snowflake userID, int64_t amount) {
    int64_t currentLevel = users[userID.str()]["status"]["level"];
    int64_t currentExp = users[userID.str()]["status"]["exp"];
    if(currentLevel < MAX_LEVEL) {
        currentExp += amount;
        while(currentExp >= MAX_EXP) {
            currentExp -= MAX_EXP;
            currentLevel++;
            if(currentLevel == MAX_LEVEL) {
                currentExp = 0;
                break;
            }
        }
        users[userID.str()]["status"]["exp"] = currentExp;
        users[userID.str()]["status"]["level"] = currentLevel;
    }
    savefile(users_address, users);
}

void loseExp(nlohmann::json& users, dpp::snowflake userID, int64_t amount) {
    int64_t currentLevel = users[userID.str()]["status"]["level"];
    int64_t currentExp = users[userID.str()]["status"]["exp"];
    if(currentLevel > MIN_LEVEL || currentExp > MIN_EXP) {
        currentExp -= amount;
        while(currentExp < 0) {
            currentLevel--;
            currentExp += MAX_EXP;
            if(currentLevel == MIN_LEVEL) {
                currentExp = currentExp < MIN_EXP ? 0 : currentExp;
                break;
            }
        }
        users[userID.str()]["status"]["exp"] = currentExp;
        users[userID.str()]["status"]["level"] = currentLevel;
    }
    savefile(users_address, users);
}

void editExp(nlohmann::json& users, dpp::snowflake userID, int64_t amount) {
    if(amount < 0)
        loseExp(users, userID, -amount);
    else
        addExp(users, userID, amount);
}

std::pair<int64_t, int64_t> getLvlExp(nlohmann::json users, dpp::snowflake userID) {
    int64_t exp = static_cast<int64_t>(users[userID.str()]["status"]["exp"]);
    int64_t lvl = static_cast<int64_t>(users[userID.str()]["status"]["level"]);
    return std::make_pair(lvl, exp);
}