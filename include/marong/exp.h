#pragma once

#ifndef EXP_H_
#define EXP_H_

#include <dpp/dpp.h>
#include <dpp/nlohmann/json.hpp>
#include <marong/helper.h>

#define MIN_LEVEL 0
#define MAX_LEVEL 5000
#define MIN_EXP 0
#define MAX_EXP 500

extern std::string users_address;

void addExp(nlohmann::json& users, dpp::snowflake userID, int64_t amount);
void loseExp(nlohmann::json& users, dpp::snowflake userID, int64_t amount);
void editExp(nlohmann::json& users, dpp::snowflake userID, int64_t amount);

#endif