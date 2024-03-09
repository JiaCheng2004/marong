#include <dpp/dpp.h>
#include <dpp/nlohmann/json.hpp>
#include <sstream>
#include <iostream>

bool has_role(dpp::guild_member M, dpp::snowflake role_id);

bool has_channel(dpp::guild G, dpp::snowflake channel_id);

dpp::channel newTextChannel(std::string channelName, dpp::snowflake guildId, dpp::snowflake parentId, uint16_t position);

dpp::channel newCategory(std::string categoryName, dpp::snowflake guildId, uint16_t position);
