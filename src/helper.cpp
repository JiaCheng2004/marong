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
    dpp::channel newChannel;

    newChannel.set_name(channelName).set_guild_id(guildId);
    newChannel.set_parent_id(parentId);
    newChannel.set_type(dpp::channel_type::CHANNEL_TEXT);
    newChannel.set_position(position);

    return newChannel;
}

dpp::channel newCategory(std::string categoryName, dpp::snowflake guildId, uint16_t position){
    dpp::channel newCat;

    newCat.set_name(categoryName);
    newCat.set_guild_id(guildId);
    newCat.set_type(dpp::channel_type::CHANNEL_CATEGORY);
    newCat.set_position(position);
    
    return newCat;
}
