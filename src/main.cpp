#include <marong/marong.h>
#include <marong/helper.h>
#include <sstream>
#include <iostream>
#include <ctime>
#include <time.h>

using json = nlohmann::json;

int main(int argc, char const *argv[])
{

    std::ifstream configfile("../.config/config.json");
    std::ifstream settings_file("../.config/settings.json");

    json configdocument = json::parse(configfile);
    json settings = json::parse(settings_file);

    configfile.close();
    settings_file.close();

    dpp::cluster bot(configdocument["token"], dpp::i_all_intents);

    time_t timer;

    bot.on_log(dpp::utility::cout_logger());

    bot.on_slashcommand([&bot, &settings](const dpp::slashcommand_t& event) {
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

                        std::ofstream output_file("../.config/settings.json");
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
        }
    });

    bot.on_message_create([&bot, &timer, settings, configdocument](const dpp::message_create_t& event) {
        if (event.msg.author.is_bot()){
            return;
        }

        auto channelId = static_cast<int64_t>(event.msg.channel_id);
        if (channelId == static_cast<int64_t>(settings["channels"]["chatbots"]["gpt"]["chatter"]["id"])) {
            std::string prompt = event.msg.content;
            std::vector<dpp::attachment> attachments_vect = event.msg.attachments;
            if (!attachments_vect.empty()) {
                event.reply("此频道 __**暂时**__ 不支持文字文件", true);
                return;
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
                event.reply("此频道 __**暂时**__ 不支持文字文件", true);
                return;
            } else {
                json response = Claude3_API(prompt, configdocument["claude-key"]);
                event.reply(standardMessageFileWrapper(channelId, response["content"][0]["text"]), true);
            }
        }

        else if (channelId == static_cast<int64_t>(settings["channels"]["chatbots"]["gpt"]["gemini"]["id"])) {
            std::string prompt = event.msg.content;
            std::vector<dpp::attachment> attachments_vect = event.msg.attachments;
            if (!attachments_vect.empty()) {
                event.reply("此频道 __**暂时**__ 不支持文字文件", true);
                return;
            } else {
                json response = Gemini_API(prompt, configdocument["gemini-key"]);
                try {
                    event.reply(standardMessageFileWrapper(channelId, response["candidates"][0]["content"]["parts"][0]["text"]), true);
                } catch (const nlohmann::json::exception &e) {
                    std::cerr << to_string(response) << std::endl;
                    try {
                        json response = Gemini_API("您的名字叫兔子, 请像对待老板一样用短短一句话, 萌萌哒礼貌的语气来跟我道歉说 我回答不了您这个问题哦, 记得加上可爱的表情", configdocument["gemini-key"]);
                        event.reply(standardMessageFileWrapper(channelId, response["candidates"][0]["content"]["parts"][0]["text"]), true);
                    } catch (const nlohmann::json::exception &e) {                        
                        event.reply(settings["channels"]["chatbots"]["catagory"]["error"][getRandomIndex(settings["channels"]["chatbots"]["catagory"]["error"].size()-1)], true);
                    }
                }
            }
        }

        else if (channelId == static_cast<int64_t>(settings["channels"]["chatbots"]["gpt"]["gpt4"]["id"])) {
            std::string prompt = event.msg.content;
            std::vector<dpp::attachment> attachments_vect = event.msg.attachments;
            if (!attachments_vect.empty()) {
                event.reply("此频道 __**暂时**__ 不支持文字文件", true);
                return;
                for (dpp::attachment& attachment: attachments_vect){
                    std::string file_type = getStringBeforeSlash(attachment.content_type);
                    if (file_type == "text"){

                    } else if (file_type == "image"){
                        event.reply("此频道不支持图片文件 只支持文字和文字文件 但是 Gemini Pro 频道支持图片文件", true);
                        return;
                    } else {
                        event.reply("此频道不支持此类型文件 只支持文字和文字文件", true);
                    }
                }
            }

            json response = GPT4_API(prompt, configdocument["openai-key"], "gpt-4");
            event.reply(standardMessageFileWrapper(channelId, response["choices"][0]["message"]["content"]), true);
        }

        else if (channelId == static_cast<int64_t>(settings["channels"]["chatbots"]["gpt"]["gpt4-turbo"]["id"])) {
            std::string prompt = event.msg.content;
            std::vector<dpp::attachment> attachments_vect = event.msg.attachments;
            if (!attachments_vect.empty()) {
                event.reply("此频道 __**暂时**__ 不支持文字文件", true);
                return;
                for (dpp::attachment& attachment: attachments_vect){
                    std::string file_type = getStringBeforeSlash(attachment.content_type);
                    if (file_type == "text"){

                    } else if (file_type == "image"){
                        event.reply("此频道不支持图片文件 只支持文字和文字文件 但是 Gemini Pro 频道支持图片文件", true);
                        return;
                    } else {
                        event.reply("此频道不支持此类型文件 只支持文字和文字文件", true);
                    }
                }
            }

            json response = GPT4_API(prompt, configdocument["openai-key"], "gpt-4-turbo-preview");
            event.reply(standardMessageFileWrapper(channelId, response["choices"][0]["message"]["content"]), true);
        }
    });

    bot.on_ready([&bot](const dpp::ready_t& event) {

        if (dpp::run_once<struct register_bot_commands>()) {
            bot.global_command_create(dpp::slashcommand("gpt4-turbo", "仅服务器拥有者或特级权限管理员: 创建一个 GPT4 Turbo 的频道", bot.me.id));
            bot.global_command_create(dpp::slashcommand("gpt4", "仅服务器拥有者或特级权限管理员: 创建一个 GPT4 的频道", bot.me.id));
            bot.global_command_create(dpp::slashcommand("gemini", "仅服务器拥有者或特级权限管理员: 创建一个 Gemini Pro Ultra 的频道", bot.me.id));
            bot.global_command_create(dpp::slashcommand("claude3", "仅服务器拥有者或特级权限管理员: 创建一个 Claude 3 Opus 的频道", bot.me.id));
            bot.global_command_create(dpp::slashcommand("chatter", "仅服务器拥有者或特级权限管理员: 创建一个可以和 marong 闲聊的频道", bot.me.id));
        }
    });

    bot.start(dpp::st_wait);

    return 0;
}