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

void setlower(std::string& str) {
    std::transform(str.begin(), str.end(), str.begin(), ::tolower);
    return;
}

std::string getlower(std::string str) {
    std::string str_copy = str;
    std::transform(str_copy.begin(), str_copy.end(), str_copy.begin(), ::tolower);
    return str_copy;
}

void setupper(std::string& str) {
    std::transform(str.begin(), str.end(), str.begin(), ::toupper);
    return;
}

std::string getupper(std::string str) {
    std::string str_copy = str;
    std::transform(str_copy.begin(), str_copy.end(), str_copy.begin(), ::toupper);
    return str_copy;
}

int getRandomIndex(int length) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, length - 1);
    return dis(gen);
}

void insertionSort(std::vector<std::pair<std::string, int>>& arr){
    if (arr.empty()) {
        return;
    }

    int i, j;
    std::pair<std::string, int> key;
    for(i = 1; i<arr.size(); i++){
        key = arr[i];
        j = i - 1;
    
        /* Move elements of arr[0..i-1], that are
        less than key, to one position ahead
        of their current position */
        
        while(j >= 0 && arr[j].second < key.second){
            arr[j+1] = arr[j];
            j = j - 1;
        }
        arr[j + 1] = key;
    }
}

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
        std::cerr << "--------------------------------------------------------\n";
        std::cerr << "Channel: " << pair.first << std::endl;
        const auto& vec = pair.second;
        for (const auto& inner_pair : vec) {
            int i = 1;
            std::cerr << i <<".     UserID: " << inner_pair.first << ", Priority: " << inner_pair.second << std::endl;
            ++i;
        }
        std::cerr << "--------------------------------------------------------\n";
    }
}

void savefile(std::string filename, nlohmann::json& file) {
    std::ofstream output_file(filename);
    if (output_file.is_open()) {
        output_file << file.dump(4) << std::endl;
        output_file.close();
        std::cerr << "成功保存\n";
    } else {
        std::cerr << "保存失败\n";
    }
}

void channelMapRemove(std::map<std::string, std::vector<std::pair<std::string, int>>>& channel_map, std::string ChannelID, std::string UserID) {
    auto& voiceMembers = channel_map[ChannelID];
    for (auto it = voiceMembers.begin(); it != voiceMembers.end(); ++it) {
        std::cerr << it->first << " == " << UserID << std::endl;
        if (it->first == UserID) {
            voiceMembers.erase(it);
            break;
        }
    }
}

void UpdateSuperTitle(dpp::cluster& bot, dpp::channel& Channel, nlohmann::json& settings, nlohmann::json& users, std::map<std::string, dpp::timer>& timer_map, std::vector<std::pair<std::string, int>>& voiceMember) {
    std::cerr << "a\n";
    std::string ChannelName;
    if (voiceMember.empty()) {
        std::cerr << Channel.id.str() << "\n";
        ChannelName = settings["channels"]["public-voice-channels"][Channel.id.str()]["name"];
        std::cerr << "e\n";
        Channel.set_name(ChannelName);
        std::cerr << "f\n";
        bot.stop_timer(timer_map[Channel.id.str()]);
        std::cerr << "c\n";
        timer_map.erase(Channel.id.str());
    } else {
        std::cerr << "g\n";
        ChannelName = get_supertitle(users, voiceMember[0].first);
        std::cerr << Channel.id.str() << "\n";
        Channel.set_name(ChannelName);
        std::cerr << "i\n";
    }
    std::cerr << "j\n";
    bot.channel_edit(Channel);
}

bool isAllDigits(const std::string& str) {
    std::string::const_iterator it = str.begin();
    if (it != str.end() && *it == '-') ++it;
    while (it != str.end() && std::isdigit(*it)) ++it;
    return !str.empty() && it == str.end();
}