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
