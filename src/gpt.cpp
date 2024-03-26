#include <marong/gpt.h>

size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    userp->append((char*)contents, size * nmemb);
    return size * nmemb;
}

std::string replaceAll(const std::string& str, const std::string& arg1, const std::string& arg2) {
    std::string result = str; 
    size_t pos = 0; 
    while ((pos = result.find(arg1, pos)) != std::string::npos) {
        result.replace(pos, arg1.length(), arg2);
        pos += arg2.length();
    }
    return result;
}

std::string getStringBeforeSlash(const std::string& str) {
    size_t pos = str.find('/');
    if (pos != std::string::npos) {
        return str.substr(0, pos);
    }
    return str;
}

void attachTextfile(std::string& prompt, const std::string& fileName, const std::string& fileContent) {
    if (fileName == "message.txt") {
        prompt += "\n\n" + fileContent;
    } else {
        prompt += "\n\nThis is " + fileName + ":\n```" + getFileType(fileName) + fileContent + "```\n";
    }
}

std::string getFileName(const std::string& str) {
    std::string identifier = "filename=\"";
    std::size_t idx = str.find(identifier);

    if (idx != std::string::npos) {
        std::size_t start = idx + identifier.length();
        std::size_t end = str.find("\"", start) - start;
        std::string filename = str.substr(start, end);
        return filename;
    }

    return "";
}

std::string getFileType(const std::string& fileName) {
    size_t found = fileName.find_last_of(".");
    if (found == std::string::npos)
        return "\n";
    else
        return fileName.substr(found + 1) + "\n";
}

dpp::message standardMessageFileWrapper(dpp::snowflake channelID, std::string filecontent) {
    dpp::message newMessage = dpp::message();
    if (strlen(filecontent.c_str()) > MESSAGE_LENGTH_LIMIT ){
        newMessage.set_content("My response was more than 2000 characters, this was my response:");
        newMessage.set_channel_id(channelID);
        newMessage.add_file("message.txt", filecontent);
    } else {
        newMessage.set_content(filecontent);
    }
    return newMessage;
}

nlohmann::json QingYunKe_API(const std::string prompt, const std::string KEY) {
    CURL* curl;
    CURLcode res;
    std::string response;

    curl = curl_easy_init();

    if(curl) {
        std::string url = "http://api.qingyunke.com/api.php?key=" + KEY + "&appid=0&msg=" + replaceAll(prompt, " ", "");
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        res = curl_easy_perform(curl);

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);

        if(res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        }
    }

    return nlohmann::json::parse(response);
}

nlohmann::json GPT4_API(const std::string prompt, const std::string KEY) {
    CURL* curl;
    CURLcode res;
    std::string response;

    curl = curl_easy_init();

    if(curl) {
        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        std::string auth_header = "Authorization: Bearer " + KEY;
        headers = curl_slist_append(headers, auth_header.c_str());

        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        nlohmann::json json_data = {
            {"model", "gpt-4"},
            {"messages", {
                {
                    {"role", "system"},
                    {"content", "You are a helpful assistant"}
                },
                {
                    {"role", "user"},
                    {"content", prompt}
                }
            }}
        };

        std::string requestDataStr = json_data.dump();

        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, requestDataStr.c_str());

        curl_easy_setopt(curl, CURLOPT_URL, "https://api.openai.com/v1/chat/completions");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        res = curl_easy_perform(curl);

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);

        if(res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        }
    }
    return nlohmann::json::parse(response);
}

nlohmann::json GPT4_Turbo_API(const std::string prompt, const std::string KEY) {
    CURL* curl;
    CURLcode res;
    std::string response;

    curl = curl_easy_init();

    if(curl) {
        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        std::string auth_header = "Authorization: Bearer " + KEY;
        headers = curl_slist_append(headers, auth_header.c_str());

        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        nlohmann::json json_data = {
            {"model", "gpt-4-turbo-preview"},
            {"messages", {
                {
                    {"role", "system"},
                    {"content", "You are a helpful assistant"}
                },
                {
                    {"role", "user"},
                    {"content", prompt}
                }
            }}
        };

        std::string requestDataStr = json_data.dump();

        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, requestDataStr.c_str());

        curl_easy_setopt(curl, CURLOPT_URL, "https://api.openai.com/v1/chat/completions");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        res = curl_easy_perform(curl);

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);

        if(res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        }
    }
    return nlohmann::json::parse(response);
}

