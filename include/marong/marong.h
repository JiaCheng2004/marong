#pragma once

#ifndef MARONG_H_
#define MARONG_H_

#include <dpp/dpp.h>
#include <dpp/nlohmann/json.hpp>

// Place any forward declarations here

#include <marong/helper.h>
#include <marong/user.h>
#include <sstream>
#include <ctime>
#include <time.h>
#include <queue>
#include <map>
#include <iostream>
#include <curl/curl.h>

using json = nlohmann::json;
using GPTFunctionCaller = std::function<nlohmann::json(const std::string, const std::string)>;
using GPTResponseDealer = std::function<std::pair<std::string, int>(nlohmann::json)>;

#endif /* MARONG_H_ */
