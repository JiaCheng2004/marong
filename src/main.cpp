#include <marong/marong.h>

using json = nlohmann::json;

int main(int argc, char const *argv[]) {

    std::ifstream configfile("../config/config.json");
    std::ifstream settings_file("../config/settings.json");
    std::ifstream users_file("../config/users.json");

    json configdocument = json::parse(configfile);
    json settings = json::parse(settings_file);
    json users = json::parse(users_file);

    configfile.close();
    settings_file.close();

    dpp::cluster bot(configdocument["token"], dpp::i_all_intents);

    std::map<std::string, dpp::channel> user_voice_map;
    std::map<std::string, std::vector<std::pair<std::string, int>>> channel_map;

    time_t timer;

    bot.on_log(dpp::utility::cout_logger());

    bot.on_slashcommand([&bot, &settings, &users](const dpp::slashcommand_t& event) {
        if (settings["channels"]["chatbots"]["gpt"].contains(event.command.get_command_name())) {
            dpp::guild Guild = event.command.get_guild();
            dpp::snowflake UserID = event.command.get_issuing_user().id;
            if ((Guild.owner_id == UserID) || (has_role(event.command.member, settings["roles"]["PRIVILEGED_ADMIN"]))){
                if (!has_channel(Guild,  settings["channels"]["chatbots"]["category"]["id"])){
                    dpp::channel newCat = newCategory( settings["channels"]["chatbots"]["category"]["label"], Guild.id, 0);
                    settings["channels"]["chatbots"]["category"]["id"] = newCat.id;
                    bot.channel_create(newCat);
                }

                dpp::snowflake CategoryID = settings["channels"]["chatbots"]["category"]["id"];
                std::string gptName = event.command.get_command_name();
                auto chatbot = settings["channels"]["chatbots"]["gpt"][gptName];

                if (!has_channel(Guild, chatbot["id"])){
                    dpp::channel newChannel = newTextChannel(std::string(chatbot["emoji_unicode"]) + "｜﹒" + std::string(chatbot["label"]), Guild.id, CategoryID, 0);
                    bot.channel_create(newChannel, [&bot, &settings, chatbot, event, gptName](const dpp::confirmation_callback_t& callback){
                        if (callback.is_error()) { 
                            bot.log(dpp::loglevel::ll_error, callback.get_error().message);
                            return;
                        }

                        auto channel = callback.get<dpp::channel>();
                        settings["channels"]["chatbots"]["gpt"][gptName]["id"] = static_cast<int64_t>(channel.id);

                        std::ofstream output_file("../config/settings.json");
                        if (output_file.is_open()) {
                            output_file << settings.dump(4) << std::endl;
                            output_file.close();
                            event.reply("成功创建了支持 " + std::string(chatbot["fullname"]) + " 的聊天频道");
                        } else {
                            event.reply("成功创建了支持 " + std::string(chatbot["fullname"]) + " 的聊天频道, 但无法保存频道的ID到文件");
                        }
                    });
                } else {
                    event.reply("此服务器已存在 " + std::string(chatbot["fullname"]) + " 的频道");
                }
            } else {
                event.reply("仅服务器拥有者或特级权限管理员才能使用此指令");
            }
        } else if (event.command.get_command_name() == "newuser"){
            std::ofstream output_file("../config/users.json");
            dpp::guild_member Member = event.command.member;
            newUser(users, Member.user_id.str(), Member.get_nickname());
            if (output_file.is_open()) {
                output_file << users.dump(4) << std::endl;
                output_file.close();
                event.reply("registered!!");
            } else {
                event.reply("failed to register...");
            }
        }
    });


    bot.on_message_create([&bot, &timer, settings, configdocument](const dpp::message_create_t& event) -> dpp::task<void>  {
        if (event.msg.author.is_bot()){
            co_return;
        }

        int64_t channelId = static_cast<int64_t>(event.msg.channel_id);
        if (channelId == static_cast<int64_t>(settings["channels"]["chatbots"]["gpt"]["chatter"]["id"])) {
            std::string prompt = event.msg.content;
            std::vector<dpp::attachment> attachments_vect = event.msg.attachments;
            if (!attachments_vect.empty()) {
                event.reply("此频道 **暂时** 不支持文字文件", true);
                co_return;
            } else {
                std::string response = QingYunKe_API(prompt)["content"];
                replaceAll(response, "{br}", "\n");
                event.reply(standardMessageFileWrapper(channelId, response), true);
            }
        }

        else if (channelId == static_cast<int64_t>(settings["channels"]["chatbots"]["gpt"]["claude3"]["id"])) {
            std::string prompt = event.msg.content;
            std::vector<dpp::attachment> attachments_vect = event.msg.attachments;
            if (!attachments_vect.empty()) {
                for (dpp::attachment& attachment: attachments_vect){
                    std::string file_type = getStringBeforeSlash(attachment.content_type);
                    if (file_type == "text"){
                        dpp::http_request_completion_t result = co_await bot.co_request(attachment.url, dpp::m_get);
                        if (result.status == 200){
                            attachTextfile(prompt, attachment.filename, result.body);
                        }
                    } else {
                        event.reply("此频道 **暂时** 不支持 **" + attachment.content_type + "** 类型文件 只支持文字和文字文件", true);
                    }
                }
            }
            json response = Claude3_API(prompt, configdocument["claude-key"]);
            try {
                event.reply(standardMessageFileWrapper(channelId, response["content"][0]["text"]), true);
            } catch (const nlohmann::json::exception &e) {
                std::cerr << "Claude3:\n" << to_string(response) << std::endl;
                prompt = "你的名字叫 marong , 请像对待老板一样, 用简短的话语和可爱的语气来跟我道歉, 非常着急的说'这种问题一般不会发生!!! 如果你看到我说这句话说明要赶紧联系 JC 来修我啦!'，记得要带上可爱的表情哦~";
                json response = Gemini_API(prompt, configdocument["gemini-key"]);
                try {
                    event.reply(standardMessageFileWrapper(channelId, response["candidates"][0]["content"]["parts"][0]["text"]), true);
                } catch (const nlohmann::json::exception &e) {                        
                    event.reply(settings["channels"]["chatbots"]["catagory"]["error"][0], true);
                }
            }
        }

        else if (channelId == static_cast<int64_t>(settings["channels"]["chatbots"]["gpt"]["gemini"]["id"])) {
            std::string prompt = event.msg.content;
            std::vector<dpp::attachment> attachments_vect = event.msg.attachments;
            if (!attachments_vect.empty()) {
                for (dpp::attachment& attachment: attachments_vect){
                    std::string file_type = getStringBeforeSlash(attachment.content_type);
                    if (file_type == "text"){
                        dpp::http_request_completion_t result = co_await bot.co_request(attachment.url, dpp::m_get);
                        if (result.status == 200){
                            attachTextfile(prompt, attachment.filename, result.body);
                        }
                    } else {
                        event.reply("此频道 **暂时** 不支持 **" + attachment.content_type + "** 类型文件 只支持文字和文字文件", true);
                    }
                }
            }
            json response = Gemini_API(prompt, configdocument["gemini-key"]);
            try {
                event.reply(standardMessageFileWrapper(channelId, response["candidates"][0]["content"]["parts"][0]["text"]), true);
            } catch (const nlohmann::json::exception &e) {
                std::cerr << "Gemini:\n" << to_string(response) << std::endl;
                try {
                    int errorCode = response["error"]["code"].get<int>();
                    switch (errorCode) {
                        case 400:
                            event.reply(standardMessageFileWrapper(channelId, "404 烂API拒绝访问"), true);
                        case 500:
                            prompt = "你的名字叫 marong , 请像对待老板一样, 用简短的话语和可爱的语气来跟我道歉, 遗憾的说'抱歉主人 我的内部API访问出了点问题 请稍后再来问小奴这个问题吧'，记得要带上可爱的表情哦~";
                            break;
                        case 503:
                            prompt = "你的名字叫 marong , 请像对待老板一样, 用简短的话语和可爱的语气来跟我道歉, 耐心的说'问题太多啦 我处理不过来啦 请稍后再来问小奴这个问题吧~'，记得要带上可爱的表情哦~";
                            break;
                        default:
                            prompt = "你的名字叫 marong , 请像对待老板一样, 用简短的话语和可爱的语气来跟我道歉, 可惜的说'这个问题超出我的知识范围啦, 小奴无能为力呢~', 记得要带上可爱的表情哦~";
                    }
                    json response = Gemini_API(prompt, configdocument["gemini-key"]);
                    event.reply(standardMessageFileWrapper(channelId, response["candidates"][0]["content"]["parts"][0]["text"]), true);
                } catch (const nlohmann::json::exception &e) {                        
                    event.reply(settings["channels"]["chatbots"]["catagory"]["error"][0], true);
                }
            }
        }

        else if (channelId == static_cast<int64_t>(settings["channels"]["chatbots"]["gpt"]["gpt4"]["id"])) {
            std::string prompt = event.msg.content;
            std::vector<dpp::attachment> attachments_vect = event.msg.attachments;
            if (!attachments_vect.empty()) {
                for (dpp::attachment& attachment: attachments_vect){
                    std::string file_type = getStringBeforeSlash(attachment.content_type);
                    if (file_type == "text"){
                        dpp::http_request_completion_t result = co_await bot.co_request(attachment.url, dpp::m_get);
                        if (result.status == 200){
                            attachTextfile(prompt, attachment.filename, result.body);
                        }
                    } else {
                        event.reply("此频道 **暂时** 不支持 **" + attachment.content_type + "** 类型文件 只支持文字和文字文件", true);
                    }
                }
            }
            json response = GPT4_API(prompt, configdocument["openai-key"], "gpt-4");
            try{
                event.reply(standardMessageFileWrapper(channelId, response["choices"][0]["message"]["content"]), true);
            } catch (const nlohmann::json::exception &e) {
                std::cerr << "GPT4:\n" << to_string(response) << std::endl;
                prompt = "你的名字叫 marong , 请像对待老板一样, 用简短的话语和可爱的语气来跟我道歉, 非常着急的说'这种问题一般不会发生!!! 如果你看到我说这句话说明要赶紧联系 JC 来修我啦!'，记得要带上可爱的表情哦~";
                json response = Gemini_API(prompt, configdocument["gemini-key"]);
                try {
                    event.reply(standardMessageFileWrapper(channelId, response["candidates"][0]["content"]["parts"][0]["text"]), true);
                } catch (const nlohmann::json::exception &e) {                        
                    event.reply(settings["channels"]["chatbots"]["catagory"]["error"][0], true);
                }
            }
        }

        else if (channelId == static_cast<int64_t>(settings["channels"]["chatbots"]["gpt"]["gpt4-turbo"]["id"])) {
            std::string prompt = event.msg.content;
            std::vector<dpp::attachment> attachments_vect = event.msg.attachments;
            if (!attachments_vect.empty()) {
                for (dpp::attachment& attachment: attachments_vect){
                    std::string file_type = getStringBeforeSlash(attachment.content_type);
                    if (file_type == "text"){
                        dpp::http_request_completion_t result = co_await bot.co_request(attachment.url, dpp::m_get);
                        if (result.status == 200){
                            attachTextfile(prompt, attachment.filename, result.body);
                        }
                    } else {
                        event.reply("此频道 **暂时** 不支持 **" + attachment.content_type + "** 类型文件 只支持文字和文字文件", true);
                    }
                }
            }
            json response = GPT4_API(prompt, configdocument["openai-key"], "gpt-4-turbo-preview");
            try{
                event.reply(standardMessageFileWrapper(channelId, response["choices"][0]["message"]["content"]), true);
            } catch (const nlohmann::json::exception &e) {
                std::cerr << "GPT4 Turbo:\n" << to_string(response) << std::endl;
                prompt = "你的名字叫 marong , 请像对待老板一样, 用简短的话语和可爱的语气来跟我道歉, 非常着急的说'这种问题一般不会发生!!! 如果你看到我说这句话说明要赶紧联系 JC 来修我啦!'，记得要带上可爱的表情哦~";
                json response = Gemini_API(prompt, configdocument["gemini-key"]);
                try {
                    event.reply(standardMessageFileWrapper(channelId, response["candidates"][0]["content"]["parts"][0]["text"]), true);
                } catch (const nlohmann::json::exception &e) {                        
                    event.reply(settings["channels"]["chatbots"]["catagory"]["error"][0], true);
                }
            }
        }
    });

    bot.on_voice_state_update([&bot, &settings, &users, &user_voice_map, &channel_map](const dpp::voice_state_update_t& event) -> dpp::task<void> {

        if (!users.contains(event.state.user_id.str())){
            co_return;
        }

        std::string UserID = event.state.user_id.str();
        std::string ChannelID = event.state.channel_id.str();
        std::string GuildID = event.state.guild_id.str();
        
        dpp::guild_member Member = bot.guild_get_member_sync(event.state.guild_id, event.state.user_id);
        std::pair<std::string, int> user_info = std::make_pair(UserID, 1);
        
        if (event.state.channel_id != 0){

                dpp::confirmation_callback_t channel_response = co_await bot.co_channel_get(event.state.channel_id);
                dpp::channel voiceChannel = channel_response.get<dpp::channel>();
                user_voice_map[UserID] = voiceChannel;

            if (user_voice_map.find(UserID) != user_voice_map.end()) { // joining a voice channel

                // Check if the destination channel is empty
                if (channel_map.find(ChannelID) != channel_map.end()){ // destination channel is empty
                    // Push user_info to the array of destination channel
                    channel_map[ChannelID].push_back(user_info);  
                    
                    // Don't need to Sort the array of destination channel because it only have one user

                    // Update the name of destination voice channel
                    voiceChannel.set_name(get_supertitle(users, UserID));
                    bot.channel_edit(voiceChannel);
                } else {
                    // Initialize destination channel with an empty vector of pairs <std::string, int>
                    channel_map[ChannelID] = std::vector<std::pair<std::string, int>>();
                    // Push user_info to the array of destination channel
                    channel_map[ChannelID].push_back(user_info);

                    // Sort the array of destination channel
                    insertionSort(channel_map[ChannelID]); 
                    
                    // Update the name of destination voice channel

                    voiceChannel.set_name(get_supertitle(users, channel_map[ChannelID][0].first));
                    bot.channel_edit(voiceChannel);
                }

            } else { // else if the user switch a voice channel
                // Switch to new voice Channel case:

                // Get the previous voice channel
                dpp::channel prevChannel = user_voice_map[UserID];

                // Check if the previous voice channel is going to be empty
                if (channel_map[voiceChannel.id.str()].size() == 1) { // is going to be empty
                    channel_map.erase(ChannelID);

                    // Set the the voice channel's name that user was in back to orignal 
                    std::string originalName = settings["channels"]["public-voice-channels"][prevChannel.id.str()]["name"];
                    prevChannel.set_name(originalName);
                    bot.channel_edit(prevChannel);
                } else {
                    // Iterating over the vector to find and remove the pair
                    auto& vectorToRemoveFrom = channel_map[voiceChannel.id.str()];
                    for (auto it = vectorToRemoveFrom.begin(); it != vectorToRemoveFrom.end(); ++it) {
                        if (it->first == UserID) {
                            vectorToRemoveFrom.erase(it);
                            break;
                        }
                    }

                    // Sort the array of destination channel
                    insertionSort(channel_map[ChannelID]); 
                    // Update the name of previous voice channel
                    prevChannel.set_name(get_supertitle(users, channel_map[ChannelID][0].first));
                    bot.channel_edit(prevChannel);
                }

                // Check if the destination channel is empty
                if (channel_map.find(ChannelID) != channel_map.end()){ // destination channel is not empty
                    // Push user_info to the array of destination channel
                    channel_map[ChannelID].push_back(user_info);

                    // Sort the array of destination channel
                    insertionSort(channel_map[ChannelID]); 
                    
                    // Update the name of destination voice channel

                    voiceChannel.set_name(get_supertitle(users, channel_map[ChannelID][0].first));
                    bot.channel_edit(voiceChannel);
                } else {
                    // Initialize destination channel with an empty vector of pairs <std::string, int>
                    channel_map[ChannelID] = std::vector<std::pair<std::string, int>>();
                    // Push user_info to the array of destination channel
                    channel_map[ChannelID].push_back(user_info);

                    // Don't need to sort the array of destination channel because it only have one user

                    // Update the name of destination voice channel
                    voiceChannel.set_name(get_supertitle(users, UserID));
                    bot.channel_edit(voiceChannel);
                }

            }
        } else { // else if a user disconnect from a voice channel

            // Extra Debug in Case
            if (user_voice_map.find(UserID) != user_voice_map.end()) {
                // Get the previous voice channel that user was in
                dpp::channel prevChannel = user_voice_map[UserID];

                // Clear user from user_voice_map (to Save Memory)
                user_voice_map.erase(UserID);

                // Check if the previous voice channel that user was is going to be empty
                if (channel_map[prevChannel.id.str()].size() == 1) { // The previous voice channel is now empty
                    // Remove the previous voice channel that user was in from channel_map (to Save Memory)
                    channel_map.erase(ChannelID);
                    // Set the the voice channel's name that user was in back to orignal 
                    std::string originalName = settings["channels"]["public-voice-channels"][prevChannel.id.str()]["name"];
                    prevChannel.set_name(originalName);
                    bot.channel_edit(prevChannel);
                } else {
                    // Get reference to the vector associated with the key
                    auto& vectorToRemoveFrom = channel_map[prevChannel.id.str()];

                    // Iterating over the vector to find and remove the pair
                    for (auto it = vectorToRemoveFrom.begin(); it != vectorToRemoveFrom.end(); ++it) {
                        if (it->first == UserID) {
                            vectorToRemoveFrom.erase(it);
                            break;
                        }
                    }
                                    
                    // Sort the array of destination channel
                    insertionSort(channel_map[ChannelID]); 

                    // Update the name of destination voice channel
                    prevChannel.set_name(get_supertitle(users, channel_map[ChannelID][0].first));
                    bot.channel_edit(prevChannel);
                }
            } else {
                std::cerr << "Impossible. User disconnected from a voice channel that he was never in" << std::endl;
            }
        }
    });


    bot.on_ready([&bot](const dpp::ready_t& event) {

        if (dpp::run_once<struct register_bot_commands>()) {
            bot.global_command_create(dpp::slashcommand("gpt4-turbo", "仅服务器拥有者或特级权限管理员: 创建一个 GPT4 Turbo 的频道", bot.me.id));
            bot.global_command_create(dpp::slashcommand("gpt4", "仅服务器拥有者或特级权限管理员: 创建一个 GPT4 的频道", bot.me.id));
            bot.global_command_create(dpp::slashcommand("gemini", "仅服务器拥有者或特级权限管理员: 创建一个 Gemini Pro Ultra 的频道", bot.me.id));
            bot.global_command_create(dpp::slashcommand("claude3", "仅服务器拥有者或特级权限管理员: 创建一个 Claude 3 Opus 的频道", bot.me.id));
            bot.global_command_create(dpp::slashcommand("chatter", "仅服务器拥有者或特级权限管理员: 创建一个可以和 marong 闲聊的频道", bot.me.id));
            bot.global_command_create(dpp::slashcommand("newuser", "创建用户", bot.me.id));
        }
    });

    bot.start(dpp::st_wait);

    return 0;
}