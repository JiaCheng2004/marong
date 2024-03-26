#pragma once

#ifndef GPT_H_
#define GPT_H_

#include <dpp/dpp.h>
#include <dpp/nlohmann/json.hpp>
#include <sstream>
#include <iostream>
#include <curl/curl.h>

#define MESSAGE_LENGTH_LIMIT 2000

using json = nlohmann::json;

size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp);

std::string replaceAll(const std::string& str, const std::string& arg1, const std::string& arg2);

std::string getStringBeforeSlash(const std::string& str);

void attachTextfile(std::string& prompt, const std::string& fileName, const std::string& fileContent);

std::string getFileName(const std::string& str);

std::string getFileType(const std::string &fileName);

nlohmann::json QingYunKe_API(const std::string prompt, const std::string KEY);

nlohmann::json GPT4_API(const std::string prompt, const std::string KEY);

nlohmann::json GPT4_Turbo_API(const std::string prompt, const std::string KEY);

nlohmann::json Gemini_API(const std::string prompt, const std::string KEY);

nlohmann::json Claude3_API(const std::string prompt, const std::string KEY);

std::pair<std::string, int> QingYunKe_API_Response(nlohmann::json response);

std::pair<std::string, int> Claude3_API_Response(nlohmann::json response);

std::pair<std::string, int> Gemini_API_Response(nlohmann::json response);

std::pair<std::string, int> GPT4_API_Response(nlohmann::json response);

std::pair<std::string, int> GPT4_Turbo_API_Response(nlohmann::json response);

dpp::message standardMessageFileWrapper(dpp::snowflake channelID, std::string filecontent);

std::string FileErrorMessage(const nlohmann::json& settings, const std::string& nickname, const std::string& filetype);

#endif