nlohmann::json Gemini_API(const std::string prompt, const std::string KEY) {
    CURL *curl;
    CURLcode res;
    std::string response;

    curl = curl_easy_init();
    if (curl) {
        std::string url = "https://generativelanguage.googleapis.com/v1beta/models/gemini-pro:generateContent?key=" + KEY;
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        nlohmann::json json_data = {
            {"contents", {{
                {"parts", {{
                    {"text", prompt}
                }}}
            }}},
            {"safetySettings", {{
                {"category", "HARM_CATEGORY_SEXUALLY_EXPLICIT"},
                {"threshold", "BLOCK_NONE"},
            }, {
                {"category", "HARM_CATEGORY_HATE_SPEECH"},
                {"threshold", "BLOCK_NONE"},
            }, {
                {"category", "HARM_CATEGORY_HARASSMENT"},
                {"threshold", "BLOCK_NONE"},
            }, {
                {"category", "HARM_CATEGORY_DANGEROUS_CONTENT"},
                {"threshold", "BLOCK_NONE"},
            }}},
            {"generationConfig", {
                {"stopSequences", nullptr},
                {"temperature", 1.0},
                {"maxOutputTokens", 2048},
                {"topP", 0.8},
                {"topK", 10}
            }}
        };

        std::string requestDataStr = json_data.dump();
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, requestDataStr.c_str());

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        res = curl_easy_perform(curl);

        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);

        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        }
    }

    return nlohmann::json::parse(response);
}

nlohmann::json Claude3_API(const std::string prompt, const std::string KEY) {
    CURL *curl;
    CURLcode res;
    std::string response;

    curl = curl_easy_init();
    if (curl) {
        struct curl_slist *headers = NULL;
        std::string auth_header = "x-api-key: " + KEY;
        headers = curl_slist_append(headers, auth_header.c_str());
        headers = curl_slist_append(headers, "anthropic-version: 2023-06-01");
        headers = curl_slist_append(headers, "content-type: application/json");
        curl_easy_setopt(curl, CURLOPT_URL, "https://api.anthropic.com/v1/messages");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        nlohmann::json json_data = {
            {"model", "claude-3-opus-20240229"},
            {"max_tokens", 1024},
            {"messages", {{
                {"role", "user"},
                {"content", prompt}
            }}}
        };

        std::string requestDataStr = json_data.dump();
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, requestDataStr.c_str());

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        res = curl_easy_perform(curl);

        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);

        if (res != CURLE_OK)
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
    }

    return nlohmann::json::parse(response);
}

std::pair<std::string, int> QingYunKe_API_Response(nlohmann::json response) {
    std::string content;
    int errcode;

    try {
        content = response["content"];
        errcode = 200;
    } catch (const nlohmann::json::exception &e) { 
        std::cerr << "QingYunKe: " << e.what() << std::endl;
        content = "";
        errcode = response["result"];
    }

    return std::make_pair(content, errcode);
}

std::pair<std::string, int> Claude3_API_Response(nlohmann::json response) {
    std::string content;
    int errorCode = 200;

    if (response["type"] == "message") {
        content = response["content"][0]["text"];
    } else if (response.contains("error")) {
        content = response["error"]["message"].get<std::string>(); 
        std::string errorType = response["error"]["type"];

        if (errorType == "invalid_request_error") {
            errorCode = 400;
        } else if (errorType == "authentication_error") {
            errorCode = 401;
        } else if (errorType == "permission_error") {
            errorCode = 403;
        } else if (errorType == "not_found_error") {
            errorCode = 404;
        } else if (errorType == "rate_limit_error") {
            errorCode = 429;
        } else if (errorType == "api_error") {
            errorCode = 500;
        } else if (errorType == "overloaded_error") {
            errorCode = 529;
        } else {
            errorCode = -1; // Unknown error type
        }
    }

    return std::make_pair(content, errorCode);
}

