#pragma once

#ifndef HELPER_H_
#define HELPER_H_

#include <dpp/dpp.h>
#include <dpp/nlohmann/json.hpp>
#include <sstream>
#include <iostream>
#include <curl/curl.h>
#include <random>

bool has_role(dpp::guild_member M, dpp::snowflake role_id);

bool has_channel(dpp::guild G, dpp::snowflake channel_id);

dpp::channel newTextChannel(std::string channelName, dpp::snowflake guildId, dpp::snowflake parentId, uint16_t position);

dpp::channel newCategory(std::string categoryName, dpp::snowflake guildId, uint16_t position);

int getRandomIndex(int length);

void insertionSort(std::vector<std::pair<std::string, int>>& arr);

void printUserVoiceMap(const std::map<std::string, dpp::channel>& user_voice_map);

void printChannelMap(const std::map<std::string, std::vector<std::pair<std::string, int>>>& channel_map);

#endif
