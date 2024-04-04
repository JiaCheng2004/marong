#pragma once

#ifndef MARONG_H_
#define MARONG_H_

#include <dpp/dpp.h>
#include <dpp/nlohmann/json.hpp>

// Place any forward declarations here

#include <marong/helper.h>
#include <marong/music.h>
#include <marong/user.h>
#include <marong/gpt.h>
#include <marong/exp.h>
#include <sstream>
#include <ctime>
#include <time.h>
#include <queue>
#include <map>
#include <iostream>
#include <curl/curl.h>
// #include <ogg/ogg.h>
// #include <oggz/oggz.h>
// #include <opus/opusfile.h>

using json = nlohmann::json;
using GPTFunctionCaller = std::function<nlohmann::json(const std::string, const std::string)>;
using GPTResponseDealer = std::function<std::pair<std::string, int>(nlohmann::json)>;

std::string config_address = "../config/config.json";
std::string settings_address = "../config/settings.json";
std::string users_address = "../config/users.json";

#endif /* MARONG_H_ */
