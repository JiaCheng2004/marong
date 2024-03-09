#include <marong/marong.h>
#include <marong/helper.h>
#include <sstream>
#include <iostream>

using json = nlohmann::json;

int main(int argc, char const *argv[])
{
    
    std::ifstream configfile("../config/config.json");
    std::ifstream settings_file("../config/settings.json");

    json configdocument = json::parse(configfile);
    json settings = json::parse(settings_file);

    configfile.close();
    settings_file.close();

    /* Setup the bot */
    dpp::cluster bot(configdocument["token"], dpp::i_default_intents | dpp::i_guild_members );

    /* Output simple log messages to stdout */
    bot.on_log(dpp::utility::cout_logger());

    /* Handle slash command */
    bot.on_slashcommand([&bot, &settings](const dpp::slashcommand_t& event) {
        if (settings["channels"]["chatbots"]["gpt"].contains(event.command.get_command_name())) {
            dpp::guild Guild = event.command.get_guild();
            dpp::snowflake UserID = event.command.get_issuing_user().id;
            if ((Guild.owner_id == UserID) || (has_role(event.command.member, settings["roles"]["SUPREME_ADMIN"]))){
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
                            event.reply("成功创建了支持 " + std::string(chatbot["fullname"]) + " API 的聊天频道");
                        } else {
                            event.reply("成功创建了支持 " + std::string(chatbot["fullname"]) + " API 的聊天频道, 但无法保存频道的ID到文件");
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

    /* Register slash command here in on_ready */
    bot.on_ready([&bot](const dpp::ready_t& event) {
        /* Wrap command registration in run_once to make sure it doesnt run on every full reconnection */
        if (dpp::run_once<struct register_bot_commands>()) {
            bot.global_command_create(dpp::slashcommand("gpt4-turbo", "仅服务器拥有者或特级权限管理员: 创建一个支持GPT4 Turbo的文本频道", bot.me.id));
            bot.global_command_create(dpp::slashcommand("gpt4", "仅服务器拥有者或特级权限管理员: 创建一个支持 GPT4 的文本频道", bot.me.id));
            bot.global_command_create(dpp::slashcommand("gemini", "仅服务器拥有者或特级权限管理员: 创建一个支持 Gemini Pro Ultra 的文本频道", bot.me.id));
            bot.global_command_create(dpp::slashcommand("claude3", "仅服务器拥有者或特级权限管理员: 创建一个支持 Claude 3 Opus 的文本频道", bot.me.id));
        }
    });

    /* Start the bot */
    bot.start(dpp::st_wait);

    return 0;
}