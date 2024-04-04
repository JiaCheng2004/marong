#include <marong/marong.h>

int main(int argc, char const *argv[]) {

    std::ifstream configfile(config_address);
    std::ifstream settingsfile(settings_address);
    std::ifstream usersfile(users_address);

    json configdocument = json::parse(configfile);
    json settings = json::parse(settingsfile);
    json users = json::parse(usersfile);

    configfile.close();
    settingsfile.close();
    usersfile.close();

    dpp::cluster bot(configdocument["token"], dpp::i_all_intents);
    std::map<uint64_t, GPTFunctionCaller> channelFunctionMap;

    std::map<std::string, dpp::channel> user_voice_map;

    //       ChannelID    [<UserID, Level>]
    std::map<std::string, std::vector<std::pair<std::string, int>>> channel_map;   

    std::map<std::string, dpp::timer> timer_map; 

    // Initialize the gptFuntionMap
    std::map<uint64_t, std::pair<GPTFunctionCaller, GPTResponseDealer>> gptFuntionMap = {
        {static_cast<uint64_t>(settings["channels"]["chatbots"]["gpt"]["chatter"]["id"]), {QingYunKe_API, QingYunKe_API_Response}},
        {static_cast<uint64_t>(settings["channels"]["chatbots"]["gpt"]["claude3"]["id"]), {Claude3_API, Claude3_API_Response}},
        {static_cast<uint64_t>(settings["channels"]["chatbots"]["gpt"]["gemini"]["id"]), {Gemini_API, Gemini_API_Response}},
        {static_cast<uint64_t>(settings["channels"]["chatbots"]["gpt"]["gpt4"]["id"]), {GPT4_API, GPT4_API_Response}},
        {static_cast<uint64_t>(settings["channels"]["chatbots"]["gpt"]["gpt4-turbo"]["id"]), {GPT4_Turbo_API, GPT4_Turbo_API_Response}}
    };

    // Initialize the gptKeyMap
    std::map<uint64_t, std::string> gptKeyMap;

    time_t os_timer;

    bot.on_log(dpp::utility::cout_logger());

    bot.on_slashcommand([&bot, &settings, &users, &gptKeyMap, configdocument](const dpp::slashcommand_t& event) -> dpp::task<void> {

        // when a command is "gpt"
        if (event.command.get_command_name() == "gpt") {

            // get the dpp::guild and dpp::user of the user that issue the command
            dpp::guild Guild = event.command.get_guild();
            dpp::user User = event.command.get_issuing_user();

            // If user is not a SUPER_ADMIN, PRIVILEGED_ADMIN, or SERVER_OWNER, then they can't user this command
            if ((Guild.owner_id != User.id) && (!has_role(event.command.member, settings["roles"]["SUPER_ADMIN"])) && (!has_role(event.command.member, settings["roles"]["PRIVILEGED_ADMIN"]))) {
                event.reply("仅服务器拥有者, 超级权限管理员或特级权限管理员才能使用此指令");
                co_return;
            }
            
            // Get the model channel that the user is trying to create
            std::string model = std::get<std::string>(event.get_parameter("model"));
            // Set the option into lower case for better to deal with
            setlower(model);
            // If the option is not a supported model, then tell the user what models are supported
            if (!settings["channels"]["chatbots"]["gpt"].contains(model)) {
                event.reply("目前只支持 gpt4, gpt4-turbo, gemini, claude3, chatter, 和 bing");
                co_return;
            }

            // If the category does not exist or can not be found, then create a category to place the gpt channels
            if (!has_channel(Guild,  settings["channels"]["chatbots"]["category"]["id"])){
                dpp::channel newCat = newCategory( settings["channels"]["chatbots"]["category"]["label"], Guild.id, 0);
                settings["channels"]["chatbots"]["category"]["id"] = newCat.id;
                bot.channel_create(newCat);
            }

            // initialize all the variables for future usage.
            // CategoryID - to know where to put the channel if create one
            dpp::snowflake CategoryID = settings["channels"]["chatbots"]["category"]["id"];

            // chatbot    - to know which one the user is trying to create, this is a json that include the emoji_unicode, label and id
            json chatbot = settings["channels"]["chatbots"]["gpt"][model];

            // channelID  - the channelID of the chat bot (initaially zero)
            uint64_t channelID = dpp::snowflake(chatbot["id"]);

            // In the gptKeyMap, if the key already exist or the channel already exist
            if (gptKeyMap.find(channelID) != gptKeyMap.end() || has_channel(Guild, chatbot["id"])) {
                gptKeyMap[static_cast<uint64_t>(chatbot["id"])] = configdocument[model];
                std::cerr << static_cast<uint64_t>(chatbot["id"]) << std::endl;
                event.reply("此服务器已存在 " + std::string(chatbot["fullname"]) + " 的频道");
                co_return;
            }
            
            // create a new channel with the emoji_unicode and label
            dpp::channel newChannel = newTextChannel(std::string(chatbot["emoji_unicode"]) + "｜﹒" + std::string(chatbot["label"]), Guild.id, CategoryID, 0);

            // make a corotine function call to create a channel
            dpp::confirmation_callback_t callback = co_await bot.co_channel_create(newChannel);

            // If got an error then log the error and return;
            if (callback.is_error()) {
                bot.log(dpp::loglevel::ll_error, callback.get_error().message);
                co_return;
            }

            // Get the info about channel from the callback
            dpp::channel Channel = callback.get<dpp::channel>();

            // Write the channelID to the settings.json after creation
            settings["channels"]["chatbots"]["gpt"][model]["id"] = static_cast<int64_t>(Channel.id);

            // Push the new key/value to the gptKeyMap
            gptKeyMap[static_cast<uint64_t>(Channel.id)] = configdocument[model];

            // Save the settings.json file
            savefile(settings_address, settings);

            // Tell the administrator that channel is sucessfully created
            event.reply("成功创建 " + std::string(chatbot["fullname"]) + " 的频道");
            co_return;
            
        } else if (event.command.get_command_name() == "newuser") {
            dpp::guild_member Member = event.command.member;
            newUser(users, Member.user_id.str(), Member.get_nickname());
            savefile(users_address, users);
        } else if (event.command.get_command_name() == "play") {
            std::string song = std::get<std::string>(event.get_parameter("search"));
	        dpp::guild* g = dpp::find_guild(event.command.guild_id);
 
            if (!g->connect_member_voice(event.command.get_issuing_user().id)) {
                event.reply("You don't seem to be in a voice channel!");
                co_return;
            }

            dpp::voiceconn* v = event.from->get_voice(event.command.guild_id);

	        if (!v || !v->voiceclient || !v->voiceclient->is_ready()) {
                event.reply("There was an issue with getting the voice channel. Make sure I'm in a voice channel!");
                co_return;
            }

            handle_streaming(v, song);
        }
    });


    bot.on_message_create([&bot, settings, configdocument, &gptKeyMap, gptFuntionMap](const dpp::message_create_t& event) -> dpp::task<void> {
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
                        } else {
                            event.reply("抱歉, 无法下载此文件: " + attachment.filename, true);
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
                            event.reply("文件类型错误 目前仅支持文本格式文件 (如.txt .cpp .c .py .java .js类文件等等)", true);
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
                co_return;
            } else {
                // Implement Later. (Different error code reponse handling)
                event.reply("抱歉 由于请求限制或其他错误 无法发出请求 请稍后再试 错误代码: " + std::to_string(responsePair.second), true);
                co_return;
            }
        }
    });

    bot.on_voice_state_update([&bot, &settings, &users, &user_voice_map, &channel_map, &timer_map](const dpp::voice_state_update_t& event) -> dpp::task<void> {
        std::cerr << "0\n";

        if (!users.contains(event.state.user_id.str()) || !settings["channels"]["public-voice-channels"].contains(event.state.channel_id.str())){
            co_return;
        }

        std::cerr << "\n\n==========================================\n";
        printUserVoiceMap(user_voice_map);
        printChannelMap(channel_map);
        std::cerr << "==========================================\n";

        std::cerr << "0.5\n";

        std::string UserID = event.state.user_id.str();
        std::string ChannelID = event.state.channel_id.str();
        std::string GuildID = event.state.guild_id.str();
        int TIME = 5;

        std::cerr << "0.75\n";

        dpp::guild_member Member = bot.guild_get_member_sync(event.state.guild_id, event.state.user_id);
        std::cerr << "0.8\n";
        int priority = static_cast<int>(users[UserID]["status"]["level"]) * 500 + static_cast<int>(users[UserID]["status"]["exp"]);
        std::cerr << priority << std::endl;
        std::pair<std::string, int> user_info = std::make_pair(UserID, priority);

        if (event.state.channel_id != 0) {                              // Connecting to a channel

            dpp::confirmation_callback_t channel_response = co_await bot.co_channel_get(event.state.channel_id);
            dpp::channel voiceChannel = channel_response.get<dpp::channel>();

            std::cerr << "1\n";

            if (user_voice_map.find(UserID) != user_voice_map.end()) {  // Switching to the a Channel

                std::cerr << "2\n";
                dpp::channel prevChannel = user_voice_map[UserID];
                channelMapRemove(channel_map, prevChannel.id.str(), UserID);
                user_voice_map[UserID] = voiceChannel;

                std::cerr << "3\n";

                channel_map[voiceChannel.id.str()].push_back(user_info);

                std::cerr << "4\n";
                if (channel_map.find(ChannelID) != channel_map.end()) { // Empty channel
                    std::cerr << "5\n";
                    dpp::timer handle = bot.start_timer([&bot, &voiceChannel, &users, &timer_map, &channel_map, &settings](dpp::timer h) {
                        UpdateSuperTitle(bot, voiceChannel, settings, users, timer_map, channel_map[voiceChannel.id.str()]);
                    }, TIME);
                    std::cerr << "6\n";
                    timer_map[ChannelID] = handle;
                    std::cerr << "7\n";
                }
            } else {                                                    // Joining a channel
                std::cerr << "8\n";
                user_voice_map[UserID] = voiceChannel;
                channel_map[voiceChannel.id.str()].push_back(user_info);
                std::cerr << "9\n";
                if (channel_map.find(ChannelID) != channel_map.end()) { // Empty channel
                    std::cerr << "10\n";
                    dpp::timer handle = bot.start_timer([&bot, &voiceChannel, &users, &timer_map, &channel_map, &settings](dpp::timer h) {
                        UpdateSuperTitle(bot, voiceChannel, settings, users, timer_map, channel_map[voiceChannel.id.str()]);
                    }, TIME);
                    std::cerr << "11\n";
                    timer_map[ChannelID] = handle;
                    std::cerr << "12\n";
                }
            }
        } else {                                                        // Disconnecting from a channel
            dpp::channel prevChannel = user_voice_map[UserID];
            std::cerr << "20\n";
            if (user_voice_map.find(UserID) != user_voice_map.end()) {
                std::cerr << "21\n";
                channelMapRemove(channel_map, prevChannel.id.str(), UserID);
            }
        }
    });


    bot.on_ready([&bot](const dpp::ready_t& event) {

        if (dpp::run_once<struct register_bot_commands>()) {
            bot.global_command_create(dpp::slashcommand("gpt", "仅服务器拥有者或特级权限管理员: 创建一个或者重新创建 GPT 频道", bot.me.id)
                .add_option(dpp::command_option(dpp::co_string, "model", "要创建或者重新创建的GPT频道(gpt4-turbo/gpt4/gemini/claude3/chatter/bing)", true))
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