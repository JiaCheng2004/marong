#ifndef EXP_H_
#define EXP_H_

#include <dpp/dpp.h>
#include <dpp/nlohmann/json.hpp>

void addExp(nlohmann::json& user, int amount);

void loseExp(nlohmann::json& user, int amount);

#endif