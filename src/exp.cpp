#include <marong/exp.h>

void addExp(nlohmann::json& users, dpp::snowflake userID, int64_t amount) {
    int64_t currentExp = users[userID]["exp"];
    int64_t currentLevel = users[userID]["level"];

    currentExp += amount;

    if (currentExp >= MAX_EXP) {
        currentLevel += 1;
        if (currentLevel > MAX_LEVEL) {
            currentLevel = MAX_LEVEL;
        }
        currentExp = 0;
    }

    users[userID]["exp"] = currentExp;
    users[userID]["level"] = currentLevel;

    savefile(users_address, users);
}

void loseExp(nlohmann::json& users, dpp::snowflake userID, int64_t amount) {
    int64_t currentExp = users[userID]["exp"];
    int64_t currentLevel = users[userID]["level"];

    currentExp -= amount;

    if (currentExp < MIN_EXP) {
        currentLevel -= 1;
        if (currentLevel < MIN_LEVEL) {
            currentLevel = MIN_LEVEL;
        }
        currentExp = MAX_EXP;
    }

    users[userID]["exp"] = currentExp;
    users[userID]["level"] = currentLevel;

    savefile(users_address, users);
}

void editExp(nlohmann::json& users, dpp::snowflake userID, int64_t amount) {
    int64_t currentLevel = amount / MAX_EXP;
    if (currentLevel > MAX_LEVEL) {
        currentLevel = MAX_LEVEL;
    }
    else if (currentLevel < MIN_LEVEL) {
        currentLevel = MIN_LEVEL;
    }

    int64_t currentExp = amount % MAX_EXP;

    users[userID]["exp"] = currentExp;
    users[userID]["level"] = currentLevel;

    savefile(users_address, users);
}

std::pair<int64_t, int64_t> getLvlExp(nlohmann::json users, dpp::snowflake userID) {
    int64_t exp = static_cast<int64_t>(users[userID.str()]["status"]["exp"]);
    int64_t lvl = static_cast<int64_t>(users[userID.str()]["status"]["level"]);
    return std::make_pair(lvl, exp);
}