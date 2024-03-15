#include <marong/helper.h>

bool has_role(dpp::guild_member M, dpp::snowflake role_id){
    std::vector<dpp::snowflake> role_ids = M.get_roles();

    if (role_ids.empty()) {return false;}

    for (dpp::snowflake i: role_ids){
        if (i == role_id){
            return true;
        }
    }

    return false;
}

bool has_channel(dpp::guild G, dpp::snowflake channel_id){
    std::vector<dpp::snowflake> channel_ids = G.channels;

    for (dpp::snowflake i: channel_ids){
        if (i == channel_id){
            return true;
        }
    }

    return false;
}

dpp::channel newTextChannel(std::string channelName, dpp::snowflake guildId, dpp::snowflake parentId, uint16_t position){
    dpp::channel newChannel = dpp::channel().set_name(channelName).set_guild_id(guildId).set_parent_id(parentId).set_type(dpp::channel_type::CHANNEL_TEXT).set_position(position);
    return newChannel;
}

dpp::channel newCategory(std::string categoryName, dpp::snowflake guildId, uint16_t position){
    dpp::channel newCat = dpp::channel().set_name(categoryName).set_guild_id(guildId).set_type(dpp::channel_type::CHANNEL_CATEGORY).set_position(position);
    return newCat;
}

nlohmann::json curlUrlToJsonResponse(const std::string& urlString) { 
    CURL* curl = curl_easy_init();
    nlohmann::json jsonData;

    if(curl) {
        std::string readBuffer;

        curl_easy_setopt(curl, CURLOPT_URL, urlString.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        curl_easy_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

        long httpCode(0);

        curl_easy_perform(curl);
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
        curl_easy_cleanup(curl);

        if(httpCode == 200) {
            try {
                jsonData = nlohmann::json::parse(readBuffer);

            } catch(nlohmann::json::parse_error &e) {

                return jsonData;
            }
            return jsonData;
        }
    }

    return jsonData;
}

void replaceAll(std::string& str, const std::string& arg1, const std::string& arg2) {
    size_t pos = 0;
    while ((pos = str.find(arg1, pos)) != std::string::npos) {
        str.replace(pos, arg1.length(), arg2);
        pos += arg2.length();
    }
}

size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp){
    userp->append((char*)contents, size * nmemb);
    return size * nmemb;
}

nlohmann::json QingYunKe_API(std::string prompt){
    replaceAll(prompt, " ", "");
    CURL* curl;
    CURLcode res;
    std::string response;

    curl = curl_easy_init();

    if(curl) {
        std::string url = "http://api.qingyunke.com/api.php?key=free&appid=0&msg=" + prompt;
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

nlohmann::json GPT4_API(std::string prompt, std::string KEY, std::string model) {
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
            {"model", model},
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
                {"stopSequences", {""}},
                {"temperature", 1.0},
                {"maxOutputTokens", 800},
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

std::string getStringBeforeSlash(const std::string& str) {
    size_t pos = str.find('/');
    if (pos != std::string::npos) {
        return str.substr(0, pos);
    }
    return str;
}

void attachTextfile(std::string& prompt, const std::string& fileName, const std::string& fileContent){
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

int getRandomIndex(int length) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, length - 1);
    return dis(gen);
}

void insertionSort(std::vector<std::pair<std::string, int>>& arr){
    int i, j;
    std::pair<std::string, int> key;
    for(i = 1; i<arr.size(); i++){
        key = arr[i];
        j = i - 1;

        /* Move elements of arr[0..i-1], that are
        greater than key, to one position ahead
        of their current position */
        while(j >= 0 && arr[j].second > key.second){
            arr[j+1] = arr[j];
            j = j - 1;
        }
        arr[j + 1] = key;
    }
}

int getMostPrioritizedRole(void){
    return 0;
}

// dpp::task<dpp::guild_member> coro_get_channel(dpp::cluster &bot, dpp::snowflake &channel_id){
//         dpp::confirmation_callback_t result = co_await bot.co_channel_get(channel_id);
//         if (result.http_info.status == 200){
//             co_return result.get();
//         } else {
//             std::cerr << "Invalid http_info: " << result.http_info << std::endl;
//         }
// }

// Function to print the contents of 'user_voice_map'
void printUserVoiceMap(const std::map<std::string, dpp::channel>& user_voice_map) {
    for (const auto& pair : user_voice_map) {
        std::cerr << "User: " << pair.first << ", Channel: " << pair.second.name << std::endl;
        // Assuming dpp::channel has a suitable << operator overload or method for printing
    }
}

// Function to print the contents of 'channel_map'
void printChannelMap(const std::map<std::string, std::vector<std::pair<std::string, int>>>& channel_map) {
    for (const auto& pair : channel_map) {
        std::cerr << "Channel: " << pair.first << std::endl;
        const auto& vec = pair.second;
        for (const auto& inner_pair : vec) {
            std::cerr << "  User: " << inner_pair.first << ", Int: " << inner_pair.second << std::endl;
        }
    }
}
