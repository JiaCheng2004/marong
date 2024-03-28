#include <marong/marong.h>

int main(int argc, char const *argv[]) {

    std::ifstream configfile("../config/config.json");
    std::ifstream settings_file("../config/settings.json");
    std::ifstream users_file("../config/users.json");

    json configdocument = json::parse(configfile);
    json settings = json::parse(settings_file);
    json users = json::parse(users_file);

    configfile.close();
    settings_file.close();
    users_file.close();

    dpp::cluster bot(configdocument["token"], dpp::i_all_intents);
    std::map<uint64_t, GPTFunctionCaller> channelFunctionMap;

    std::map<std::string, dpp::channel> user_voice_map;
    std::map<std::string, std::vector<std::pair<std::string, int>>> channel_map;

    // Initialize the gptFuntionMap
    std::map<uint64_t, std::pair<GPTFunctionCaller, GPTResponseDealer>> gptFuntionMap = {
        {static_cast<uint64_t>(settings["channels"]["chatbots"]["gpt"]["chatter"]["id"]), {QingYunKe_API, QingYunKe_API_Response}},
        {static_cast<uint64_t>(settings["channels"]["chatbots"]["gpt"]["claude3"]["id"]), {Claude3_API, Claude3_API_Response}},
        {static_cast<uint64_t>(settings["channels"]["chatbots"]["gpt"]["gemini"]["id"]), {Gemini_API, Gemini_API_Response}},
        {static_cast<uint64_t>(settings["channels"]["chatbots"]["gpt"]["gpt4"]["id"]), {GPT4_API, GPT4_API_Response}},
        {static_cast<uint64_t>(settings["channels"]["chatbots"]["gpt"]["gpt4-turbo"]["id"]), {GPT4_Turbo_API, GPT4_Turbo_API_Response}}
    };

    // Initialize the gptKeyMap
    std::map<uint64_t, std::string> gptKeyMap = {
        {static_cast<uint64_t>(settings["channels"]["chatbots"]["gpt"]["chatter"]["id"]), "free"},
        {static_cast<uint64_t>(settings["channels"]["chatbots"]["gpt"]["claude3"]["id"]), configdocument["claude-key"]},
        {static_cast<uint64_t>(settings["channels"]["chatbots"]["gpt"]["gemini"]["id"]), configdocument["gemini-key"]},
        {static_cast<uint64_t>(settings["channels"]["chatbots"]["gpt"]["gpt4"]["id"]), configdocument["openai-key"]},
        {static_cast<uint64_t>(settings["channels"]["chatbots"]["gpt"]["gpt4-turbo"]["id"]), configdocument["openai-key"]}
    };

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
        } else if (event.command.get_command_name() == "play") {
            std::string song = std::get<std::string>(event.get_parameter("search"));
	        dpp::guild* g = dpp::find_guild(event.command.guild_id);
 
            if (!g->connect_member_voice(event.command.get_issuing_user().id)) {
                event.reply("You don't seem to be in a voice channel!");
                return;
            }

            dpp::voiceconn* v = event.from->get_voice(event.command.guild_id);

	        if (!v || !v->voiceclient || !v->voiceclient->is_ready()) {
                event.reply("There was an issue with getting the voice channel. Make sure I'm in a voice channel!");
                return;
            }

            handle_streaming(v, song);
        }
    });


    bot.on_message_create([&bot, &timer, settings, configdocument, gptKeyMap, gptFuntionMap](const dpp::message_create_t& event) -> dpp::task<void> {
        if (event.msg.author.is_bot()){
            co_return;
        }

        // Convert message's channelID into uint64_t type
        uint64_t channelID = dpp::snowflake(event.msg.channel_id);

        // Check if this message's channel exist in the gptKeyMap, meaning if it's a valid channel to deal with
        if (gptKeyMap.find(channelID) != gptKeyMap.end()) {
            // If channelID is in gptKeyMap

            // If this message contains any stickers, return.
            if (!event.msg.stickers.empty()) {
                co_return;
            }

            // Get the gptKey from gptKeyMap to make the api call
            std::string key = gptKeyMap.at(channelID);

            // Get the function api maker/dealer specifically for this channel from the gptFunctionMap associated with the channelID
            // first value in the gptFunctionMap is the function api caller
            GPTFunctionCaller gptFuntionCaller = gptFuntionMap.at(channelID).first;
            // second value in the gptFunctionMap is the function response dealer
            GPTResponseDealer gptResponseDealer = gptFuntionMap.at(channelID).second;

            // Get the member from this guild that sended this message
            dpp::guild_member Author = event.msg.member;
            // Get the message content of this message
            std::string MessageContent = event.msg.content;
            // Get all the attachments of this message
            std::vector<dpp::attachment> attachments_vect = event.msg.attachments;

            // Check if this message contains attachments
            if (!attachments_vect.empty()) {
                // Iterate through all the attachment in the attachments vector
                for (dpp::attachment& attachment: attachments_vect){
                    // Using getStringBeforeSlash() to get the file type of current attachment
                    std::string filetype = getStringBeforeSlash(attachment.content_type);
                    // if the file type is a text format file. (.txt, .cpp, .c, .py, .java, etc...)
                    if (filetype == "text") {
                        // Use co_request to request the content of the attachment with attachment's url and in m_get format
                        dpp::http_request_completion_t result = co_await bot.co_request(attachment.url, dpp::m_get);
                        // Check if request is success
                        if (result.status == 200){
                            // If success, use attachTextfile to append the content of the file to the prompt
                            attachTextfile(MessageContent, attachment.filename, result.body);
                        }
                    } else { // Any other type of file
                        // Use FileErrorMessage to get a file error prompt to tell message author what went wrong
                        std::string prompt = FileErrorMessage(settings, Author.get_nickname(), attachment.content_type);
                        // Make an Gemini API call to with the file error prompt and get a pair of the response
                        std::pair<std::string, int> resPair = Gemini_API_Response(Gemini_API(prompt, configdocument["gemini-key"]));
                        // If the respondse is 200, successful.
                        if (resPair.second == 200) {
                            // Reply the message with the standardMessageFileWrapper
                            event.reply(standardMessageFileWrapper(event.msg.channel_id, resPair.first), true);
                        } else { // Else if something when wrong with the Gemini API Caller:
                            // Implement Later. (Different error code reponse handling)
                            event.reply("Error. Wrong file type. Only support text format file(.txt, .cpp, .c, .py, .java, etc...) ", true);
                        }
                        // return to stop dealing with this message
                        co_return;
                    }
                }
            }

            // Make an API call with the result prompt and key
            json response = gptFuntionCaller(MessageContent, key);
            // Store the response in a response paire
            std::pair<std::string, int> responsePair = gptResponseDealer(response);
            
            // If the respondse is 200, successful.
            if (responsePair.second == 200) {
                // Reply the message with the standardMessageFileWrapper
                event.reply(standardMessageFileWrapper(event.msg.channel_id, responsePair.first), true);
            } else {
                // Implement Later. (Different error code reponse handling)
                event.reply("apologize, no answer", true);
            }
        }
    });

    bot.on_voice_state_update([&bot, &settings, &users, &user_voice_map, &channel_map](const dpp::voice_state_update_t& event) -> dpp::task<void> {

        if (!users.contains(event.state.user_id.str())){
            co_return;
        }

        std::cerr << "\n\n==========================================\n";
        printUserVoiceMap(user_voice_map);
        printChannelMap(channel_map);
        std::cerr << "==========================================\n";

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
                if (channel_map[prevChannel.id.str()].size() == 1) { // is going to be empty
                    channel_map.erase(prevChannel.id.str());

                    // Set the the voice channel's name that user was in back to orignal 
                    std::string originalName = settings["channels"]["public-voice-channels"][prevChannel.id.str()]["name"];
                    prevChannel.set_name(originalName);
                    bot.channel_edit(prevChannel);
                } else {
                    // Iterating over the vector to find and remove the pair
                    auto& vectorToRemoveFrom = channel_map[prevChannel.id.str()];
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
                    channel_map.erase(prevChannel.id.str());
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
            bot.global_command_create(dpp::slashcommand("gpt", "仅服务器拥有者或特级权限管理员: 创建一个或者重新创建 GPT 频道", bot.me.id)
                .add_option(dpp::command_option(dpp::co_string, "model", "要创建或者重新创建的GPT频道(all/gpt4-turbo/gpt4/gemini/claude3/chatter/bing)", true))
            );
            bot.global_command_create(dpp::slashcommand("newuser", "创建用户", bot.me.id));
            bot.global_command_create(dpp::slashcommand("exp", "查看自己的经验值", bot.me.id));
            bot.global_command_create(dpp::slashcommand("music", "创建自己的音乐频道", bot.me.id)
                .add_option(dpp::command_option(dpp::co_string, "channel-name", "音乐频道", true))
            );
            bot.global_command_create(dpp::slashcommand("play", "点歌", bot.me.id)
                .add_option(dpp::command_option(dpp::co_string, "search", "歌曲链接/歌名/歌手", true))
            );
        }
    });

    bot.start(dpp::st_wait);

    return 0;
}