#pragma once

#ifndef HELPER_H_
#define HELPER_H_

#include <dpp/dpp.h>
#include <dpp/nlohmann/json.hpp>
#include <sstream>
#include <iostream>
#include <curl/curl.h>
#include <random>

#define MESSAGE_LENGTH_LIMIT 2000

bool has_role(dpp::guild_member M, dpp::snowflake role_id);

bool has_channel(dpp::guild G, dpp::snowflake channel_id);

dpp::channel newTextChannel(std::string channelName, dpp::snowflake guildId, dpp::snowflake parentId, uint16_t position);

dpp::channel newCategory(std::string categoryName, dpp::snowflake guildId, uint16_t position);

size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* s);

nlohmann::json curlUrlToJsonResponse(const std::string& urlString);

void replaceAll(std::string& str, const std::string& arg1, const std::string& arg2);;

std::string getStringBeforeSlash(const std::string& str);

void attachTextfile(std::string& prompt, const std::string& fileName, const std::string& fileContent);

std::string getFileName(const std::string& str);

std::string getFileType(const std::string &fileName);

size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp);

dpp::message standardMessageFileWrapper(dpp::snowflake channelID, std::string filecontent);

nlohmann::json QingYunKe_API(std::string prompt);

nlohmann::json GPT4_API(std::string prompt, std::string OPENAI_API_KEY, std::string model);

nlohmann::json Gemini_API(const std::string prompt, const std::string KEY);

nlohmann::json Claude3_API(const std::string prompt, const std::string KEY);

int getRandomIndex(int length);

void insertionSort(std::vector<std::pair<std::string, int>>& arr);

void printUserVoiceMap(const std::map<std::string, dpp::channel>& user_voice_map);

void printChannelMap(const std::map<std::string, std::vector<std::pair<std::string, int>>>& channel_map);

#endif
