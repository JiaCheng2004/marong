#include <marong/exp.h>

void addExp(nlohmann::json& users, dpp::snowflake userID, int64_t amount) {
    int64_t exp = static_cast<int64_t>(users[userID.str()]["status"]["exp"]);
    int64_t lvl = static_cast<int64_t>(users[userID.str()]["status"]["level"]);

    // Add the amount of experience
    exp += amount;

    // Calculate the level increase
    int64_t levelIncrease = (exp + 499) / 500; // Add 499 to ensure correct rounding

    // Update level and remaining exp
    lvl += levelIncrease;
    exp = (exp + 499) % 500; // Add 499 to ensure correct rounding

    // Cap level at MAX_LEVEL
    lvl = std::min(lvl, static_cast<int64_t>(MAX_LEVEL));

    users[userID.str()]["status"]["exp"] = exp;
    users[userID.str()]["status"]["level"] = lvl;
    savefile(users_address, users);
}

void loseExp(nlohmann::json& users, dpp::snowflake userID, int64_t amount) {
    int64_t exp = static_cast<int64_t>(users[userID.str()]["status"]["exp"]);
    int64_t lvl = static_cast<int64_t>(users[userID.str()]["status"]["level"]);

    // Subtract the amount of experience
    exp -= amount;

    // Calculate the level decrease
    int64_t levelDecrease = exp / 500; // Integer division, truncates towards zero

    // Update level and remaining exp
    lvl -= levelDecrease;
    exp %= 500; // Take the remainder after level decrease

    // Ensure level doesn't go below zero
    lvl = std::max(lvl, int64_t(0));

    users[userID.str()]["status"]["exp"] = exp;
    users[userID.str()]["status"]["level"] = lvl;
    savefile(users_address, users);
}


void editExp(nlohmann::json& users, dpp::snowflake userID, int64_t amount) {
    int64_t exp = static_cast<int64_t>(users[userID.str()]["status"]["exp"]);
    int64_t lvl = static_cast<int64_t>(users[userID.str()]["status"]["level"]);

    exp += amount;

    // Check if the level should increase
    while (exp >= MAX_EXP) {
        lvl += 1;
        exp -= MAX_EXP;
        if (lvl >= MAX_LEVEL) {
            exp = MIN_EXP; // Reset exp if max level reached
            break;
        }
    }

    // Check if the level should decrease
    while (exp < MIN_EXP && lvl > MIN_LEVEL) {
        lvl -= 1;
        exp += MAX_EXP;
    }

    // Ensure level doesn't go below zero
    if (lvl < MIN_LEVEL)
        lvl = MIN_LEVEL;

    users[userID.str()]["status"]["exp"] = exp;
    users[userID.str()]["status"]["level"] = lvl;
    savefile(users_address, users);
}