std::pair<std::string, int> Gemini_API_Response(nlohmann::json response) {
    std::string content;
    int errcode = 200;

    try {
        if (response.contains("candidates") && response["candidates"].size() > 0 &&
            response["candidates"][0].contains("content") && 
            response["candidates"][0]["content"].contains("parts") &&
            response["candidates"][0]["content"]["parts"].size() > 0 &&
            response["candidates"][0]["content"]["parts"][0].contains("text")) {
            content = response["candidates"][0]["content"]["parts"][0]["text"];
        } else {
            content = "status: Data missing or invalid";
            errcode = 400; 
        }
    } catch (const nlohmann::json::exception &e) {
        content = "status: JSON parsing error";
        errcode = 500; 
    }

    return std::make_pair(content, errcode);
}

std::pair<std::string, int> GPT4_API_Response(nlohmann::json response) {
    std::string content;
    int errcode = 200;

    try {
        // Check if "choices" array exists and has at least one element
        if (response.find("choices") != response.end() && response["choices"].is_array() && !response["choices"].empty()) {
            // Check if "message" object exists and "content" key exists within it
            if (response["choices"][0].find("message") != response["choices"][0].end() && response["choices"][0]["message"].is_object() && response["choices"][0]["message"].find("content") != response["choices"][0]["message"].end()) {
                content = response["choices"][0]["message"]["content"];
            } else {
                throw std::invalid_argument("Invalid JSON format: 'message' or 'content' key not found.");
            }
        } else {
            throw std::invalid_argument("Invalid JSON format: 'choices' array not found or empty.");
        }
    } catch (const std::invalid_argument &e) {
        content = response["error"]["type"].get<std::string>() + ": " + e.what();
        errcode = -1; // Unknown/null cases
    } catch (const nlohmann::json::exception &e) {
        content = response["error"]["type"].get<std::string>() + ": " + response["error"]["message"].get<std::string>();
        errcode = -1; // Unknown/null cases
    }

    return std::make_pair(content, errcode);
}

std::pair<std::string, int> GPT4_Turbo_API_Response(nlohmann::json response) {
    std::string content;
    int errcode = 200;

    try {
        // Check if "choices" array exists and has at least one element
        if (response.find("choices") != response.end() && response["choices"].is_array() && !response["choices"].empty()) {
            // Check if "message" object exists and "content" key exists within it
            if (response["choices"][0].find("message") != response["choices"][0].end() && response["choices"][0]["message"].is_object() && response["choices"][0]["message"].find("content") != response["choices"][0]["message"].end()) {
                content = response["choices"][0]["message"]["content"];
            } else {
                throw std::invalid_argument("Invalid JSON format: 'message' or 'content' key not found.");
            }
        } else {
            throw std::invalid_argument("Invalid JSON format: 'choices' array not found or empty.");
        }
    } catch (const std::invalid_argument &e) {
        content = response["error"]["type"].get<std::string>() + ": " + e.what();
        errcode = -1; // Unknown/null cases
    } catch (const nlohmann::json::exception &e) {
        content = response["error"]["type"].get<std::string>() + ": " + response["error"]["message"].get<std::string>();
        errcode = -1; // Unknown/null cases
    }

    return std::make_pair(content, errcode);
}

std::string FileErrorMessage(const nlohmann::json& settings, const std::string& nickname, const std::string& filetype) {
    std::string message = settings["channels"]["chatbots"]["category"]["error"]["incompatiblefile"].get<std::string>();
    // Replace placeholders with actual values
    size_t pos = 0;
    while ((pos = message.find("{nickname}", pos)) != std::string::npos) {
        message.replace(pos, 10, nickname); // 10 is the length of "{nickname}"
        pos += nickname.length(); // Use nickname.length() instead of name.length()
    }
    pos = 0;
    while ((pos = message.find("{filetype}", pos)) != std::string::npos) {
        message.replace(pos, 10, filetype); // 10 is the length of "{filetype}"
        pos += filetype.length(); // Use filetype.length() instead of location.length()
    }
    return message;
}